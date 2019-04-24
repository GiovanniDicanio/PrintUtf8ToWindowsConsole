////////////////////////////////////////////////////////////////////////////////
// 
//              *** Helper code to convert from UTF-8 to UTF-16 ***
// 
//                              by Giovanni Dicanio
// 
// 
// This code is based on my MSDN Magazine article published 
// on the 2016 September issue:
//
// "C++ - Unicode Encoding Conversions with STL Strings and Win32 APIs"
// https://msdn.microsoft.com/magazine/mt763237
//
//////////////////////////////////////////////////////////////////////////////// 

#pragma once

#include <limits>           // std::numeric_limits
#include <stdexcept>        // std::runtime_error, std::overflow_error
#include <string>           // std::string, std::wstring

#include <crtdbg.h>         // _ASSERTE()
#include <Windows.h>        // Win32 Platform SDK main header


//------------------------------------------------------------------------------
// Error when converting from UTF-8 to UTF-16
//------------------------------------------------------------------------------
class Utf16FromUtf8ConversionError
    : public std::runtime_error
{
public:

    // Initialize with error message raw C-string and last Win32 error code 
    Utf16FromUtf8ConversionError(const char* message, DWORD errorCode)
        : std::runtime_error(message)
        , m_errorCode(errorCode)
    {}

    // Initialize with error message string and last Win32 error code 
    Utf16FromUtf8ConversionError(const std::string& message, DWORD errorCode)
        : std::runtime_error(message)
        , m_errorCode(errorCode)
    {}

    // Retrieve error code associated to the failed conversion
    DWORD ErrorCode() const
    {
        return m_errorCode;
    }

private:
    // Error code from GetLastError()
    DWORD  m_errorCode;
};


//------------------------------------------------------------------------------
// Convert from UTF-8 to UTF-16.
// Throws Utf16FromUtf8ConversionError on error.
//------------------------------------------------------------------------------
inline std::wstring Utf16FromUtf8(const std::string& utf8)
{
    // Result of the conversion
    std::wstring utf16;

    // First, handle the special case of empty input string
    if (utf8.empty())
    {
        _ASSERTE(utf16.empty());
        return utf16;
    }

    // Safely fail if an invalid UTF-8 character sequence is encountered
    const DWORD kFlags = MB_ERR_INVALID_CHARS;

    // Safely cast the length of the source UTF-8 string (expressed in chars)
    // from size_t (returned by std::string::length()) to int 
    // for the MultiByteToWideChar API.
    // If the size_t value is too big to be stored into an int, 
    // throw an exception to prevent conversion errors like huge size_t values 
    // converted to *negative* integers.
    if (utf8.length() > static_cast<size_t>((std::numeric_limits<int>::max)()))
    {
        throw std::overflow_error(
            "Input string too long: size_t-length doesn't fit into int.");
    }
    const int utf8Length = static_cast<int>(utf8.length());

    // Get the size of the destination UTF-16 string
    const int utf16Length = ::MultiByteToWideChar(
        CP_UTF8,       // source string is in UTF-8
        kFlags,        // conversion flags
        utf8.data(),   // source UTF-8 string pointer
        utf8Length,    // length of the source UTF-8 string, in chars
        nullptr,       // unused - no conversion done in this step
        0              // request size of destination buffer, in wchar_ts
    );
    if (utf16Length == 0)
    {
        // Conversion error: capture error code and throw
        const DWORD error = ::GetLastError();
        throw Utf16FromUtf8ConversionError(
            error == ERROR_NO_UNICODE_TRANSLATION ?
            "Invalid UTF-8 sequence found in input string."
            :
            "Cannot get result string length when converting " \
            "from UTF-8 to UTF-16 (MultiByteToWideChar failed).",
            error
        );
    }

    // Make room in the destination string for the converted bits
    utf16.resize(utf16Length);

    // Do the actual conversion from UTF-8 to UTF-16
    int result = ::MultiByteToWideChar(
        CP_UTF8,       // source string is in UTF-8
        kFlags,        // conversion flags
        utf8.data(),   // source UTF-8 string pointer
        utf8Length,    // length of source UTF-8 string, in chars
        &utf16[0],     // pointer to destination buffer
        utf16Length    // size of destination buffer, in wchar_ts           
    );
    if (result == 0)
    {
        // Conversion error: capture error code and throw
        const DWORD error = ::GetLastError();
        throw Utf16FromUtf8ConversionError(
            error == ERROR_NO_UNICODE_TRANSLATION ?
            "Invalid UTF-8 sequence found in input string."
            :
            "Cannot convert from UTF-8 to UTF-16 "\
            "(MultiByteToWideChar failed).",
            error
        );
    }

    return utf16;
}

////////////////////////////////////////////////////////////////////////////////
