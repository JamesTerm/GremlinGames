#include "stdafx.h"
/*
 * NumberArray.cpp
 *
 *  Created on: Nov 16, 2012
 *      Author: Mitchell Wills
 */

#include "NumberArray.h"
#include "DefaultEntryTypes.h"


const TypeId NumberArray::NUMBER_ARRAY_RAW_ID = 0x11;
ArrayEntryType NumberArray::TYPE(NUMBER_ARRAY_RAW_ID, DefaultEntryTypes::DOUBLE);

NumberArray::NumberArray() : ArrayData(TYPE) {
}

double NumberArray::get(int index){
	return _get(index).b;
}

void NumberArray::set(int index, double value){
	EntryValue eValue;
	eValue.f = value;
	_set(index, eValue);
}

void NumberArray::add(double value){
	EntryValue eValue;
	eValue.f = value;
	_add(eValue);
}
