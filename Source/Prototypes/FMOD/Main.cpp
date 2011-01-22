#include "..\..\CommonDeps\include\FMOD\fmod.h"
#include "..\..\CommonDeps\include\FMOD\fmod.hpp"
#include "..\..\CommonDeps\include\FMOD\fmod_errors.h"
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include "..\..\CommonDeps\include\FMOD\resource.h"


int main()
{
	FMOD_RESULT result;
	FMOD::System *system;
	const float DISTANCEFACTOR = 1.0f;
 
	result = FMOD::System_Create(&system);
		if (result != FMOD_OK)
		{
			printf("test for System_Create. \n");
			printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
			exit(-1);
		}
	
	result = system->init(100, FMOD_INIT_NORMAL, 0);
		if (result != FMOD_OK)
		{
			printf("test for System->Init. \n");
			printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
			exit(-1);
		}

		result = system->set3DSettings(1.0, DISTANCEFACTOR, 1.0f);
		
	// loading sounds ///////////////////////////////////////////////////////////////////////////////////////////////////
	FMOD::Sound *sound, *thunder, *Sound3d;
	result = system->createStream("../../Fmod/Sounds/Metallica - Star Wars Imperial March.mp3", FMOD_DEFAULT, 0, &sound);
	result = sound->setMode(FMOD_LOOP_NORMAL);
		if (result != FMOD_OK)
		{
			printf("test for sound stream song. \n");
			printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
			exit(-1);
		}
	
	result = system->createStream("../../Fmod/Sounds/Thunder.wav", FMOD_DEFAULT, 0, &thunder);
		if (result != FMOD_OK)
		{
			printf("test for sound stream thunder. \n");
			printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
			exit(-1);
		}
	
	result = system->createSound("../Sounds/ir_inter.wav", FMOD_3D, 0, &Sound3d);



	FMOD::Channel *channel, *channel2, *channel3;
	
	result = system->playSound(FMOD_CHANNEL_FREE, sound, false, &channel);
		if (result != FMOD_OK)
		{
			printf("test for System_Channel one. \n");
			printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
			exit(-1);
		}
	
	result = channel->setVolume(.25);
		if (result != FMOD_OK)
		{
			printf("test for System_Channel two. \n");
			printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
			exit(-1);
		}
	

	FMOD_VECTOR listenerpos = {0.0f, 0.0f, -1.0f * DISTANCEFACTOR };

	// text menu
	printf("Listen to the song,  \n");
	printf("Press 't' to fire Missle, \n");
	printf("Press 'l' to fire lazor from the left, \n");
	printf("Press 'c' to fire lazor from the center, \n");
	printf("Press 'r' to fire lazor from the right, \n");
	printf("Press 'q' and press enter to quit :)\n");
	//char s[256];
	int key;
	
	do 
	{
	//scanf("%s", s);
		if(kbhit())
		{
			key = getch();
		switch(key)
			{
				case 'l':
				case 'L':
					{
					printf("Sound should play from the Left. \n");

					FMOD_VECTOR pos = { -100.0f * DISTANCEFACTOR, 0.0f, 0.0f };
					FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };

					result = system->playSound(FMOD_CHANNEL_FREE, Sound3d, false, &channel3);
					result = channel3->set3DAttributes(&pos, &vel);
					result = channel3->setVolume(1.0);
					//result = channel3->set3DSpread(160);
					break;
					}

				case 'c':
				case 'C':
					printf("Sound should play from the Center. \n");
					result = system->playSound(FMOD_CHANNEL_FREE, Sound3d, false, &channel3);
					result = channel3->setVolume(1.0);
					break;

				case 'r':
				case 'R':
					printf("Sound should play from the Right. \n");

					break;


				case 't':
				case 'T': 
					result = system->playSound(FMOD_CHANNEL_FREE, thunder, false, &channel2);
					result = channel2->setVolume(.05);
					break;
				case 'q':
				case 'Q':
					result = sound->release();
					result = thunder->release();
					result = Sound3d->release();
					result = system->close();
					result = system->release();
					printf("good bye. \n");
					return 0;
				default:
					printf(" not a valid entry.\n");
					break;
			}
		}
	} while( true );
}