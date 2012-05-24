#ifndef ISUBTITLERENDERER_H
#define ISUBTITLERENDERER_H
#pragma once

#include <strmif.h>
#include <string>
#include "SubtitleCoreEnumerations.h"

namespace SubtitleCore
{
	// Forward declarations
	struct SubtitlePicture;

	// ISubtitleRenderer
	//	Pure abstract interface that all subtitle renderers provided by SubtitleCore implement.
	//	Note that this interface (and SubtitleCore in general) does not obey the rules of COM
	//	since the library is never expected to be non-statically linked to (plus COM has a lot of
	//	complexity and overhead that didn't seem worthwhile to support). Instead, users can check
	//	what type of subtitle renderer they have through GetRendererSubtitleType and then 
	//	static_cast to the correct renderer. Users are also recommended to use the accompanying
	//	SubtitleRendererFactory to create the correct subtitle renderer given the subtitle type
	//	as there may be initialization data (margins, video image size, etc.) that need to be 
	//	given to a renderer. If the factory is used, only the factory needs to have its data 
	//	updated and the user can then simply request a new renderer.
	//
	//	Be aware that in general, ISubtitleRenderer implementations are not thread-safe. Each
	//	thread should own their own subtitle renderer.
	class __declspec(novtable) ISubtitleRenderer
	{
	public:

		// For text-based subtitle renderers, this allows the renderer to parse a complete script.
		// The entire script is expected to be encoded in UTF-16 and contained entirely in the
		// vector input with each entry in the vector corresponding to a new line in the original
		// text file.
		//
		// Returns true if the renderer succeeded in parsing the script. False otherwise.
		virtual bool ParseScript(const std::vector<std::wstring>& script) = 0;

		// For text-based subtitle renderers, this allows the renderer to parse a single line of
		// subtitle data. Input line is expected to be UTF-16 encoded and null-terminated.
		//
		// Returns true if the renderer succeeded in parsing the line. False otherwise.
		virtual bool ParseLine(const std::wstring& line) = 0;

		// Allows the renderer to parse whatever subtitle header/format data is defined by that
		// subtitle type. Overloaded to allow passing in of a byte-stream for non-text-based
		// subtitle types.
		//
		// Returns true if the renderer succeeded in parsing the header. False otherwise.
		virtual bool ParseFormatHeader(const std::wstring& header) = 0;
		virtual bool ParseFormatHeader(const unsigned char* header) = 0;

		// Allows the renderer to parse a byte-stream. What the semantics of this byte-stream 
		// are will depend on the subtitle type. It is expected that text-based subtitle 
		// renderers will do nothing if this function is called. Overloaded forms allow user to 
		// pass in complete byte-stream and offsets where parsing should start and/or end.
		//
		// Returns true if the renderer succeeded in parsing the data. False otherwise.
		virtual bool ParseData(const unsigned char* data) = 0;
		virtual bool ParseData(const unsigned char* data, ptrdiff_t offset) = 0;
		virtual bool ParseData(const unsigned char* data, ptrdiff_t startOffset, ptrdiff_t endOffset) = 0;

		// Signal the subtitle renderer that any data it has stored and/or processed should be 
		// cleared. Most likely needed when switching subtitle streams.
		virtual void Invalidate() = 0;

		// Get the subtitle type that this renderer is responsible for
		SubtitleType GetRendererSubtitleType() const { return m_SubtitleType; }

		// Get the number of subtitle pictures that will be returned given the current playback
		// time. This must be called prior to call GetSubtitlePictures as that function expects
		// a pre-allocated array large enough to contain all the pointers to the returned
		// subtitle pictures.
		//
		// A return value of 0 indicates no subtitles for the current playback time.
		virtual size_t GetSubtitlePictureCount(REFERENCE_TIME rtNow) = 0;

		// Stores GetSubtitlePictureCount number of SubtitlePicture pointers into the passed in
		// preallocated array of SubtitlePicture pointers.
		virtual void GetSubtitlePicture(REFERENCE_TIME rtNow, SubtitlePicture** ppOutSubPics) = 0;

	protected:
		SubtitleType m_SubtitleType;
	};
};

#endif