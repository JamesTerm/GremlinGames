
#include "stdafx.h"
#include "../../Common/FileSelectionLib/Common.h"
#include "../../../GG_Framework/ui/Audio/GG_Framework.UI.Audio.h"

enum ConsoleApp
{
	eConsoleMain
}	g_DefaultConsole=eConsoleMain;

const double PI=3.1415926535897;

//Use PI/2 to init the cycle on zero for rho
double SineInfluence(double &rho,double freq_hz=1000.0,double SampleRate=48000.0,double amplitude=1.0)
{
	double			 theta, scale, pi2;

	pi2 = PI;
	theta = freq_hz / SampleRate;
	theta *= (pi2 * 2.0);
	pi2 *= 2.0;

	scale = amplitude;

	double Sample= cos( rho ) * scale;
	rho += theta;
	if ( rho > pi2 )
		rho -= pi2;
	return Sample;
}

void DisplayHelp()
{
	DisplayFileSelectionHelp();
	printf(
		"SetLoop <start> <stop> <duration>\n"
		"Use stop=0 for reset and -1 to loop entire clip\n"
		"Play\n"
		"LoopTest\n"
		"Stream\n"
		"Latency\n"
		"PauseTest\n"
		"PanNode\n"
		"PanCam <0 pan 1 forward 2 up>\n"
		"Volume <0-1>\n"
		"VolSound <0-1>\n"
		"VolMusic <0-1>\n"
		"Help (displays this)\n"
		"\nType \"Quit\" at anytime to exit this application\n"
		);
}

using namespace GG_Framework::UI::Audio;
using namespace GG_Framework::Base;

class Updater : public OpenThreads::Thread
{
public:
	bool Terminate;
	Updater() : Terminate(false) {}
	virtual void run()
	{
		size_t i=0;
		while(!Terminate)
		{
			//printf("\r%d    ",i++);
			SOUNDSYSTEM.SystemFrameUpdate();
			Sleep(33);
		}
	}
};

ISound *SetUpSound(const size_t LoopStart,const size_t LoopEnd,int mode=f3DSound|fUseSoftware)
{
	//Note using loop points requires using the software mode, and I would not recommend using this mode for other cases, because
	//I have found quality loss with it (e.g. try quickbeep)
	//You can loop the whole file (which is ideal for music) so I don't bother using software for this case (ie. end==-1)
	if ((LoopEnd!=0)&&(LoopEnd!=-1))
		mode|=fUseSoftware;
	ISound *sound;
	if (mode & f3DSound)
		sound=SOUNDSYSTEM.CreateSound(InputFileName,(SoundMode_Flags)mode);
	else if (mode & f2DSound)
		sound=SOUNDSYSTEM.CreateStream(InputFileName);
	else
		assert(false);
	if (LoopEnd!=0)
	{
		//Setting end to -1 is a way to loop the entire sound by not setting the points
		if (LoopEnd!=-1)
			sound->setLoopPoints(LoopStart,LoopEnd);
		sound->setMode(fLoopNormal);
	}
	else
		sound->setMode(fLoopOff);
	return sound;
}

