#include "AutopatcherClient.h"
#include "DirectoryDeltaTransfer.h"
#include "FileList.h"
#include "StringCompressor.h"
#include "RakPeerInterface.h"
#include "FileListTransfer.h"
#include "FileListTransferCBInterface.h"
#include "BitStream.h"
#include "PacketEnumerations.h"
#include "AutopatcherPatchContext.h"
#include "ApplyPatch.h"
#include "FileOperations.h"
#include "SHA1.h"
#include <stdio.h>
#include "FileOperations.h"

#ifdef _MSC_VER
#pragma warning( push )
#endif

#define COPY_ON_RESTART_EXTENSION ".patched.tmp"

class AutopatcherClientCallback : public FileListTransferCBInterface
{
public:
	char applicationDirectory[512];
	FileListTransferCBInterface *onFileCallback;
	AutopatcherClient *client;

	virtual void OnFile(
		unsigned fileIndex,
		char *filename,
		char *fileData,
		unsigned compressedTransmissionLength,
		unsigned finalDataLength,
		unsigned short setID,
		unsigned setCount,	
		unsigned setTotalCompressedTransmissionLength,
		unsigned setTotalFinalLength,
		unsigned char context)
	{
		char fullPathToDir[1024];

		if (filename && fileData)
		{
			strcpy(fullPathToDir, applicationDirectory);
			strcat(fullPathToDir, filename);
			if (context==PC_WRITE_FILE)
			{
				if (WriteFileWithDirectories(fullPathToDir, (char*)fileData, finalDataLength)==false)
				{
					char newDir[1024];
					strcpy(newDir, fullPathToDir);
					strcat(newDir, COPY_ON_RESTART_EXTENSION);
					if (WriteFileWithDirectories(newDir, (char*)fileData, finalDataLength))
					{
						// Regular file in use but we can write the temporary file.  Restart and copy it over the existing
						client->CopyAndRestart(filename);	
						onFileCallback->OnFile(fileIndex, fullPathToDir, fileData, compressedTransmissionLength, finalDataLength, setID, setCount, setTotalCompressedTransmissionLength, setTotalFinalLength, PC_NOTICE_WILL_COPY_ON_RESTART);
					}
					else
						onFileCallback->OnFile(fileIndex, fullPathToDir, fileData, compressedTransmissionLength, finalDataLength, setID, setCount, setTotalCompressedTransmissionLength, setTotalFinalLength, PC_ERROR_FILE_WRITE_FAILURE);
				}
				else
					onFileCallback->OnFile(fileIndex, fullPathToDir, fileData, compressedTransmissionLength, finalDataLength, setID, setCount, setTotalCompressedTransmissionLength, setTotalFinalLength, context);
			}
			else
			{
				assert(context==PC_HASH_WITH_PATCH);
				
				CSHA1 sha1;
				FILE *fp;
				unsigned prePatchLength, postPatchLength;
				char *prePatchFile, *postPatchFile;

				fp=fopen(fullPathToDir, "rb");
				if (fp==0)
				{
					onFileCallback->OnFile(fileIndex, fullPathToDir, fileData, compressedTransmissionLength, finalDataLength, setID, setCount, setTotalCompressedTransmissionLength, setTotalFinalLength, PC_ERROR_PATCH_TARGET_MISSING);
					client->Redownload(filename);
					return;
				}
				fseek(fp, 0, SEEK_END);
				prePatchLength = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				prePatchFile= new char [prePatchLength];
				fread(prePatchFile, prePatchLength, 1, fp);
				fclose(fp);

//				printf("apply patch %i bytes\n", finalDataLength-SHA1_LENGTH);
//				for (int i=0; i < finalDataLength-SHA1_LENGTH; i++)
//					printf("%i ", fileData[SHA1_LENGTH+i]);
//				printf("\n");
				if (ApplyPatch((char*)prePatchFile, prePatchLength, &postPatchFile, &postPatchLength, (char*)fileData+SHA1_LENGTH, finalDataLength-SHA1_LENGTH)==false)
				{
					delete [] prePatchFile;
					// Failure - signal class and download this file.
					onFileCallback->OnFile(fileIndex, fullPathToDir, fileData, compressedTransmissionLength, finalDataLength, setID, setCount, setTotalCompressedTransmissionLength, setTotalFinalLength, PC_ERROR_PATCH_APPLICATION_FAILURE);
					client->Redownload(filename);
					return;
				}

				sha1.Reset();
				sha1.Update((unsigned char*) postPatchFile, postPatchLength);
				sha1.Final();

				if (memcmp(sha1.GetHash(), fileData, SHA1_LENGTH)!=0)
				{
					delete [] postPatchFile;
					delete [] prePatchFile;
					// Failure - signal class and download this file.
					onFileCallback->OnFile(fileIndex, fullPathToDir, fileData, compressedTransmissionLength, finalDataLength, setID, setCount, setTotalCompressedTransmissionLength, setTotalFinalLength, PC_ERROR_PATCH_RESULT_CHECKSUM_FAILURE);
					client->Redownload(filename);
					return;
				}

				// Write postPatchFile over the existing file
				if (WriteFileWithDirectories(fullPathToDir, (char*)postPatchFile, postPatchLength)==false)
				{
					char newDir[1024];
					strcpy(newDir, fullPathToDir);
					strcat(newDir, ".copy_on_restart.tmp");
					if (WriteFileWithDirectories(newDir, (char*)postPatchFile, postPatchLength))
					{
						// Regular file in use but we can write the temporary file.  Restart and copy it over the existing
						client->CopyAndRestart(filename);
						onFileCallback->OnFile(fileIndex, fullPathToDir, (char*) postPatchFile, compressedTransmissionLength, postPatchLength, setID, setCount, setTotalCompressedTransmissionLength, setTotalFinalLength, PC_NOTICE_WILL_COPY_ON_RESTART);
					}
					else
						onFileCallback->OnFile(fileIndex, fullPathToDir, (char*) postPatchFile, compressedTransmissionLength, postPatchLength, setID, setCount, setTotalCompressedTransmissionLength, setTotalFinalLength, PC_ERROR_FILE_WRITE_FAILURE);
				}
				else
					onFileCallback->OnFile(fileIndex, fullPathToDir, (char*) postPatchFile, compressedTransmissionLength, postPatchLength, setID, setCount, setTotalCompressedTransmissionLength, setTotalFinalLength, context);

				delete [] postPatchFile;
				delete [] prePatchFile;
			}
		}
	}
	virtual void OnFileProgress(unsigned fileIndex,
		char *filename,
		unsigned compressedTransmissionLength,
		unsigned finalDataLength,
		unsigned short setID,
		unsigned setCount,	
		unsigned setTotalCompressedTransmissionLength,
		unsigned setTotalFinalLength,
		unsigned char context,
		unsigned int partCount,
		unsigned int partTotal,
		unsigned int partLength)
	{
		char fullPathToDir[1024];

		if (filename)
		{
			strcpy(fullPathToDir, applicationDirectory);
			strcat(fullPathToDir, filename);
			onFileCallback->OnFileProgress(fileIndex, fullPathToDir, compressedTransmissionLength, finalDataLength, setID, setCount, setTotalCompressedTransmissionLength, setTotalFinalLength, context, partCount, partTotal, partLength);
		}
	}
};

