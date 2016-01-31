#ifndef _UTF16STRING_HPP
#define _UTF16STRING_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace OsmFileParser
{
class Utf16String
{
public:
    Utf16String();

    ~Utf16String();

    void setFromUtf8Bytes(
        const std::string&  utf8Bytes
    );

    std::string toUtf8() const;

    void clear();

private:
    std::vector<std::uint16_t>  m_internalUtf16String;
};
}

#endif // _UTF16STRING_HPP
