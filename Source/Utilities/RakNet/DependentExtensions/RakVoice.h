/// \file
/// \brief Voice compression and transmission interface
///
/// This file is part of RakNet Copyright 2003 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.rakkarsoft.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#ifndef __RAK_VOICE_H
#define __RAK_VOICE_H

class RakPeerInterface;
#include "NetworkTypes.h"
#include "PluginInterface.h"
#include "DS_OrderedList.h"

// How many frames large to make the circular buffers in the VoiceChannel structure
#define FRAME_OUTGOING_BUFFER_COUNT 100
#define FRAME_INCOMING_BUFFER_COUNT 100

/// \internal
struct VoiceChannel
{
	PlayerID playerId;
	void *enc_state;
	void *dec_state;
	bool remoteIsShortSampleType;
	unsigned short remoteSampleRate;
	
	// Circular buffer of unencoded sound data read from the user.
	char *outgoingBuffer;
	// Each frame sent to speex requires this many samples, of whatever size you are using.
	int speexOutgoingFrameSampleCount;
	// Index in is bytes.
	// Write index points to the next byte to write to, which must be free.
	unsigned outgoingReadIndex, outgoingWriteIndex;
	bool bufferOutput;
	bool copiedOutgoingBufferToBufferedOutput;
	unsigned short outgoingMessageNumber;

	// Circular buffer of unencoded sound data to be passed to the user.  Each element in the buffer is of size bufferSizeBytes bytes.
	char *incomingBuffer;
	int speexIncomingFrameSampleCount;
	unsigned incomingReadIndex, incomingWriteIndex;	// Index in bytes
	unsigned short incomingMessageNumber;  // The ID_VOICE message number we expect to get.  Used to drop out of order and detect how many missing packets in a sequence

	RakNetTime lastSend;
};
int VoiceChannelComp( const PlayerID &key, VoiceChannel * const &data );

/// \brief Encodes, decodes, and transmits voice data.
/// Voice compression and transmission interface
/// \addtogroup Plugins
class RakVoice : public PluginInterface
{
public:
	RakVoice();
	~RakVoice();

	// --------------------------------------------------------------------------------------------
	// User functions
	// --------------------------------------------------------------------------------------------

	/// \brief Starts RakVoice
	/// \param[in] sampleTypeIsShort true if your sample types are unsigned shorts - otherwise they must be floats
	/// \param[in] speexSampleRate 8000, 16000, or 32000
	/// \param[in] bufferSizeBytes How many bytes long inputBuffer and outputBuffer are in SendFrame and ReceiveFrame are.  Should be your sample size * the number of samples to encode at once.
	void Init(bool sampleTypeIsShort, unsigned short speexSampleRate, unsigned bufferSizeBytes);

	/// Shuts down RakVoice
	void Deinit(void);
	
	/// \brief Opens a channel to another connected system
	/// You will get ID_RAKVOICE_OPEN_CHANNEL_REPLY on success
	/// \param[in] recipient Which system to open a channel to
	void RequestVoiceChannel(PlayerID recipient);

	/// \brief Closes an existing voice channel.
	/// Other system will get ID_RAKVOICE_CLOSE_CHANNEL
	/// \param[in] recipient Which system to close a channel with
	void CloseVoiceChannel(PlayerID recipient);

	/// \brief Closes all existing voice channels
	/// Other systems will get ID_RAKVOICE_CLOSE_CHANNEL
	void CloseAllChannels(void);

	/// \brief Sends voice data to a system on an open channel
	/// \pre \a recipient must refer to a system with an open channel via RequestVoiceChannel
	/// \param[in] recipient The system to send voice data to
	/// \param[in] inputBuffer The voice data.  The size of inputBuffer should be what was specified as bufferSizeBytes in Init
	bool SendFrame(PlayerID recipient, void *inputBuffer);

	/// \brief Gets decoded voice data, from one or more remote senders
	/// \param[out] outputBuffer The voice data.  The size of outputBuffer should be what was specified as bufferSizeBytes in Init
	void ReceiveFrame(void *outputBuffer);

	// --------------------------------------------------------------------------------------------
	// Message handling functions
	// --------------------------------------------------------------------------------------------
	virtual void OnAttach(RakPeerInterface *peer);
	virtual void OnDisconnect(RakPeerInterface *peer);
	virtual void Update(RakPeerInterface *peer);
	virtual PluginReceiveResult OnReceive(RakPeerInterface *peer, Packet *packet);
	virtual void OnCloseConnection(RakPeerInterface *peer, PlayerID playerId);
protected:
	void OnOpenChannelRequest(RakPeerInterface *peer, Packet *packet);
	void OnOpenChannelReply(RakPeerInterface *peer, Packet *packet);
	void OnVoiceData(RakPeerInterface *peer, Packet *packet);
	void OpenChannel(RakPeerInterface *peer, Packet *packet);
	void FreeChannelMemory(PlayerID recipient);
	void FreeChannelMemory(unsigned index);
	void WriteOutputToChannel(VoiceChannel *channel, char *dataToWrite);
	
	RakPeerInterface *rakPeer;
	DataStructures::OrderedList<PlayerID, VoiceChannel*, VoiceChannelComp> voiceChannels;
	bool shortSampleType;
	int sampleRate;
	int sampleSize; // Size of each sample - 2 if shortSampleType==true - 4 otherwise
	unsigned bufferSizeBytes;
	float *bufferedOutput;
	unsigned bufferedOutputCount;
	bool zeroBufferedOutput;
};

#endif
