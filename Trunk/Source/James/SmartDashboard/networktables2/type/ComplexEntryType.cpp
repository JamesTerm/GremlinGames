#include "stdafx.h"
/*
 * ComplexEntryType.cpp
 *
 *  Created on: Sep 24, 2012
 *      Author: Mitchell Wills
 */

#include "ComplexEntryType.h"


ComplexEntryType::ComplexEntryType(TypeId id, const char* name) : NetworkTableEntryType(id, name){}

bool ComplexEntryType::isComplex(){
	return true;
}
