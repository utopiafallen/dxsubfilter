// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DXSUBFILTER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DXSUBFILTER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef DXSUBFILTER_EXPORTS
#define DXSUBFILTER_API __declspec(dllexport)
#else
#define DXSUBFILTER_API __declspec(dllimport)
#endif

// This class is exported from the dxsubfilter.dll
class DXSUBFILTER_API Cdxsubfilter {
public:
	Cdxsubfilter(void);
	// TODO: add your methods here.
};

extern DXSUBFILTER_API int ndxsubfilter;

DXSUBFILTER_API int fndxsubfilter(void);
