#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "portaudio.h"
#include <conio.h>
#include "RakPeerInterface.h"
#include "PacketEnumerations.h"
#include "RakNetworkFactory.h"
#include "RakVoice.h"
#include "RakNetStatistics.h"

/// To test sending to myself
//#define _TEST_LOOPBACK

#if 0
// speex doesn't like floats between 0 and 1, which is what portaudio returns so paFloat32 doesn't work
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#endif

// Reads and writes per second of the sound data
// Speex only supports these 3 values
#define SAMPLE_RATE  (8000)
//#define SAMPLE_RATE  (16000)
//#define SAMPLE_RATE  (32000)

RakPeerInterface *rakPeer;

// I think one buffer has to be full (of samples) before you hear the sound.
// So higher frames per buffer means that there will be a larger latency before you hear the sound
// However, it would lock and unlock the buffer more often, hindering performance.
#define FRAMES_PER_BUFFER  (2048 / (32000 / SAMPLE_RATE))

bool mute;
RakVoice rakVoice;

// inputBuffer and outputBuffer is an array of SAMPLE of count framesPerBuffer
// A sample is one unit of sound.
// The sample rate is the number of sound samples taken per second
// I think one frame is a set of samples equal to the number of channels.  I'm not sure though or how that sample is arranged.
static int PACallback( void *inputBuffer, void *outputBuffer,
						  unsigned long framesPerBuffer,
						  PaTimestamp outTime, void *userData )
{
	/*
	// If there is input then inputBuffer is non-null.
	if (inputBuffer && !mute)
	{
		// Test port audio feedback
		memcpy(outputBuffer, inputBuffer, sizeof(SAMPLE) * framesPerBuffer);
	}
	else
	{
		// If there is not input then write silence.
		unsigned i;
		SAMPLE *out = (SAMPLE *)outputBuffer;

		for (i=0; i < framesPerBuffer; i++)
			out[i]=SAMPLE_SILENCE;
	}
	*/

	if (inputBuffer && !mute)
	{
		// TODO - if the input data is mostly silence, don't send and save the bandwidth.

#ifndef _TEST_LOOPBACK
		unsigned i;

		for (i=0; i < rakPeer->GetMaximumNumberOfPeers(); i++)
		{
			rakVoice.SendFrame(rakPeer->GetPlayerIDFromIndex(i), inputBuffer);
		}
#else
		rakVoice.SendFrame(UNASSIGNED_PLAYER_ID, inputBuffer);
#endif
	}

	rakVoice.ReceiveFrame(outputBuffer);

	return 0;
}

int main(void)
{
	PortAudioStream *stream;
	PaError    err;
	mute=false;

	bool quit;
	char ch;

	printf("A sample on how to use RakVoice. You need a microphone for this sample.\n");
	printf("RakVoice relies on Speex for voice encoding and decoding.\n");
	printf("See DependentExtensions/RakVoice/speex-1.1.12 for speex projects.\n");
	printf("For windows, I had to define HAVE_CONFIG_H, include win32/config.h,\n");
	printf("and include the files under libspeex, except those that start with test.\n");
	printf("PortAudio is also included and is used to read and write audio data.  You\n");
	printf("can substitute whatever you want if you do not want to use portaudio.\n");
	printf("Difficulty: Advanced\n\n");


	char port[256];
	rakPeer = RakNetworkFactory::GetRakPeerInterface();
	printf("Enter local port: ");
	gets(port);
	if (port[0]==0)
		strcpy(port, "60000");
	rakPeer->Initialize(4,atoi(port), 30, 0);
	rakPeer->SetMaximumIncomingConnections(4);
	rakPeer->AttachPlugin(&rakVoice);
	rakVoice.Init(sizeof(SAMPLE)==2, SAMPLE_RATE, FRAMES_PER_BUFFER*sizeof(SAMPLE));

	err = Pa_Initialize();
	if( err != paNoError ) goto error;
	
	err = Pa_OpenStream(
		&stream,
		Pa_GetDefaultInputDeviceID(),
		1, // Num channels, whatever the fuck that means
		PA_SAMPLE_TYPE,
		NULL,
		Pa_GetDefaultOutputDeviceID(),
		1, // Num channels
		PA_SAMPLE_TYPE,
		NULL,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,            /* frames per buffer */
		0,               /* number of buffers, if zero then use default minimum */
		0, /* paDitherOff, // flags */
		PACallback,
		0 );

	if( err != paNoError ) goto error;

	err = Pa_StartStream( stream );
	if( err != paNoError ) goto error;
	
    
	Packet *p;
	quit=false;
	printf("(Q)uit. (C)onnect. (D)isconnect. (M)ute. ' ' for stats.\n");
	while (!quit)
	{
		if (kbhit())
		{
			ch=getch();
			if (ch=='y')
			{
				quit=true;
			}
			else if (ch=='c')
			{
				char ip[256];
				printf("Enter IP of remote system: ");
				gets(ip);
				if (ip[0]==0)
					strcpy(ip, "127.0.0.1");
				printf("Enter port of remote system: ");
				gets(port);
				if (port[0]==0)
					strcpy(port, "60000");
				rakPeer->Connect(ip, atoi(port), 0,0);
			}
			else if (ch=='m')
			{
				mute=!mute;
				if (mute)
					printf("Now muted.\n");
				else
					printf("No longer muted.\n");
			}
			else if (ch=='d')
			{
				rakPeer->Disconnect(100,0);
			}
			else if (ch==' ')
			{
				char message[1024];
				RakNetStatisticsStruct *rss=rakPeer->GetStatistics(rakPeer->GetPlayerIDFromIndex(0));
				StatisticsToString(rss, message, 2);
				printf("%s", message);
			}
			else if (ch=='q')
				quit=true;
			ch=0;
		}

		p=rakPeer->Receive();
		while (p)
		{
			if (p->data[0]==ID_CONNECTION_REQUEST_ACCEPTED)
			{
				printf("ID_CONNECTION_REQUEST_ACCEPTED from %s\n", p->playerId.ToString());
				rakVoice.RequestVoiceChannel(p->playerId);
			}
			else if (p->data[0]==ID_RAKVOICE_OPEN_CHANNEL_REQUEST || p->data[0]==ID_RAKVOICE_OPEN_CHANNEL_REPLY)
			{
				printf("Got new channel from %s\n", p->playerId.ToString());
			}

			rakPeer->DeallocatePacket(p);
			p=rakPeer->Receive();
		}


		Pa_Sleep( 30 );
//		Pa_Sleep( 1000 );
	}

	err = Pa_CloseStream( stream );
	if( err != paNoError ) goto error;

	Pa_Terminate();
	return 0;

error:
	Pa_Terminate();
	fprintf( stderr, "An error occured while using the portaudio stream\n" );
	fprintf( stderr, "Error number: %d\n", err );
	fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
	return -1;
}
