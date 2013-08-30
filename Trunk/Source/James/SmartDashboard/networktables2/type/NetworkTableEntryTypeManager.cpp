#include "stdafx.h"
/*
 * NetworkTableEntryTypeManager.cpp
 *
 *  Created on: Sep 16, 2012
 *      Author: Mitchell Wills
 */

#include "NetworkTableEntryTypeManager.h"
#include "DefaultEntryTypes.h"


NetworkTableEntryType* NetworkTableEntryTypeManager::GetType(TypeId id){
	return entryTypes[id];
}

void NetworkTableEntryTypeManager::RegisterType(NetworkTableEntryType& type){
	entryTypes[type.id] = &type;
}

NetworkTableEntryTypeManager::NetworkTableEntryTypeManager(){
	for(int i = 0; i<MAX_NUM_TABLE_ENTRY_TYPES; ++i)
		entryTypes[i] = NULL;
	
	DefaultEntryTypes::registerTypes(this);
}
