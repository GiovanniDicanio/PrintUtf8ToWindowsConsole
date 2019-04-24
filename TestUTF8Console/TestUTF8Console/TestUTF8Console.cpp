////////////////////////////////////////////////////////////////////////////////
//
// Test printing UTF-8-encoded text to the Windows console
//
// by Giovanni Dicanio
//
////////////////////////////////////////////////////////////////////////////////


#include "UnicodeConv.hpp"  // My Unicode conversion helpers

#include <fcntl.h>
#include <io.h>
#include <stdint.h>
#include <iostream>
#include <string>


int main()
{
    // Change stdout to Unicode UTF-16.
    //
    // Note: _O_U8TEXT doesn't seem to work, e.g.:
    // https://blogs.msmvps.com/gdicanio/2017/08/22/printing-utf-8-text-to-the-windows-console/
    //
    _setmode(_fileno(stdout), _O_U16TEXT);


    // Japanese name for Japan, encoded in UTF-8
    uint8_t utf8[] = {
        0xE6, 0x97, 0xA5, // U+65E5
        0xE6, 0x9C, 0xAC, // U+672C
        0x00
    };
    std::string japan(reinterpret_cast<const char*>(utf8));

    // Print UTF-16-encoded text
    std::wcout << Utf16FromUtf8("Japan") << L"\n\n";
    std::wcout << Utf16FromUtf8(japan)    << L'\n';
}

////////////////////////////////////////////////////////////////////////////////

