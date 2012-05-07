// dxsubfilter.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "dxsubfilter.h"


// This is an example of an exported variable
DXSUBFILTER_API int ndxsubfilter=0;

// This is an example of an exported function.
DXSUBFILTER_API int fndxsubfilter(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see dxsubfilter.h for the class definition
Cdxsubfilter::Cdxsubfilter()
{
	return;
}
