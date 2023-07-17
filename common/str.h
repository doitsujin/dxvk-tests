#pragma once

#include <cstring>
#include <cstdint>
#include <string>
#include <sstream>
#include <vector>

template<size_t S> struct UnicodeChar { };
template<> struct UnicodeChar<1> { using type = uint8_t;  };
template<> struct UnicodeChar<2> { using type = uint16_t; };
template<> struct UnicodeChar<4> { using type = uint32_t; };

template<typename T>
using UnicodeCharType = typename UnicodeChar<sizeof(T)>::type;

inline uint32_t lzcnt(uint32_t n) {
  uint32_t mask = 0x80000000;

  for (uint32_t i = 0; i < 31; i++) {
    if (n & (mask >> i))
      return i;
  }

  return 32;
}


inline const uint8_t* decodeTypedChar(
  const uint8_t*  begin,
  const uint8_t*  end,
        uint32_t& ch) {
  uint32_t first = begin[0];

  if (first < 0x80) {
    // Basic ASCII character
    ch = uint32_t(first);
    return begin + 1;
  } else if (first < 0xC0) {
    // Character starts with a continuation byte,
    // just skip until we find the next valid prefix
    while ((begin < end) && (((*begin) & 0xC0) == 0x80))
      begin += 1;

    ch = uint32_t('?');
    return begin;
  } else {
    // The number of leading 1 bits in the first byte
    // determines the length of this character
    size_t length = lzcnt((~first) << 24);

    if (begin + length > end) {
      ch = uint32_t('?');
      return end;
    }

    if (first < 0xE0) {
      ch = ((uint32_t(begin[0]) & 0x1F) << 6)
          | ((uint32_t(begin[1]) & 0x3F));
    } else if (first < 0xF0) {
      ch = ((uint32_t(begin[0]) & 0x0F) << 12)
          | ((uint32_t(begin[1]) & 0x3F) << 6)
          | ((uint32_t(begin[2]) & 0x3F));
    } else if (first < 0xF8) {
      ch = ((uint32_t(begin[0]) & 0x07) << 18)
          | ((uint32_t(begin[1]) & 0x3F) << 12)
          | ((uint32_t(begin[2]) & 0x3F) << 6)
          | ((uint32_t(begin[3]) & 0x3F));
    } else {
      // Invalid prefix
      ch = uint32_t('?');
    }

    return begin + length;
  }
}

inline const uint16_t* decodeTypedChar(
  const uint16_t* begin,
  const uint16_t* end,
        uint32_t& ch) {
  uint32_t first = begin[0];

  if (first < 0xD800) {
    ch = first;
    return begin + 1;
  } else if (first < 0xDC00) {
    if (begin + 2 > end) {
      ch = uint32_t('?');
      return end;
    }

    ch = 0x10000
        + ((uint32_t(begin[0]) & 0x3FF) << 10)
        + ((uint32_t(begin[1]) & 0x3FF));
    return begin + 2;
  } else if (first < 0xE000) {
    // Stray low surrogate
    ch = uint32_t('?');
    return begin + 1;
  } else {
    ch = first;
    return begin + 1;
  }
}


inline const uint32_t* decodeTypedChar(
  const uint32_t* begin,
  const uint32_t* end,
        uint32_t& ch) {
  ch = begin[0];
  return begin + 1;
}


inline size_t encodeTypedChar(
        uint8_t*  begin,
        uint8_t*  end,
        uint32_t  ch) {
  if (ch < 0x80) {
    if (begin) {
      if (begin + 1 > end)
        return 0;

      begin[0] = uint8_t(ch);
    }

    return 1;
  } else if (ch < 0x800) {
    if (begin) {
      if (begin + 2 > end)
        return 0;

      begin[0] = uint8_t(0xC0 | (ch >> 6));
      begin[1] = uint8_t(0x80 | (ch & 0x3F));
    }

    return 2;
  } else if (ch < 0x10000) {
    if (begin) {
      if (begin + 3 > end)
        return 0;

      begin[0] = uint8_t(0xE0 | ((ch >> 12)));
      begin[1] = uint8_t(0x80 | ((ch >> 6) & 0x3F));
      begin[2] = uint8_t(0x80 | ((ch >> 0) & 0x3F));
    }

    return 3;
  } else if (ch < 0x200000) {
    if (begin) {
      if (begin + 4 < end)
        return 0;

      begin[0] = uint8_t(0xF0 | ((ch >> 18)));
      begin[1] = uint8_t(0x80 | ((ch >> 12) & 0x3F));
      begin[2] = uint8_t(0x80 | ((ch >> 6) & 0x3F));
      begin[3] = uint8_t(0x80 | ((ch >> 0) & 0x3F));
    }

    return 4;
  } else {
    // Invalid code point for UTF-8
    return 0;
  }
}


