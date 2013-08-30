/*
 * BooleanArray.h
 *
 *  Created on: Nov 16, 2012
 *      Author: Mitchell Wills
 */

#ifndef BOOLEANARRAY_H_
#define BOOLEANARRAY_H_

#include "ArrayData.h"
#include "ArrayEntryType.h"

/**
 *
 * @author Mitchell
 */
class BooleanArray : public ArrayData{

public:
	static const TypeId BOOLEAN_ARRAY_RAW_ID;
    static ArrayEntryType TYPE;


    BooleanArray();
    
    bool get(int index);
    void set(int index, bool value);
    void add(bool value);
};



#endif /* BOOLEANARRAY_H_ */
