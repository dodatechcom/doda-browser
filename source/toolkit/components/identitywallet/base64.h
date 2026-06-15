#ifndef DODA_BASE64_H
#define DODA_BASE64_H

#include "nsString.h"
#include "mozilla/Span.h"
#include <cstdint>
#include <cstring>

static const char kBase64Alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline nsCString Base64Encode(mozilla::Span<const uint8_t> aData) {
  nsCString result;
  uint32_t inputLen = aData.Length();
  uint32_t outputLen = ((inputLen + 2) / 3) * 4;
  result.SetLength(outputLen);

  const uint8_t* input = aData.Elements();
  char* output = result.BeginWriting();

  for (uint32_t i = 0; i < inputLen; i += 3) {
    uint32_t remaining = inputLen - i;
    uint32_t byte0 = input[i];
    uint32_t byte1 = (remaining > 1) ? input[i + 1] : 0;
    uint32_t byte2 = (remaining > 2) ? input[i + 2] : 0;
    uint32_t triple = (byte0 << 16) | (byte1 << 8) | byte2;

    output[0] = kBase64Alphabet[(triple >> 18) & 0x3F];
    output[1] = kBase64Alphabet[(triple >> 12) & 0x3F];
    output[2] = (remaining > 1) ? kBase64Alphabet[(triple >> 6) & 0x3F] : '=';
    output[3] = (remaining > 2) ? kBase64Alphabet[triple & 0x3F] : '=';
    output += 4;
  }
  return result;
}

inline nsTArray<uint8_t> Base64Decode(const nsACString& aData) {
  nsTArray<uint8_t> result;
  uint32_t inputLen = aData.Length();
  if (inputLen % 4 != 0) return result;

  uint32_t outputLen = inputLen / 4 * 3;
  if (inputLen > 0 && aData[inputLen - 1] == '=') outputLen--;
  if (inputLen > 1 && aData[inputLen - 2] == '=') outputLen--;

  result.SetLength(outputLen);
  uint8_t* output = result.Elements();

  auto decodeChar = [](char c) -> uint32_t {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return 0;
  };

  const char* input = aData.BeginReading();
  uint32_t outIdx = 0;
  for (uint32_t i = 0; i < inputLen; i += 4) {
    uint32_t triple =
        (decodeChar(input[i]) << 18) | (decodeChar(input[i + 1]) << 12) |
        ((input[i + 2] != '=') ? (decodeChar(input[i + 2]) << 6) : 0) |
        ((input[i + 3] != '=') ? decodeChar(input[i + 3]) : 0);

    output[outIdx++] = (triple >> 16) & 0xFF;
    if (input[i + 2] != '=') output[outIdx++] = (triple >> 8) & 0xFF;
    if (input[i + 3] != '=') output[outIdx++] = triple & 0xFF;
  }
  return result;
}

#endif
