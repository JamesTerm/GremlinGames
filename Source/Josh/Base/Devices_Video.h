#pragma once

#ifndef Devices_Base_Included
	#error Devices Base must be included instead of including this file directly.
#endif Devices_Base_Included

//	Video and audio devices simple provide the details of their formats
struct DeviceInterface : public Devices::DeviceInterface , public FormatInterface
{
};