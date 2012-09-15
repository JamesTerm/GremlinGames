Hopefully you can simply launch AI_Tester.exe and then at the command prompt:
>test

After driving around with it you can later do

>test followgod

Which shows a "godship" and both tank and nona drive autonomous abilities to pursue a waypoint.


To control the robot use the following keys:
a, d    to control yaw rotation
w       to move forward
s       to move reverse
x       to stop  (Note: keyboard control is different than joystick, and is not used in actual robot)
q, r    to strafe (or use left and right arrow keys)
g       toggles slide mode
f9      to show dump of speeds and mode (speed is measured in feet per second)
f3      toggles full screen display


The AI Tester is a command line driven tester to test AI code, which was originally written to develop AI for a game we started making back in 2007.  I have other test cases which show the game tests as well you can do something like:
test help

To show the other tests, the 2011 and 2012 robot in here as well.

There are a list of some other commands... if you use the zoom command, just beware that there is a UI bug with the treads and at some point I'll address that.  The treads are the lines that help visualize the speed of the wheels... the diameter of the wheels are huge on purpose, as they try to be aesthetically pleasing to the eye as the pixels match the UI dimensions.

The only other thing to point out is the lua and xml scripts... When using the nona robot it will read the TestNonaRobot.lua script during runtime...

The LUA allows you to tweak the ramping functions by limiting the maximum amount of force or torque you may apply to the robot at any time.  It also allows you to specify dimensions turning speeds etc.  
Use the the userPrefs.xml to change the actual controls, and to see/change other keyboard assignments. 


Slide Mode:
This is really a simpler and first iteration of dealing with the fundemental workings of basic physics as if the "ship is in space" where when you apply a force to an entity body it will travel in that direction until an opposing force says otherwise.  It now supports a button held down (currently set to button 5)... so say the driver pushes forward while holding down the button... the robot moves in that direction and then the driver can use the yaw control to rotate the robot while still traveling in the same direction until he releases the button.  

Oh yes... and you can plug in and assign multiple joysticks to the AI Tester... in its current default axis 0 (ordinal) is the yaw, and axis 1 is the forward reverse.  

A few words about Nona LUA settings:

This goes beyond what we have done last year where you can soup up the robot to run how you would like it to run... now then keep in mind just because you can set it up that way does not mean it is mechanically (or practically) possible.  The main settings for this are:
1.  Mass-  (setting the robot weight)... the smaller the weight the faster it can accelerate as the math is all laid out below...
2. Drive/Kicker CoF Omni Radial- currently set at 85%... unfortunately this is probably lower around 80, but we may tweak with this to get it as high as possible without skidding... the higher this the higher the acceleration on all
3.  Kicker Weight Bias-  This is probably the most important variable to change and experement... the more weight this has the faster the kicker wheel can accelerate, BUT the price costs less traction on the drive, so the idea here is to find a good balance between the two... and also find out from Parker what we are able to do.  :)
4.  is_closed-  You can use 'yes' 'no' 0 1 'y' and 'n'... I probably use 0 and 1 the most... if these are 0 (for swerve and kicker) this means its open loop and the controls will exhibit a 300ms latency, which btw would be even worse once I simulate the full weight payload.  (coming soon)

