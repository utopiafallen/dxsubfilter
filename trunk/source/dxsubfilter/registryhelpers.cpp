// Registry functions. Configuration data is stored in HCKU/Software/dxsubfilter

#include "stdafx.h"
#include "registryhelpers.h"

namespace DXSubFilter
{
	const wchar_t* DXSUBFILTER_SUBKEY_NAME = L"Software\\dxsubfilter";

	// Returns true if filter configuration data was loaded successfully from registry.
	// Returns false if filter configuration data was not found in registry.
	bool LoadFilterConfigurationFromRegistry()
	{
		HKEY hDXSubFilterKey;

		LONG lRegOpenResult = RegOpenKeyEx(HKEY_CURRENT_USER, DXSUBFILTER_SUBKEY_NAME, 0, 
											KEY_READ, &hDXSubFilterKey);

		if (lRegOpenResult == ERROR_SUCCESS)
		{
			// Load data. Will probably store this in a struct whose pointer is passed into the 
			// initialization function of CDXSubFilter
			UNREFERENCED_PARAMETER(hDXSubFilterKey);
			return true;
		}
		else
		{
			return false;
		}
	}

	// Write default filter configuration data to registry.
	void WriteFilterConfigurationDataToRegistry()
	{
		HKEY hDXSubFilterKey;

		LONG lRegCreateResult = RegCreateKeyEx(HKEY_CURRENT_USER, DXSUBFILTER_SUBKEY_NAME, 0,
			NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &hDXSubFilterKey, NULL);

		if (lRegCreateResult == ERROR_SUCCESS)
		{
			// Write data.
			UNREFERENCED_PARAMETER(hDXSubFilterKey);
		}
	}

	// Remove filter configuration data from registry.
	void RemoveFilterConfigurationDataFromRegistry()
	{
		RegDeleteKeyEx(HKEY_CURRENT_USER, DXSUBFILTER_SUBKEY_NAME, KEY_WOW64_32KEY, 0);
	}
};