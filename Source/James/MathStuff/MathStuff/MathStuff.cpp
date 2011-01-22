// MathStuff.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

void DisplayHelp()
{
	printf(
		"RotDisk <radius> <angular velocity>\n"
		"TimeDif <days>\n"
		"CF <mass> <radius> <angular velocity>\n"
		"CFrpm <mass> <radius> <revolutions per minute>\n"
		"SetMass <name> Earth, Sun, Jupiter, Moon\n"
		"GSR\n"
		"TimeDil <distance factor> [observer time=1.0]\n"
		"Help (displays this)\n"
		"\nType \"Quit\" at anytime to exit this application\n"
		);
}

const double c_GravityForceStandard=9.812865328;
const double c_PI=3.14159265358979323846;
const double c_GravitationalConstant=6.67428e-11;
const double c_SpeedOfLight=299792458.0;  //exact speed of light in mps... http://en.wikipedia.org/wiki/Speed_of_light
const double c_SpeedOfLight_sqr=c_SpeedOfLight*c_SpeedOfLight;
const double c_EarthMeanRadius=6375416; //http://en.wikipedia.org/wiki/Schwarzschild_radius
//Some huge mass http://en.wikipedia.org/wiki/List_of_Solar_System_objects_by_size
const double c_EarthMass=5973.6e+21;
const double c_MoonMass=73.5e+21;
const double c_SunMassa=332837.0;
const double c_JupiterMassa=317.83;
/// \param Radius in meters
/// \param AngularVelocity in radians (designed to be a high number!)
double Get_TimeDilation_RotateDisk(double Radius,double AngularVelocity)
{
	//see http://en.wikipedia.org/wiki/Gravitational_time_dilation
	//This is the rotate disk version of sqrt(1-v^2/c^2) which is:
	//Td=sqrt(1-r^2 x w^2 / c^2) where
	//r = radius in meters
	//w = angular velocity in radians
	//c = speed of light in meters per second
	double r=Radius,w=AngularVelocity;
	double td=sqrt((1.0-((r*r)*(w*w))/c_SpeedOfLight_sqr));
	return td;
}

/// \param mass_a to avoid overflow we use a secondary scalar to be applied after the initial multiply between the gravitational constant and mass
double Get_Schwarzschild_radius(double mass,double mass_a)
{
	return  2.0*(c_GravitationalConstant*mass)*mass_a / c_SpeedOfLight_sqr;
}

/// \param tf is the coordinate time between events A and B for a fast-ticking observer at an arbitrarily large distance from the massive object
/// \param GSR is the Schwarzchild_radius (Use Get_Schwarzschild_radius)
/// \param RCO is the radial coordinate of the observer (like the classical distance from the center of the object, but is a Sc coordinate)
double Get_TimeDilation_OutsideSphere(double tf,double GSR,double RCO)
{
	double t0=tf*(sqrt(1.0-(GSR/RCO)));
	return t0;
}

//Body		Multiple M/S^2
//Sun		27.90	274.1
//Mercury	0.3770	3.703
//Venus		0.9032	8.872
//Earth		1.0		9.8226
//Moon		0.1655	1.625
//Mars		0.3895	3.728
//Jupiter	2.640	25.93
//Saturn	1.139	11.19 ?  I find this ironic due to the size of Saturn!
//Uranus	0.917	9.01
//Neptune	1.148	11.28

//Mass in Kilograms
double Get_CentripetalForce_Linear(double mass,double Radius,double LinearVelocity)
{
	return mass*(LinearVelocity*LinearVelocity)/Radius;
}

double Get_CentripetalForce(double mass,double Radius,double AngularVelocity)
{
	return mass*(AngularVelocity*AngularVelocity)*Radius;
}

