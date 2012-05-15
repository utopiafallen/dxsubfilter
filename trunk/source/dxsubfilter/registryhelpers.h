// Helper functions for loading, writing and removing configuration settings from the
// registry. Configuration data is stored in HCKU/Software/dxsubfilter.
#pragma once

namespace DXSubFilter
{
	extern const wchar_t* DXSUBFILTER_SUBKEY_NAME;

	// Registry functions. 

	// Returns true if filter configuration data was loaded successfully from registry.
	// Returns false if filter configuration data was not found in registry.
	bool LoadFilterConfigurationFromRegistry();

	// Write default filter configuration data to registry.
	void WriteFilterConfigurationDataToRegistry();

	// Remove filter configuration data from registry.
	void RemoveFilterConfigurationDataFromRegistry();
};