inline size_t encodeTypedChar(
        uint16_t* begin,
        uint16_t* end,
        uint32_t  ch) {
  if (ch < 0xD800) {
    if (begin) {
      if (begin + 1 > end)
        return 0;

      begin[0] = ch;
    }

    return 1;
  } else if (ch < 0xE000) {
    // Private use code points,
    // we can't encode these
    return 0;
  } else if (ch < 0x10000) {
    if (begin) {
      if (begin + 1 > end)
        return 0;

      begin[0] = ch;
    }

    return 1;
  } else if (ch < 0x110000) {
    if (begin) {
      if (begin + 2 > end)
        return 0;

      ch -= 0x10000;
      begin[0] = uint16_t(0xD800 + (ch >> 10));
      begin[1] = uint16_t(0xDC00 + (ch & 0x3FF));
    }

    return 2;
  } else {
    // Invalid code point
    return 0;
  }
}


inline size_t encodeTypedChar(
        uint32_t* begin,
        uint32_t* end,
        uint32_t  ch) {
  if (begin) {
    if (begin + 1 > end)
      return 0;

    begin[0] = ch;
  }

  return 1;
}


template<typename T>
const T* decodeChar(
  const T*        begin,
  const T*        end,
        uint32_t& ch) {
  using CharType = UnicodeCharType<T>;

  const CharType* result = decodeTypedChar(
    reinterpret_cast<const CharType*>(begin),
    reinterpret_cast<const CharType*>(end),
    ch);

  return reinterpret_cast<const T*>(result);
}


template<typename T>
size_t encodeChar(
        T*        begin,
        T*        end,
        uint32_t  ch) {
  using CharType = UnicodeCharType<T>;

  return encodeTypedChar(
    reinterpret_cast<CharType*>(begin),
    reinterpret_cast<CharType*>(end),
    ch);
}


template<typename S>
size_t length(const S* string) {
  size_t result = 0;

  while (string[result])
    result += 1;

  return result;
}


template<typename D, typename S>
size_t transcodeString(
        D*      dstBegin,
        size_t  dstLength,
  const S*      srcBegin,
        size_t  srcLength) {
  size_t totalLength = 0;

  auto dstEnd = dstBegin + dstLength;
  auto srcEnd = srcBegin + srcLength;

  while (srcBegin < srcEnd) {
    uint32_t ch;

    srcBegin = decodeChar<S>(srcBegin, srcEnd, ch);

    if (dstBegin)
      totalLength += encodeChar<D>(dstBegin + totalLength, dstEnd, ch);
    else
      totalLength += encodeChar<D>(nullptr, nullptr, ch);

    if (!ch)
      break;
  }

  return totalLength;
}


inline std::string fromws(const WCHAR* ws) {
  size_t srcLen = length(ws);
  size_t dstLen = transcodeString<char>(
    nullptr, 0, ws, srcLen);

  std::string result;
  result.resize(dstLen);

  transcodeString(result.data(),
    dstLen, ws, srcLen);

  return result;
}


inline std::wstring tows(const char* mbs) {
  size_t srcLen = length(mbs);
  size_t dstLen = transcodeString<wchar_t>(
    nullptr, 0, mbs, srcLen);

  std::wstring result;
  result.resize(dstLen);

  transcodeString(result.data(),
    dstLen, mbs, srcLen);

  return result;
}


inline void format1(std::stringstream&) { }

template<typename... Tx>
void format1(std::stringstream& str, const WCHAR *arg, const Tx&... args) {
  str << fromws(arg);
  format1(str, args...);
}

template<typename T, typename... Tx>
void format1(std::stringstream& str, const T& arg, const Tx&... args) {
  str << arg;
  format1(str, args...);
}

template<typename... Args>
std::string format(const Args&... args) {
  std::stringstream stream;
  format1(stream, args...);
  return stream.str();
}

inline void strlcpy(char* dst, const char* src, size_t count) {
  if (count > 0) {
    std::strncpy(dst, src, count - 1);
    dst[count - 1] = '\0';
  }
}