#pragma warning(disable : 4996)
void CommandLineInterface()
{
	double a=c_GravitationalConstant;
	double TimeDilation;
	double Mass=c_EarthMass;
	double Mass_a=1.0;
	DisplayHelp();
	cout << endl;
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

		if (sscanf( input_line,"%s %s %s %s %s",command,str_1,str_2,str_3,str_4)>=1)
		//if (FillArguments(input_line,command,str_1,str_2,str_3,str_4)>=1)
		{

			if (!_strnicmp( input_line, "RotDisk", 7))
			{
				double Radius=atof(str_1);
				double AngularVelocity=atof(str_2);
				TimeDilation=Get_TimeDilation_RotateDisk(Radius,AngularVelocity);
				printf("Total time dilation = %f\n",TimeDilation);
			}
			else if (!_strnicmp( input_line, "TimeDif", 7))
			{
				double Days=atof(str_1);
				double Seconds= Days * 24.0 * 60.0 * 60.0;
				double Dif=Seconds-(TimeDilation*Seconds);
				printf("%f Days would be %f seconds off\n",Days,Dif);
			}
			else if (!_strnicmp( input_line, "CFrpm", 5))
			{
				double mass=atof(str_1);
				double Radius=atof(str_2);
				double rev=atof(str_3);
				double LinearVelocity=(2.0*c_PI*Radius)/(60.0/rev);
				double result=Get_CentripetalForce_Linear(mass,Radius,LinearVelocity);
				printf("Centripetal force c= %f\n",result);
				printf("Centripetal force g= %f m/s^2\n",result/mass);
				printf("%f g's \n",result/mass / c_GravityForceStandard );
			}
			else if (!_strnicmp( input_line, "CF", 2))
			{
				double mass=atof(str_1);
				double Radius=atof(str_2);
				double AngularVelocity=atof(str_3);
				double result=Get_CentripetalForce(mass,Radius,AngularVelocity);
				//check answer
				#if 0
				{
					double LinearVelocity=AngularVelocity*Radius;
					double result2=Get_CentripetalForce_Linear(mass,Radius,LinearVelocity);
					double dif=result2-result;
				}
				#endif
				printf("Centripetal force c= %f\n",result);
				printf("Centripetal force g= %f m/s^2\n",result/mass);
				printf("%f g's \n",result/mass / c_GravityForceStandard );
			}
			else if (!_strnicmp( input_line, "SetMass", 7))
			{
				if (stricmp(str_1,"Earth")==0)
				{
					Mass=c_EarthMass,Mass_a=1.0;
				}
				else if (stricmp(str_1,"Sun")==0)
				{
					Mass=c_EarthMass,Mass_a=c_SunMassa;
				}
				else if (stricmp(str_1,"Jupiter")==0)
				{
					Mass=c_EarthMass,Mass_a=c_JupiterMassa;
				}
				else if (stricmp(str_1,"Moon")==0)
				{
					Mass=c_MoonMass,Mass_a=1.0;
				}
				printf("Mass=%f,Mass_a=%f\n",Mass,Mass_a);
			}
			else if (!_strnicmp( input_line, "GSR", 3))
			{
				printf("Schwarzschild_radius=%f\n",Get_Schwarzschild_radius(Mass,Mass_a) );
			}
			else if (!_strnicmp( input_line, "TimeDil", 3))
			{
				double GSR=Get_Schwarzschild_radius(Mass,Mass_a);
				double Tf=(str_2[0]==0)?1.0:atof(str_2);
				double RCO=GSR*atof(str_1);
				TimeDilation=Get_TimeDilation_OutsideSphere(Tf,GSR,RCO);
				printf("Total time dilation = %f\n",TimeDilation);
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
	const char *DefaultFileName="TestMath.dat";
	//Set up any global init here
	//g_DefaultConsole=(ConsoleApp)LoadDefaults(DefaultFileName);

	//DisplayHelp();
	CommandLineInterface();

	//SaveOnExit:
	//SaveDefaults(DefaultFileName,(size_t)g_DefaultConsole);
}
