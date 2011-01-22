 #pragma config(Hubs,  S1, HTMotor,  HTMotor,  HTMotor,  HTMotor)
#pragma config(Motor,  motorA,          motorA,        tmotorNormal, PIDControl, encoder)
#pragma config(Motor,  motorB,          motorB,        tmotorNormal, PIDControl, encoder)
#pragma config(Motor,  motorC,          motorC,        tmotorNormal, PIDControl, encoder)
#pragma config(Motor,  mtr_S1_C1_1,     motorD,        tmotorNormal, openLoop, reversed, encoder)
#pragma config(Motor,  mtr_S1_C1_2,     motorE,        tmotorNormal, openLoop, reversed, encoder)
#pragma config(Motor,  mtr_S1_C2_1,     motorF,        tmotorNormal, openLoop, encoder)
#pragma config(Motor,  mtr_S1_C2_2,     motorG,        tmotorNormal, openLoop, encoder)
#pragma config(Motor,  mtr_S1_C3_1,     motorH,        tmotorNormal, openLoop)
#pragma config(Motor,  mtr_S1_C3_2,     motorI,        tmotorNormal, openLoop)
#pragma config(Motor,  mtr_S1_C4_1,     motorJ,        tmotorNormal, openLoop)
#pragma config(Motor,  mtr_S1_C4_2,     motorK,        tmotorNormal, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                           Tele-Operation Mode Code Template
//
// This file contains a template for simplified creation of an tele-op program for an FTC
// competition.
//
// You need to customize two functions with code unique to your specific robot.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "JoystickDriver.c"  //FTC include file to "handle" the Bluetooth messages.

#include "RobotMoves.c"  //Brandeis functions for basic robot motions

//#include "HTcommon.h"  //HiTechnic infrared sensor drivers
//#include "HTIRS-driver.h"  //HiTechnic infrared sensor functions

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                                    initializeRobot
//
// Prior to the start of tele-op mode, you may want to perform some initialization on your robot
// and the variables within your program.
//
// In most cases, you may not have to add any code to this function and it will remain "empty".
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

void initializeRobot()
{
  // Place code here to sinitialize servos to starting positions.
  // Sensors are automatically configured and setup by ROBOTC. They may need a brief time to stabilize.
  initializeDriveMotors();

  return;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                                         Main Task
//
// The following is the main code for the tele-op robot operation. Customize as appropriate for
// your specific robot.
//
// Game controller / joystick information is sent periodically (about every 50 milliseconds) from
// the FMS (Field Management System) to the robot. Most tele-op programs will follow the following
// logic:
//   1. Loop forever repeating the following actions:
//   2. Get the latest game controller / joystick settings that have been received from the PC.
//   3. Perform appropriate actions based on the joystick + buttons settings. This is usually a
//      simple action:
//      *  Joystick values are usually directly translated into power levels for a motor or
//         position of a servo.
//      *  Buttons are usually used to start/stop a motor or cause a servo to move to a specific
//         position.
//   4. Repeat the loop.
//
// Your program needs to continuously loop because you need to continuously respond to changes in
// the game controller settings.
//
// At the end of the tele-op period, the FMS will autonmatically abort (stop) execution of the program.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

task main()
{
// TURN THIS OFF ONLY DURING DEVELOPMENT
  waitForStart();   // wait for start of tele-op phase
//TURN THIS BACK ON WHEN USING FIELD MANAGEMENT SYSTEM */
//CHANGE waitForStart() also in Joystick.c !!!!!


	  ///////////////////////////////////////////////////////////
	  ///////////////////////////////////////////////////////////
	  ////                                                   ////
	  ////      Add your robot specific tele-op code here.   ////
	  ////                                                   ////
	  ///////////////////////////////////////////////////////////
	  ///////////////////////////////////////////////////////////

    // Insert code to have servos and motors respond to joystick and button values.

    // Look in the ROBOTC samples folder for programs that may be similar to what you want to perform.
    // You may be able to find "snippets" of code that are similar to the functions that you want to
    // perform.
  nSyncedMotors = synchBC;
  nSyncedTurnRatio = 100;
  while (true)
  {

    getJoystickSettings(joystick);  // Get the latest game controller / joystick settings that have been received from the PC.

    if(joystick.joy1_y1 < 30 && joystick.joy1_y1 > -30)
    {
      motor[motorD] = 0;
    }
    else
    {
      motor[motorD] = joystick.joy1_y1/2;
    }

    if(joystick.joy1_y2 < 30 && joystick.joy1_y2 > -30)
    {
      motor[motorE] = 0;
    }
    else
    {
      motor[motorE] = -joystick.joy1_y2/2;
    }


    if (joystick.joy1_Buttons == 64) // 1B7
    {
      motor[motorF]=100;
    }
    else motor[motorF]=0;

    if (joystick.joy1_Buttons == 16) // 1B5
    {
      motor[motorF]=-100;
    }
    else motor[motorF]=0;

    if (joystick.joy1_Buttons == 128)// 1B8
    {
      motor[motorG] = -100;
    }
    else motor[motorG] = 0;


    if (joystick.joy1_Buttons == 32)// 1B6
    {
      motor[motorG] = 100;
    }

    else motor[motorG] = 0;

    if (joystick.joy2_Buttons == 64) // 2B7
    {
      motor[motorA]=-80;
    }
    else motor[motorA]=0;

    if (joystick.joy2_Buttons == 16) // 2B5
    {
      motor[motorA]=80;
    }
    else motor[motorA]=0;

    if (joystick.joy2_Buttons == 128)// B8
    {
      motor[motorK] = 15;
    }
    else motor[motorK] = 0;


    if (joystick.joy2_Buttons == 32)// B6
    {
      motor[motorK] = -15;
    }

    else motor[motorK] = 0;
   /* if(joystick.joy2_Buttons == 1) //B1
    {
      motor[motorA] = -100;
    }
    else
    {
      motor[motorA] = 0;
    }
    if(joystick.joy2_Buttons == 1) //B1
    {
      motor[motorB] = -100;
    }
    else
    {
      motor[motorA] = 0;
    }*/
    if(joystick.joy2_Buttons == 2) //B2
    {
      motor[motorH]=-100;
      motor[motorI]=100;
    }
    if(joystick.joy2_Buttons == 4) //B3
    {
      motor[motorH]=0;
      motor[motorI]=0;
    }
    if(joystick.joy2_Buttons == 8) //B4
    {
      motor[motorJ]=0;
    }
    if(joystick.joy2_Buttons == 256) //B9
    {
      motor[motorJ]=-10;
    }
    if(joystick.joy2_Buttons == 512) //B10
    {
      motor[motorJ]=10;
    }
    if(joystick.joy1_Buttons == 2) //1B2
    {
      motor[motorB] = 100;
      //motor[motorC] = 100;
    }
        if(joystick.joy1_Buttons == 8) //1B4
    {
      motor[motorB] = -100;
      motor[motorC] = -100;
    }
    if(joystick.joy1_Buttons == 4) //1B3
    {
      motor[motorB] = 0;
      //motor[motorC] = 0;
    }

 }
}
