// Common includes
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "RakNetworkFactory.h"
#include "GetTime.h"
#include "RakPeerInterface.h"
#include "PacketEnumerations.h"
#include "BitStream.h"
#include "StringCompressor.h"

// Client only includes
#include "FileListTransferCBInterface.h"
#include "FileListTransfer.h"
#include "AutopatcherClient.h"
#include "AutopatcherPatchContext.h"

#ifdef _WIN32
#include <windows.h> // Sleep
#else
#include <unistd.h> // usleep
#endif

class TestCB : public FileListTransferCBInterface
{
public:
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
		if (context==PC_HASH_WITH_PATCH)
			printf("Patching: ");
		else if (context==PC_WRITE_FILE)
			printf("Writing: ");
		else if (context==PC_ERROR_FILE_WRITE_FAILURE)
			printf("Write Failure: ");
		else if (context==PC_ERROR_PATCH_TARGET_MISSING)
			printf("Patch target missing: ");
		else if (context==PC_ERROR_PATCH_APPLICATION_FAILURE)
			printf("Patch process failure: ");
		else if (context==PC_ERROR_PATCH_RESULT_CHECKSUM_FAILURE)
			printf("Patch checksum failure: ");
		else if (context==PC_NOTICE_WILL_COPY_ON_RESTART)
			printf("Copy pending restart: ");

		printf("%i. (100%%) %i/%i %s %ib->%ib / %ib->%ib\n", setID, fileIndex, setCount, filename, compressedTransmissionLength, finalDataLength, setTotalCompressedTransmissionLength, setTotalFinalLength);
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
		printf("Downloading: %i. (%i%) %i/%i %s %ib->%ib / %ib->%ib\n", setID, 100*partCount/partTotal, fileIndex, setCount, filename, compressedTransmissionLength, finalDataLength, setTotalCompressedTransmissionLength, setTotalFinalLength);
	}
} transferCallback;

void main(int argc, char **argv)
{
	printf("A simple client interface for the advanced autopatcher.\n");
	printf("Use DirectoryDeltaTransfer for a simpler version of an autopatcher.\n");
	printf("Difficulty: Intermediate\n\n");

	RakPeerInterface *rakPeer;
	rakPeer = RakNetworkFactory::GetRakPeerInterface();
	printf("Client starting...");
	rakPeer->Initialize(1,0,0,0);
	// Plugin will send us downloading progress notifications if a file is split to fit under the MTU 10 or more times
	rakPeer->SetSplitMessageProgressInterval(10);
	// Plugin will send us downloading progress notifications if a file is split to fit under the MTU 10 or more times
	rakPeer->SetSplitMessageProgressInterval(10);
	printf("started\n");
	char buff[512];
	printf("Enter server IP: ");
	gets(buff);
	if (buff[0]==0)
		strcpy(buff, "127.0.0.1");
	rakPeer->Connect(buff, 60000, 0, 0);
	printf("Connecting...\n");
	printf("Enter application directory: ");
	char appDir[512];
	gets(appDir);
	if (appDir[0]==0)
	{
		strcpy(appDir, "C:/temp/AutopatcherClient");
	}
	printf("Enter application name: ");
	char appName[512];
	gets(appName);
	if (appName[0]==0)
		strcpy(appName, "TestApp");
	AutopatcherClient autopatcherClient;
	rakPeer->AttachPlugin(&autopatcherClient);
	FileListTransfer fileListTransfer;
	autopatcherClient.SetFileListTransferPlugin(&fileListTransfer);
	rakPeer->AttachPlugin(&fileListTransfer);

	printf("Hit 'q' to quit, 'p' to patch, 'c' to cancel the patch.\n");
	char ch;
	Packet *p;
	while (1)
	{
		p=rakPeer->Receive();
		while (p)
		{
			if (p->data[0]==ID_DISCONNECTION_NOTIFICATION)
				printf("ID_DISCONNECTION_NOTIFICATION\n");
			else if (p->data[0]==ID_CONNECTION_LOST)
				printf("ID_CONNECTION_LOST\n");
			else if (p->data[0]==ID_CONNECTION_REQUEST_ACCEPTED)
				printf("ID_CONNECTION_REQUEST_ACCEPTED\n");
			else if (p->data[0]==ID_CONNECTION_ATTEMPT_FAILED)
				printf("ID_CONNECTION_ATTEMPT_FAILED\n");
			else if (p->data[0]==ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR)
			{
				char buff[256];
				RakNet::BitStream temp(p->data, p->length, false);
				temp.IgnoreBits(8);
				stringCompressor->DecodeString(buff, 256, &temp);
				printf("ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR\n");
				printf("%s\n", buff);
			}
			else if (p->data[0]==ID_AUTOPATCHER_FINISHED)
				printf("ID_AUTOPATCHER_FINISHED\n");
			else if (p->data[0]==ID_AUTOPATCHER_RESTART_APPLICATION)
				printf("Launch \"AutopatcherClientRestarter.exe autopatcherRestart.txt\"\nQuit this application immediately after to unlock files.\n");

			rakPeer->DeallocatePacket(p);
			p=rakPeer->Receive();
		}


		if (kbhit())
		{
			ch=getch();
			if (ch=='q')
				break;
			else if (ch=='p')
			{
				char lastUpdateDate[128];
				char restartFile[512];
				strcpy(restartFile, appDir);
				strcat(restartFile, "/autopatcherRestart.txt");
				printf("Enter last update date (only newer updates retrieved) or nothing to get all updates\n");
				gets(lastUpdateDate);

				if (autopatcherClient.PatchApplication(appName, appDir, lastUpdateDate, rakPeer->GetPlayerIDFromIndex(0), &transferCallback, restartFile, argv[0]))
				{
					printf("Patching process starting.\n");
				}
				else
				{
					printf("Failed to start patching.\n");
				}
			}
			else if (ch=='c')
			{
				autopatcherClient.Clear();
				printf("Autopatcher cleared.\n");
			}

		}

		Sleep(30);
	}

	rakPeer->Disconnect(500,0);
	RakNetworkFactory::DestroyRakPeerInterface(rakPeer);
}
