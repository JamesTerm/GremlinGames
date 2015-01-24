REM Be sure to set GG_UTILITIES to your path e.g. C:\GremlinGames\Source\Utilities
SetX GG_ADDN_INCLUDES "$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\include";"$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\include";"$(GG_UTILITIES)\RakNet\Include";"$(GG_UTILITIES)\FMOD\Win32\api\inc";"$(GG_UTILITIES)\ode-0.7\include";"$(GG_UTILITIES)\lua\lua5_1_2_Win32_dll7_lib\include";"$(GG_UTILITIES)/xmlParser" -m
SetX GG_ADDN_LIBS_DEBUG "$(GG_UTILITIES)\ode-0.7\lib\debugdll";"$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\lib\Debug";"$(GG_UTILITIES)\RakNet\Lib";"$(GG_UTILITIES)/FMOD/Win32/api/lib/";"$(GG_UTILITIES)\lua\lua5_1_2_Win32_dll7_lib";"$(GG_UTILITIES)/xmlParser/Debug" -m
SetX GG_ADDN_LIBS_RELEASE "$(GG_UTILITIES)\ode-0.7\lib\releasedll";"$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\lib\Release";"$(GG_UTILITIES)\RakNet\Lib";"$(GG_UTILITIES)/FMOD/Win32/api/lib/";"$(GG_UTILITIES)\lua\lua5_1_2_Win32_dll7_lib";"$(GG_UTILITIES)/xmlParser/Release" -m
pause
