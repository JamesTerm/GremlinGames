#pragma once

//********************************************************************************************************************************************

namespace Devices { namespace Audio { namespace PeakLevels {

struct IPeakLevelListener
{
	virtual void PealLevelsChanged(const float *pLevels, const size_t cChannels) = 0;
};

} } }

//********************************************************************************************************************************************