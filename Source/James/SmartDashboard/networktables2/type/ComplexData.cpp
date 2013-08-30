#include "stdafx.h"
/*
 * ComplexData.cpp
 *
 *  Created on: Sep 24, 2012
 *      Author: Mitchell Wills
 */

#include "ComplexData.h"


ComplexData::ComplexData(ComplexEntryType& _type) : type(_type){}
	
ComplexEntryType& ComplexData::GetType() {
	return type;
}