AutopatcherClient::AutopatcherClient()
{
	rakPeer=0;
	serverId=UNASSIGNED_PLAYER_ID;
	applicationDirectory[0]=0;
	fileListTransfer=0;
    priority=HIGH_PRIORITY;
	orderingChannel=0;
	serverDate[0]=0;
}
AutopatcherClient::~AutopatcherClient()
{
	Clear();
}
void AutopatcherClient::Clear(void)
{
	if (fileListTransfer)
		fileListTransfer->RemoveReceiver(serverId);
	serverId=UNASSIGNED_PLAYER_ID;
	setId=(unsigned short)-1;
	redownloadList.Clear();
	copyAndRestartList.Clear();
}
void AutopatcherClient::SetUploadSendParameters(PacketPriority _priority, char _orderingChannel)
{
	priority=_priority;
	orderingChannel=_orderingChannel;
}
void AutopatcherClient::SetFileListTransferPlugin(FileListTransfer *flt)
{
	fileListTransfer=flt;
}
char* AutopatcherClient::GetServerDate(void) const
{
	return (char*)serverDate;
}
bool AutopatcherClient::PatchApplication(const char *_applicationName, const char *_applicationDirectory, const char *lastUpdateDate, PlayerID host, FileListTransferCBInterface *onFileCallback, const char *restartOutputFilename, const char *pathToRestartExe)
{
    assert(applicationName);
	assert(applicationDirectory);
	assert(rakPeer);
	assert(pathToRestartExe);
	assert(restartOutputFilename);

	if (rakPeer->GetIndexFromPlayerID(host)==-1)
		return false;

	strcpy(applicationDirectory, _applicationDirectory);
	if (IsSlash(applicationDirectory[strlen(applicationDirectory)-1])==false)
		strcat(applicationDirectory, "\\");
	strcpy(applicationName, _applicationName);
	serverId=host;
	cb=onFileCallback;
	strcpy(copyOnRestartOut, restartOutputFilename);
	strcpy(restartExe, pathToRestartExe);

	RakNet::BitStream outBitStream;
	outBitStream.Write((unsigned char)ID_AUTOPATCHER_GET_CHANGELIST_SINCE_DATE);
	stringCompressor->EncodeString(applicationName, 512, &outBitStream);
	stringCompressor->EncodeString(lastUpdateDate, 64, &outBitStream);
    rakPeer->Send(&outBitStream, priority, RELIABLE_ORDERED, orderingChannel, host, false);
	return true;
}
void AutopatcherClient::OnAttach(RakPeerInterface *peer)
{
	rakPeer=peer;
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void AutopatcherClient::Update(RakPeerInterface *peer)
{

}
PluginReceiveResult AutopatcherClient::OnReceive(RakPeerInterface *peer, Packet *packet)
{
	switch (packet->data[0]) 
	{
	case ID_CONNECTION_LOST:
	case ID_DISCONNECTION_NOTIFICATION:
		if (packet->playerId==serverId)
			Clear();
	case ID_AUTOPATCHER_CREATION_LIST:
		return OnCreationList(peer, packet);
	case ID_AUTOPATCHER_DELETION_LIST:
		OnDeletionList(peer, packet);
		return RR_STOP_PROCESSING_AND_DEALLOCATE;
	case ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR:
		fileListTransfer->RemoveReceiver(serverId);
		Clear();
		return RR_CONTINUE_PROCESSING;
	case ID_AUTOPATCHER_FINISHED:
		return OnAutopatcherFinished(peer, packet);
	}
	return RR_CONTINUE_PROCESSING;
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void AutopatcherClient::OnDisconnect(RakPeerInterface *peer)
{

}

PluginReceiveResult AutopatcherClient::OnCreationList(RakPeerInterface *peer, Packet *packet)
{
	assert(fileListTransfer);
	if (packet->playerId!=serverId)
		return RR_STOP_PROCESSING_AND_DEALLOCATE;

	RakNet::BitStream inBitStream(packet->data, packet->length, false);
	RakNet::BitStream outBitStream;
	FileList remoteFileList, missingOrChanged;
	inBitStream.IgnoreBits(8);
	if (remoteFileList.Deserialize(&inBitStream)==false)
		return RR_STOP_PROCESSING_AND_DEALLOCATE;

	stringCompressor->DecodeString(serverDate, 128, &inBitStream);

	// Go through the list of hashes.  For each file we already have, remove it from the list.
	remoteFileList.ListMissingOrChangedFiles(applicationDirectory, &missingOrChanged, true, false);

	if (missingOrChanged.fileList.Size()==0)
	{
		packet->data[0]=ID_AUTOPATCHER_FINISHED;
		return RR_CONTINUE_PROCESSING; // Pass to user
	}

	// Prepare the transfer plugin to get a file list.
	AutopatcherClientCallback *transferCallback;
	transferCallback = new AutopatcherClientCallback;
	strcpy(transferCallback->applicationDirectory, applicationDirectory);
	transferCallback->onFileCallback=cb;
	transferCallback->client=this;
	setId = fileListTransfer->SetupReceive(transferCallback, true, packet->playerId);

	// Ask for patches for the files in the list that are different from what we have.
	outBitStream.Write((unsigned char)ID_AUTOPATCHER_GET_PATCH);
	outBitStream.Write(setId);
	stringCompressor->EncodeString(applicationName, 512, &outBitStream);
	missingOrChanged.Serialize(&outBitStream);
	rakPeer->Send(&outBitStream, priority, RELIABLE_ORDERED, orderingChannel, packet->playerId, false);

	return RR_STOP_PROCESSING_AND_DEALLOCATE; // Absorb this message
}
void AutopatcherClient::OnDeletionList(RakPeerInterface *peer, Packet *packet)
{
	if (packet->playerId!=serverId)
		return;

	RakNet::BitStream inBitStream(packet->data, packet->length, false);
	RakNet::BitStream outBitStream;
	inBitStream.IgnoreBits(8);
	FileList fileList;
	if (fileList.Deserialize(&inBitStream)==false)
		return;
	fileList.DeleteFiles(applicationDirectory);
}

PluginReceiveResult AutopatcherClient::OnAutopatcherFinished(RakPeerInterface *peer, Packet *packet)
{
	fileListTransfer->RemoveReceiver(serverId);

	// If redownload list, process it
	if (redownloadList.fileList.Size())
	{
		RakNet::BitStream outBitStream;
		AutopatcherClientCallback *transferCallback;
		transferCallback = new AutopatcherClientCallback;
		strcpy(transferCallback->applicationDirectory, applicationDirectory);
		transferCallback->onFileCallback=cb;
		transferCallback->client=this;
		setId = fileListTransfer->SetupReceive(transferCallback, true, packet->playerId);

		// Ask for patches for the files in the list that are different from what we have.
		outBitStream.Write((unsigned char)ID_AUTOPATCHER_GET_PATCH);
		outBitStream.Write(setId);
		stringCompressor->EncodeString(applicationName, 512, &outBitStream);
		redownloadList.Serialize(&outBitStream);
		rakPeer->Send(&outBitStream, priority, RELIABLE_ORDERED, orderingChannel, packet->playerId, false);
		redownloadList.Clear();
		return RR_STOP_PROCESSING_AND_DEALLOCATE; // Absorb
	}
	else if (copyAndRestartList.fileList.Size())
	{
		packet->data[0]=ID_AUTOPATCHER_RESTART_APPLICATION;
		FILE *fp;
		fp = fopen(copyOnRestartOut, "wt");
		assert(fp);
		if (fp)
		{
			fprintf(fp, "#Sleep 1000\n");
			unsigned i;
			for (i=0; i < copyAndRestartList.fileList.Size(); i++)
			{
				fprintf(fp, "del \"%s%s\"\n", applicationDirectory, copyAndRestartList.fileList[i].filename);
				fprintf(fp, "rename \"%s%s%s\" \"%s\"\n", applicationDirectory, copyAndRestartList.fileList[i].filename, COPY_ON_RESTART_EXTENSION, copyAndRestartList.fileList[i].filename);
			}
			fprintf(fp, "#CreateProcess \"%s\"\n", restartExe);
			fprintf(fp, "#DeleteThisFile\n");
			fclose(fp);
		}
	}

	RakNet::BitStream inBitStream(packet->data, packet->length, false);
	inBitStream.IgnoreBits(8);
	stringCompressor->DecodeString(serverDate, 128, &inBitStream);

	// Return to user
	return RR_CONTINUE_PROCESSING;
}
void AutopatcherClient::CopyAndRestart(const char *filePath)
{
	// We weren't able to write applicationDirectory + filePath so we wrote applicationDirectory + filePath + COPY_ON_RESTART_EXTENSION instead
	copyAndRestartList.AddFile(filePath, 0, 0, 0, 0);
}
void AutopatcherClient::Redownload(const char *filePath)
{
	redownloadList.AddFile(filePath, 0, 0, 0, 0);
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
