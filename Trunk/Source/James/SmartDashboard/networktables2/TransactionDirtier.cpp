#include "stdafx.h"
/*
 * TransactionDirtier.cpp
 *
 *  Created on: Sep 24, 2012
 *      Author: Mitchell Wills
 */

#include "TransactionDirtier.h"



	TransactionDirtier::TransactionDirtier(OutgoingEntryReceiver& _continuingReceiver) : continuingReceiver(_continuingReceiver){
	}



	void TransactionDirtier::offerOutgoingAssignment(NetworkTableEntry* entry) {
		if(entry->IsDirty())
			return;
		entry->MakeDirty();
		continuingReceiver.offerOutgoingAssignment(entry);
	}




	void TransactionDirtier::offerOutgoingUpdate(NetworkTableEntry* entry) {
		if(entry->IsDirty())
			return;
		entry->MakeDirty();
		continuingReceiver.offerOutgoingUpdate(entry);
	}

