
#include "stdafx.h"
#include <xmmintrin.h> /* Streaming SIMD Extensions Intrinsics include file */

void DisplayHelp()
{
	printf(
		"quit    - quit this program\n"
		);

}

struct  ShuffleElements
{
	byte w,x,y,z;
};

ShuffleElements ConvertShuffle(DWORD ImmediateValue)
{
	ShuffleElements ret;
	ret.w=ImmediateValue & 3;
	ImmediateValue>>=2;
	ret.x=ImmediateValue & 3;
	ImmediateValue>>=2;
	ret.y=ImmediateValue & 3;
	ImmediateValue>>=2;
	ret.z=ImmediateValue & 3;
	ImmediateValue>>=2;
	return ret;
}

void CommandLineInterface()
{
	printf("\nReady.\n");
	static char input_line[128];

	while (printf(">"),gets(input_line))
	{
		static char		command[32];
		static char		str_1[64];
		static char		str_2[64];
		static char		str_3[64];
		static char		str_4[64];

		static char		g_char = 0;

		command[0] = '\0';
		str_1[0] = '\0';
		str_2[0] = '\0';
		str_3[0] = '\0';
		str_4[0] = '\0';

		if (sscanf( input_line,"%s %s %s %s %s",command,str_1,str_2,str_3,str_4)>=1)
		{
			if (!strnicmp(input_line,"convert",7))
			{
				char *stopstring;
				ShuffleElements _=ConvertShuffle(strtoul(str_1,&stopstring,0));
				printf("_MM_SHUFFLE(%d,%d,%d,%d)\n",_.z,_.y,_.x,_.w);
				printf("Verify=%x \n",_MM_SHUFFLE(_.z,_.y,_.x,_.w));
			}
			else if (!strnicmp(input_line,"help",4))
			{
				DisplayHelp();
			}
			else if (!strnicmp( input_line, "quit", 4))
			{
				break;
			}
			else
			{
				printf("huh? - try \"help\"\n");
			}
		}
	}
}
void main()
{
	DisplayHelp();
	CommandLineInterface();
}
