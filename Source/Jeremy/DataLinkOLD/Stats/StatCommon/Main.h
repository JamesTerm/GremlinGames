#pragma once

#ifdef STATCOMMON_EXPORTS
#define STATCOMMON_API __declspec(dllexport)
#else
#define STATCOMMON_API __declspec(dllimport)
#endif

#include "auto_synchronization.h"
#include "file_extension_list.h"
#include "folder_observer.h"
#include "folder_traverser.h"
#include "stat_base.h"
#include "stat_container.h"
#include "synchronization.h"
