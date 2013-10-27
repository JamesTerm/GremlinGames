/*
 * StringArray.h
 *
 *  Created on: Nov 16, 2012
 *      Author: Mitchell Wills
 */

#ifndef STRINGARRAY_H_
#define STRINGARRAY_H_


#include "ArrayData.h"
#include "ArrayEntryType.h"

/**
 *
 * @author Mitchell
 */
class StringArray : public ArrayData{

public:
	static const TypeId STRING_ARRAY_RAW_ID;
    static ArrayEntryType TYPE;


    StringArray();
    
    std::string get(int index);
    void set(int index, std::string value);
    void add(std::string value);
};

#endif /* STRINGARRAY_H_ */