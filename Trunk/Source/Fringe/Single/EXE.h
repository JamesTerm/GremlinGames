// EXE.h

#include "stdafx.h"
#include "Fringe.Single.h"
using namespace std;

#include <conio.h>

using namespace GG_Framework::UI;
using namespace GG_Framework::Logic;
using namespace GG_Framework::Logic::Network;
using namespace Fringe::Base;

#include "../../GG_Framework/Base/ExeMain.hpp"
namespace Fringe
{
	namespace Single
	{
		class EXE : public IExeMain
		{
		public:
			EXE()
			{
				GG_Framework::UI::OSG::LoadStatus::Instance.LoadStatusUpdate.Subscribe(ehl, *this, &EXE::LoadStatusUpdate);
			}
			virtual int Main(unsigned argc, const char* argv[]);
			void LoadStatusUpdate(float timeSoFar, float timeRemaining)
			{
				if (timeRemaining > 0.0f)
				{
					int pcntHalf = (int)(50.0f * timeSoFar / (timeSoFar+timeRemaining));
					char bar[51];
					for (int i = 0; i < 51; ++i)
						bar[i] = (i < pcntHalf) ? '-' : ' ';
					bar[50] = 0;
					printf("\rLoad Status: [%s] Rem: %i s.  ", bar, (int)timeRemaining);
				}
				else if (timeRemaining == 0.0f)
				{
					printf("\rLoad COMPLETE!                                                                \n");
				}
			}

		private:
			IEvent::HandlerList ehl;
		};
	}
}