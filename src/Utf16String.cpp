#include <cstdint>
#include <string>
#include <utf8.h>
#include "Utf16String.hpp"

namespace osmpbf2apidb
{
    Utf16String::Utf16String():
        m_internalUtf16String()
    {
        ;
    }

    Utf16String::~Utf16String()
    {
        ;
    }

    void Utf16String::setFromUtf8Bytes(
        const std::string&  utf8Bytes )
    {
        utf8::utf8to16(utf8Bytes.begin(), utf8Bytes.end(),
                       std::back_inserter(m_internalUtf16String));
    }

    std::string Utf16String::toUtf8() const
    {
        std::string utf8String;

        utf8::utf16to8(m_internalUtf16String.begin(), m_internalUtf16String.end(),
                       std::back_inserter(utf8String));

        return utf8String;
    }

    void Utf16String::clear()
    {
        m_internalUtf16String.clear();
    }
}
