#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// DLL export warning
#pragma warning ( disable : 4251 ) 
#pragma warning ( disable : 4273 ) 
#pragma warning ( disable : 4275 ) 

// Using "this" in the constructor init of member vars
#pragma warning ( disable : 4355 )

// Nonstandard extension used : '__restrict' keyword not supported in this product
#pragma warning ( disable : 4235 )

// warning C4799: No EMMS at end of function
#pragma warning ( disable : 4799 )

// No C library depreciation warnings
#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )

// nonstandard extension used: enum 'FrameWork::Communication3::video::message::e_field_type' used in qualified name
#pragma warning ( disable : 4482 )

// Function compiled as native : Aligned data types not supported in managed code
#pragma warning ( disable : 4793 )

// nonstandard extension used : zero-sized array in struct/union
#pragma warning ( disable : 4200 )

// attributes not present on previous declaration.
// See "http://connect.microsoft.com/VisualStudio/feedback/details/381422/warning-of-attributes-not-present-on-previous-declaration-on-ceil-using-both-math-h-and-intrin-h"
#pragma warning ( disable : 4985 )

// initialization of 'strm' is skipped by 'goto error'
#pragma warning ( disable : 4533 )

// warning C4200: nonstandard extension used : zero-sized array in struct/union
#pragma warning ( disable : 4200 )


//TODO I really should fix these
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4312 )
#pragma warning ( disable : 4313 )
#pragma warning ( disable : 4267 )



#include "TimeCodeConverters.h"
#include "FileOperations.h"
#include "ListTemplate.h"