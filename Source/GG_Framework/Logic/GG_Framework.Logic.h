// GG_Framework.Logic.h
#pragma once

#ifdef FRAMEWORK_LOGIC_EXPORTS
#define FRAMEWORK_LOGIC_API __declspec(dllexport)
#else
#define FRAMEWORK_LOGIC_API __declspec(dllimport)
#endif

#define DEBUG_PLAYER_LOADING // printf

//////////////////
// Other Libraries
#include "Scripting\GG_Framework.Logic.Scripting.h"
#include "Network\GG_Framework.Logic.Network.h"
#include "..\Base\GG_Framework.Base.h"
#include "..\UI\GG_Framework.UI.h"
#include "Physics\GG_Framework.Logic.Physics.h"

//////////////////
// My headers
#include "PacketEnums.h"
#include "ThreadedEntityActorLoader.h"
#include "RC_Controller.h"
#include "Entity3D.h"
#include "GameClient.h"
#include "UI_GameClient.h"
#include "TransmittedEntity.h"
#include "ServerManager.h"
#include "ThreadedClientGameLoader.h"

