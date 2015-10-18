#ifndef SCULEXICALCAST_H
#define SCULEXICALCAST_H
#pragma once

#include <xstring>

// SCU Lexical Cast because including all of boost for it is excessive.
namespace SCU
{
    template <typename Target, typename Source>
    inline Target lexical_cast(const Source& src);

    template <>
    inline size_t lexical_cast<size_t, std::wstring>(const std::wstring& src)
    {
        size_t result = 0;
        size_t scale = 1;
        size_t len = src.length();
        for (size_t i = len - 1; i > 0; i--, scale *= 10)
        {
            size_t val = src[i] - L'0';
            result += val * scale;
        }

        result += (src[0] - L'0') * scale;

        return result;
    }
}

#endif