#pragma warning(disable : 4996)
void CommandLineInterface()
{
	double TimeOut=FLT_MAX;
	size_t LoopStart=0;
	size_t LoopEnd=0;

	size_t FileSelection=InitFileSelection();

	cout << endl;
	ShowCurrentSelection();
	cout << "Ready." << endl;

   	char input_line[128];
    while (cout << ">",cin.getline(input_line,128))
    {
		char		command[32];
		char		str_1[MAX_PATH];
		char		str_2[MAX_PATH];
		char		str_3[MAX_PATH];
		char		str_4[MAX_PATH];

		command[0]=0;
		str_1[0]=0;
		str_2[0]=0;
		str_3[0]=0;
		str_4[0]=0;

		//if (sscanf( input_line,"%s %s %s %s %s",command,str_1,str_2,str_3,str_4)>=1)
		if (FillArguments(input_line,command,str_1,str_2,str_3,str_4)>=1)
		{
			if (HandleFileSelectionCommands(FileSelection,input_line,str_1))
				continue;

			else if (!_strnicmp( input_line, "SetLoop", 7))
			{
				LoopStart=Hex2Long(str_1);
				LoopEnd=Hex2Long(str_2);
				TimeOut=atof(str_3);
				if (LoopEnd==0)
					TimeOut=FLT_MAX;
			}
			else if (!_strnicmp( input_line, "Play", 4))
			{
				ISound *sound=SetUpSound(LoopStart,LoopEnd);
				//enable to test sound caching
				//ISound *sound1=SetUpSound(LoopStart,LoopEnd);
				//ISound *sound2=SetUpSound(LoopStart,LoopEnd);
				IChannel *channel=NULL;
				SOUNDSYSTEM.PlaySound(sound,false,&channel);
				//Sleep(1000);
				//SOUNDSYSTEM.PlaySound(sound1,false);
				//Sleep(1000);
				//SOUNDSYSTEM.PlaySound(sound2,false);

				#if 0
				bool isPlaying;
				size_t i=0;
				while ((channel->IsPlaying(isPlaying)==true)&&(isPlaying)&&(((double)i*0.033)<TimeOut))
				{
					Sleep(33);
					printf("\r%d     ",i++);
					SOUNDSYSTEM.SystemFrameUpdate();
				}
				printf("\rComplete.\n");
				#endif
			}
			else if (!_strnicmp( input_line, "LoopTest", 8))
			{
				InputFileName="Sounds\\LoopTest.WAV";
				ISound *sound=SetUpSound(0,0,f3DSound|fUseSoftware);
				sound->setMode(fLoopNormal);
				IChannel *channel=NULL;
				SOUNDSYSTEM.PlaySound(sound,false,&channel);
				//Now set up loop points
				bool TestChannelLoopPoints=channel->setLoopPoints(48000,96000);
				bool isPlaying;
				size_t i=0;
				TimeOut=4.0;
				while ((channel->IsPlaying(isPlaying)==true)&&(isPlaying)&&(((double)i*0.033)<TimeOut))
				{
					Sleep(33);
					printf("\r%d     ",i++);
				}
				i=0;
				printf("\rDecay!    ");
				channel->setLoopCount(0);
				while ((channel->IsPlaying(isPlaying)==true)&&(isPlaying)&&(((double)i*0.033)<TimeOut))
				{
					Sleep(33);
					printf("\r%d     ",i++);
				}

				printf("\rComplete.\n");
			}

			else if (!_strnicmp( input_line, "Stream", 5))
			{
				ISound *sound=SetUpSound(LoopStart,LoopEnd,f2DSound);
				IChannel *channel=NULL;
				SOUNDSYSTEM.PlaySound(sound,false,&channel);
			}

			else if (!_strnicmp( input_line, "PanNode", 7))
			{
				ISound *sound=SetUpSound(LoopStart,LoopEnd);
				IChannel *channel=NULL;
				SOUNDSYSTEM.PlaySound(sound,false,&channel);

				Vec3 NodePos(0.0f,0.0f,0.0f);
				Vec3 Velocity(0.0f,0.0f,0.0f);
                
				bool isPlaying;
				size_t i=0;
				double rho=PI/2;
				while ((channel->IsPlaying(isPlaying)==true)&&(isPlaying)&&(((double)i++*0.033)<TimeOut))
				{
					Sleep(33);
					float Prevpos=NodePos.x;
					NodePos.x=(float)(SineInfluence(rho,1000.0,48000.0,100.0));
					//NodePos.x=1000.0;
					Velocity.x=(NodePos.x-Prevpos) / 0.033f;
					printf("\r%f,%f                           ",NodePos.x,Velocity.x);
					//DebugOutput("%f,%f\n",NodePos.x,Velocity.x);
					channel->Set3DAttributes(&NodePos,&Velocity);
				}
				printf("\rComplete.\n");
			}
			else if (!_strnicmp( input_line, "PanCam", 6))
			{
				ISound *sound=SetUpSound(LoopStart,LoopEnd);
				IChannel *channel=NULL;
				SOUNDSYSTEM.PlaySound(sound,false,&channel);

				Vec3 NodePos(0.0f,0.0f,0.0f);
				Vec3 Velocity(0.0f,0.0f,0.0f);

				enum WhatToTest
				{eTestPan,eTestForward,eTestUp} TestThis;

				TestThis=eTestPan;
				if (str_1[0]=='1')
					TestThis=eTestForward;
				else if (str_1[0]=='2')
					TestThis=eTestUp;

				//Max value for orientation is 1.0
				double Amplitude=TestThis==eTestPan?100.0:1.0;

				if (TestThis!=eTestPan)
				{
					//Place the node sound out to a place where "turning the head" would change where the sound is heard
					NodePos.z=10;
					channel->Set3DAttributes(&NodePos,&Velocity);
					if (TestThis==eTestUp)
					{
						//I'm cheating... I'll position the forward to face the left to the axis can be a fixed point and remain perpendicular
						//to the y manipulation.
						NodePos.x=-1.0;
						SOUNDSYSTEM.Set3DListenerAttributes(0,NULL,NULL,&NodePos);
					}
				}

				bool isPlaying;
				size_t i=0;
				double rho=PI/2;
				while ((channel->IsPlaying(isPlaying)==true)&&(isPlaying)&&(((double)i++*0.033)<TimeOut))
				{
					Sleep(33);
					float Prevpos=NodePos.x;
					if (TestThis!=eTestUp)
					{
						NodePos.x=(float)(SineInfluence(rho,1000.0,48000.0,Amplitude));
						//For testing forward the z coordinate is set to a value which = 1.0 distance from center
						//which is sqrt(distance^2 - x^2)... a modified form of the Pythagorean theorem 
						if (TestThis==eTestForward)
							NodePos.z=(float)sqrt(1.0-(NodePos.x*NodePos.x));
						//DebugOutput("%f %f\n",NodePos.x,NodePos.z);
					}
					else
					{
						NodePos.y=(float)(SineInfluence(rho,1000.0,48000.0,Amplitude));
						NodePos.z=(float)sqrt(1.0-(NodePos.y*NodePos.y));
						//DebugOutput("%f %f\n",NodePos.y,NodePos.z);
					}

					Velocity.x=(NodePos.x-Prevpos) * 0.033f;
					printf("\r%f,%f,%f                                        ",NodePos.x,NodePos.y,NodePos.z);
					switch (TestThis)
					{
					case eTestPan:
						SOUNDSYSTEM.Set3DListenerAttributes(0,&NodePos,&Velocity);
						break;
					case eTestForward:
						SOUNDSYSTEM.Set3DListenerAttributes(0,NULL,NULL,&NodePos);
						break;
					case eTestUp:
						SOUNDSYSTEM.Set3DListenerAttributes(0,NULL,NULL,NULL,&NodePos);
						break;
					}
				}
				//ensure this is set back to center
				NodePos.x=Velocity.x=0.0f;
				Vec3 Forward(0.0f,0.0f,1.0f);
				Vec3 Up(0.0f,1.0f,0.0f);
				SOUNDSYSTEM.Set3DListenerAttributes(0,&NodePos,&Velocity,&Forward,&Up);

				printf("\rComplete.                                          \n");
			}

			else if (!_strnicmp( input_line, "PauseTest", 9))
			{
				ISound *sound=SetUpSound(LoopStart,LoopEnd);
				IChannel *channel=NULL;
				SOUNDSYSTEM.PlaySound(sound,true,&channel);
				size_t PauseCount=0;
				bool isPlaying;
				bool isPaused;
				size_t i=0;
				while (
							(
								((channel->GetPaused(isPaused)==true)&&(isPaused)) ||
								((channel->IsPlaying(isPlaying)==true)&&(isPlaying))
							) &&
							(((double)i*0.033)<TimeOut)
						)
				{
					Sleep(33);
					if (isPaused)
					{
						printf("\rPaused-%d     ",PauseCount++);
						if (PauseCount>20)
						{
							channel->SetPaused(false);
							printf("\n");
						}
					}
					else
						printf("\r%d     ",i++);
				}
				printf("\rComplete.\n");
			}

			else if (!_strnicmp( input_line, "Latency", 7))
			{
				InputFileName="Sounds\\QuickBleep.wav";
				ISound *sound=SetUpSound(LoopStart,LoopEnd);
				for (size_t t=0;t<5;t++)
				{
					printf("\rReady...     ");
					Sleep(500);
					printf("\r||||||||||||");
					Sleep(33);
					SOUNDSYSTEM.PlaySound(sound,false);
					printf("\r            ");
					for (size_t i=0;i<30;i++)
					{
						Sleep(33);
						printf("\r%d     ",i);
					}
					printf("\rComplete.");
					Sleep(33);
				}
				printf("\n");
			}
			else if (!_strnicmp( input_line, "Volume", 4))
			{
				double volume=atof(str_1);
				SOUNDSYSTEM.SetMasterVolume(volume);
				printf("Volume Set To %f\n",volume);
			}
			else if (!_strnicmp( input_line, "VolSound", 4))
			{
				double volume=atof(str_1);
				SOUNDSYSTEM.SetSoundsVolume(volume);
				printf("Volume Set To %f\n",volume);
			}
			else if (!_strnicmp( input_line, "VolMusic", 4))
			{
				double volume=atof(str_1);
				SOUNDSYSTEM.SetStreamsVolume(volume);
				printf("Volume Set To %f\n",volume);
			}

			else if (!_strnicmp( input_line, "Help", 4))
				DisplayHelp();
			else if (!_strnicmp( input_line, "Quit", 4))
				break;
			else
				cout << "huh? - try \"help\"" << endl;
		}
    }
}


void main(void)
{
	const char *DefaultFileName="FMODTestSound.dat";
	//Set up any global init here
	g_DefaultConsole=(ConsoleApp)LoadDefaults(DefaultFileName);

	InitFileSelection();

	DisplayHelp();
	ShowFileSelections();
	SoundSystem sound;
	//SoundSystem_Mock sound;
	Updater mythread;
	mythread.Init();
	mythread.start();
	CommandLineInterface();
	//Yeah crude... should see how it is suppose to work
	mythread.Terminate=true;
	Sleep(50);

//SaveOnExit:
	SaveDefaults(DefaultFileName,(size_t)g_DefaultConsole);
}
