// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Direct2D
#include "D2D1.h"
#include "D2D1Helper.h"

// DirectWrite
#include "DWrite.h"

// Microsoft Concurrency Runtime
#include <ppl.h>
#include <concurrent_queue.h>
#include <concurrent_vector.h>

// STL
#include <algorithm>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <tuple>

// Boost
#include <boost/circular_buffer.hpp>
#include <boost/multi_array.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>