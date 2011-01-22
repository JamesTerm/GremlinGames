#pragma once

// DLL export warning
#pragma warning ( disable : 4251 ) 

// Using "this" in the constructor init of member vars
#pragma warning ( disable : 4355 )

// Nonstandard extension used : '__restrict' keyword not supported in this product
#pragma warning ( disable : 4235 )

// warning C4799: No EMMS at end of function
#pragma warning ( disable : 4799 )

// No C library depreciation warnings
#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )

// If I cast a pointer to an int, I meant to do it
#pragma warning ( disable : 4311 )

// non dll-interface class used as base for dll-interface class
#pragma warning ( disable : 4275 )

// All the non-implicit type conversion problems
#pragma warning ( disable : 4244 )
