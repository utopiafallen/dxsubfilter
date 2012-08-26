// Registry functions. Configuration data is stored in HCKU/Software/dxsubfilter

#include "stdafx.h"
#include "registryhelpers.h"

namespace DXSubFilter
{
	// Hardcoded registry paths and value names
	const wchar_t* DXSUBFILTER_SUBKEY_NAME = L"Software\\dxsubfilter";
	const wchar_t* REGVALUE_SUBTITLE_BUFFER_SIZE_NAME = L"SubtitleBufferSize";
	const wchar_t* REGVALUE_SUBTITLE_ALIGNMENT_NAME = L"SubtitleLineAlignment";
	const wchar_t* REGVALUE_SUBTITLE_MARGIN_LEFT_NAME = L"SubtitleMarginLeft";
	const wchar_t* REGVALUE_SUBTITLE_MARGIN_RIGHT_NAME = L"SubtitleMarginRight";
	const wchar_t* REGVALUE_SUBTITLE_MARGIN_TOP_NAME = L"SubtitleMarginTop";
	const wchar_t* REGVALUE_SUBTITLE_MARGIN_BOTTOM_NAME = L"SubtitleMarginBottom";
	const wchar_t* REGVALUE_SUBTITLE_FONT_PRIMARY_FILL_COLOR_NAME = L"SubtitleFontPrimaryFillColor";
	const wchar_t* REGVALUE_SUBTITLE_FONT_SECONDARY_FILL_COLOR_NAME = L"SubtitleFontSecondaryFillColor";
	const wchar_t* REGVALUE_SUBTITLE_FONT_OUTLINE_COLOR_NAME = L"SubtitleFontOutlineColor";
	const wchar_t* REGVALUE_SUBTITLE_FONT_SHADOW_COLOR_NAME = L"SubtitleFontShadowColor";
	const wchar_t* REGVALUE_SUBTITLE_FONT_BORDER_WIDTH_NAME = L"SubtitleFontBorderWidth";
	const wchar_t* REGVALUE_SUBTITLE_FONT_SHADOW_DEPTH_NAME = L"SubtitleFontShadowDepth";
	const wchar_t* REGVALUE_SUBTITLE_FONT_SIZE_NAME = L"SubtitleFontSize";
	const wchar_t* REGVALUE_SUBTITLE_FONTNAME_NAME = L"SubtitleFontName";
	const wchar_t* REGVALUE_SUBTITLE_FONT_STYLE_NAME = L"SubtitleFontStyle";

	// Global subtitle core configuration settings
	SubtitleCore::SubtitleCoreConfigurationData g_SubtitleCoreConfigData;

	// Returns true if filter configuration data was loaded successfully from registry.
	// Returns false if filter configuration data was not found in registry.
	bool LoadFilterConfigurationFromRegistry()
	{
		HKEY hDXSubFilterKey;

		LONG lRegResult = RegOpenKeyEx(HKEY_CURRENT_USER, DXSUBFILTER_SUBKEY_NAME, 0, 
											KEY_READ, &hDXSubFilterKey);

		if (lRegResult == ERROR_SUCCESS)
		{
			// Load data.
			DWORD data;
			DWORD dataSize;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_BUFFER_SIZE_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_SubtitleBufferSize = data;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_ALIGNMENT_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_LineAlignment = static_cast<SubtitleCore::SubtitleCoreConfigurationData::DefaultLineAlignment>(data);

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_MARGIN_LEFT_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_LineMarginLeft = data;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_MARGIN_RIGHT_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_LineMarginRight = data;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_MARGIN_TOP_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_LineMarginTop = data;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_MARGIN_BOTTOM_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_LineMarginBottom = data;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_FONT_PRIMARY_FILL_COLOR_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_FontPrimaryFillColor = data;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_FONT_SECONDARY_FILL_COLOR_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_FontSecondaryFillColor = data;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_FONT_OUTLINE_COLOR_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_FontOutlineColor = data;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_FONT_SHADOW_COLOR_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_FontShadowColor = data;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_FONT_BORDER_WIDTH_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_fFontBorderWidth = static_cast<float>(data);
			g_SubtitleCoreConfigData.m_uFontBorderWidth = data;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_FONT_SHADOW_DEPTH_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_FontShadowDepth = data;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_FONT_SIZE_NAME, 
				RRF_RT_DWORD, NULL, &data, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			g_SubtitleCoreConfigData.m_FontSize = data;

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_FONTNAME_NAME, 
				RRF_RT_REG_SZ, NULL, NULL, &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}
			std::vector<wchar_t> fontName(dataSize/sizeof(wchar_t), L'A');

			lRegResult = RegGetValue(hDXSubFilterKey, NULL, REGVALUE_SUBTITLE_FONTNAME_NAME, 
				RRF_RT_REG_SZ, NULL, &fontName[0], &dataSize);
			if (lRegResult != ERROR_SUCCESS)
			{
				return false;
			}

			g_SubtitleCoreConfigData.m_FontName = &fontName[0];

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

		LONG lRegResult = RegCreateKeyEx(HKEY_CURRENT_USER, DXSUBFILTER_SUBKEY_NAME, 0,
			NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &hDXSubFilterKey, NULL);

		if (lRegResult == ERROR_SUCCESS)
		{
			// Write data.
			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_BUFFER_SIZE_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_SubtitleBufferSize), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_ALIGNMENT_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_LineAlignment), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_MARGIN_LEFT_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_LineMarginLeft), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_MARGIN_RIGHT_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_LineMarginRight), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_MARGIN_TOP_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_LineMarginTop), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_MARGIN_BOTTOM_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_LineMarginBottom), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_FONT_PRIMARY_FILL_COLOR_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_FontPrimaryFillColor), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_FONT_SECONDARY_FILL_COLOR_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_FontSecondaryFillColor), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_FONT_OUTLINE_COLOR_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_FontOutlineColor), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_FONT_SHADOW_COLOR_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_FontShadowColor), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_FONT_BORDER_WIDTH_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_uFontBorderWidth), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_FONT_SHADOW_DEPTH_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_FontShadowDepth), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_FONT_SIZE_NAME, NULL, 
				REG_DWORD, 
				reinterpret_cast<BYTE*>(&g_SubtitleCoreConfigData.m_FontSize), 
				sizeof(unsigned int));

			lRegResult = RegSetValueEx(hDXSubFilterKey, REGVALUE_SUBTITLE_FONTNAME_NAME, NULL, 
				REG_SZ, 
				reinterpret_cast<BYTE*>(const_cast<wchar_t*>(g_SubtitleCoreConfigData.m_FontName.c_str())), 
				(g_SubtitleCoreConfigData.m_FontName.length() + 1)*sizeof(wchar_t));
		}
	}

	// Remove filter configuration data from registry.
	void RemoveFilterConfigurationDataFromRegistry()
	{
		RegDeleteKeyEx(HKEY_CURRENT_USER, DXSUBFILTER_SUBKEY_NAME, KEY_WOW64_32KEY, 0);
	}
};