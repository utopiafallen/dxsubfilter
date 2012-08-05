// Helper functions for loading, writing and removing configuration settings from the
// registry. Configuration data is stored in HCKU/Software/dxsubfilter.
#ifndef DXSUBFILTERREGHELPER_H
#define DXSUBFILTERREGHELPER_H
#pragma once

#include "SubtitleCoreEnumerations.h"

namespace DXSubFilter
{
	extern const wchar_t* DXSUBFILTER_SUBKEY_NAME;

	extern SubtitleCore::SubtitleCoreConfigurationData g_SubtitleCoreConfigData;

	// Registry functions. 

	// Returns true if filter configuration data was loaded successfully from registry.
	// Returns false if filter configuration data was not found in registry.
	bool LoadFilterConfigurationFromRegistry();

	// Write default filter configuration data to registry.
	void WriteFilterConfigurationDataToRegistry();

	// Remove filter configuration data from registry.
	void RemoveFilterConfigurationDataFromRegistry();
};

#endif