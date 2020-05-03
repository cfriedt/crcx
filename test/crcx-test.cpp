/*
 * MIT License
 *
 * Copyright (c) 2020 Christopher Friedt
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cstdlib>

#include <gtest/gtest.h>

using namespace std;

#include "crcx/crcx.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / (sizeof((x)[0])))
#endif

static void eraseAll(string &haystack, const string &needle) {
  string &s = haystack;

  vector<uintmax_t> r;

  for (auto pos = s.find(needle); string::npos != pos; pos = s.find(needle)) {
    s.erase(pos, needle.size());
  }
}

static vector<uintmax_t> to_vector(string s) {
  vector<uintmax_t> r;

  eraseAll(s, "0x");

  for (istringstream iss(s); iss;) {
    string s2;
    iss >> s2;
    if (s2.empty()) {
      break;
    }
    uintmax_t x = ::strtoull(s2.c_str(), nullptr, 16);
    r.push_back(x);
  }

  return r;
}

static vector<uintmax_t> to_vector(uintmax_t *x, size_t len) {
  return vector<uintmax_t>(x, x + len);
}

//
// Sanity checks, input validation, etc
//

TEST(Sanity, ctx_null) { ASSERT_FALSE(::crcx_valid(nullptr)); }

TEST(Sanity, poly_zero) {
  ::crcx_ctx ctx = {
      .n = 8,
      .init = 0,
      .fini = 0,
      .poly = 0, // a polynomial of 0 is invalid
      .mask = (1 << 8) - 1,
      .msb = 1 << (8 - 1),
      .reflect_input = false,
      .reflect_output = false,
      .table = {},
      .lfsr = 0,
  };
  ASSERT_FALSE(::crcx_valid(&ctx));
}

TEST(Sanity, poly_highest_bit_greater_than_n) {
  ::crcx_ctx ctx = {
      .n = 8,
      .init = 0,
      .fini = 0,
      .poly = 0x8000,
      .mask = (1 << 8) - 1,
      .msb = 1 << (8 - 1),
      .reflect_input = false,
      .reflect_output = false,
      .table = {},
      .lfsr = 0,
  };
  ASSERT_FALSE(::crcx_valid(&ctx));
}

TEST(Sanity, n_zero) {
  ::crcx_ctx ctx = {
      .n = 0, // number of bits must be a non-zero and positive multiple of 8
      .init = 0,
      .fini = 0,
      .poly = 0x7,
      .mask = (1 << 8) - 1,
      .msb = 1 << (8 - 1),
      .reflect_input = false,
      .reflect_output = false,
      .table = {},
      .lfsr = 0,
  };
  ASSERT_FALSE(::crcx_valid(&ctx));
}

TEST(Sanity, n_not_a_multiple_of_8) {
  ::crcx_ctx ctx = {
      .n = 7, // number of bits must be a non-zero and positive multiple of 8
      .init = 0,
      .fini = 0,
      .poly = 0x7,
      .mask = (1 << 8) - 1,
      .msb = 1 << (8 - 1),
      .reflect_input = false,
      .reflect_output = false,
      .table = {},
      .lfsr = 0,
  };
  ASSERT_FALSE(::crcx_valid(&ctx));
}

TEST(Sanity, n_greater_than_sizeof_uintmax) {
  ::crcx_ctx ctx = {
      .n = 8 * sizeof(uintmax_t) +
           1, // number of bits must be a non-zero and positive multiple of 8
      .init = 0,
      .fini = 0,
      .poly = 0x7,
      .mask = (1 << 8) - 1,
      .msb = 1 << (8 - 1),
      .reflect_input = false,
      .reflect_output = false,
      .table = {},
      .lfsr = 0,
  };
  ASSERT_FALSE(::crcx_valid(&ctx));
}

TEST(Sanity, msb_invalid) {
  ::crcx_ctx ctx = {
      .n = 8,
      .init = 0,
      .fini = 0,
      .poly = 0x7,
      .mask = (1 << 8) - 1,
      .msb = 0x1, // msb must be 1 << (n - 1)
      .reflect_input = false,
      .reflect_output = false,
      .table = {},
      .lfsr = 0,
  };
  ASSERT_FALSE(::crcx_valid(&ctx));
}

TEST(Sanity, mask_invalid) {
  ::crcx_ctx ctx = {
      .n = 8,
      .init = 0,
      .fini = 0,
      .poly = 0x7,
      .mask = (1 << 8) - 2, // mask must be (1 << n) - 1
      .msb = 1 << (8 - 1),
      .reflect_input = false,
      .reflect_output = false,
      .table = {},
      .lfsr = 0,
  };
  ASSERT_FALSE(::crcx_valid(&ctx));
}

TEST(Sanity, mask_invalid64) {
  ::crcx_ctx ctx = {
      .n = 64,
      .init = 0,
      .fini = 0,
      .poly = 0x7,
      .mask = 0xfffffffffffffffeULL, // mask must be (1 << n) - 1
      .msb = 0x8000000000000000ULL,
      .reflect_input = false,
      .reflect_output = false,
      .table = {},
      .lfsr = 0,
  };
  ASSERT_FALSE(::crcx_valid(&ctx));
}

TEST(Sanity, invalid_ctx_tries_to_generate_table) {
  ASSERT_FALSE(::crcx_generate_table(nullptr));
}

TEST(Sanity, invalid_param_to_crcx_init) {
  ::crcx_ctx ctx = {};
  ASSERT_FALSE(::crcx_init(&ctx, 1, 0, 0, 0x7, false, false));
}

// clang-format off
TEST(Sanity, invalid_ctx_to_crcx_fini) {
	ASSERT_EQ(::crcx_fini(nullptr), -1);
}
// clang-format on

// clang-format off
TEST(Sanity, invalid_ctx_to_crcx) {
	ASSERT_FALSE(::crcx(nullptr, nullptr, 0));
}
// clang-format on

// This test shows that the CRC works for a single byte.
// It also tests to make sure that the input parameters are set accordingly.
// Also, it checks that the LUT is generated correctly.
TEST(LibCRCx, Wikipedia_example1) {
  // https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#Example
  // compute the 8-bit CRC of the ascii character 'W' (0b01010111, 0x57, 87)
  // CRC-8-ATM (HEC) polynomial x^8 + x^2 + x + 1 => '100000111'

  ::crcx_ctx ctx = {};

  ASSERT_TRUE(::crcx_init(&ctx, 8, 0b100000111, 0, 0, false, false));

  ASSERT_EQ(ctx.n, 8);
  ASSERT_EQ(ctx.msb, 0x80);
  ASSERT_EQ(ctx.mask, 0xff);
  ASSERT_EQ(ctx.init, 0);
  ASSERT_EQ(ctx.fini, 0);
  ASSERT_EQ(ctx.poly, 0b100000111);
  ASSERT_EQ(ctx.lfsr, 0);

  // This site has a table generator and gives the same output as the wikipedia
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // clang-format off
  auto expected_v = to_vector(
    "0x00 0x07 0x0E 0x09 0x1C 0x1B 0x12 0x15 0x38 0x3F 0x36 0x31 0x24 0x23 0x2A 0x2D "
    "0x70 0x77 0x7E 0x79 0x6C 0x6B 0x62 0x65 0x48 0x4F 0x46 0x41 0x54 0x53 0x5A 0x5D "
    "0xE0 0xE7 0xEE 0xE9 0xFC 0xFB 0xF2 0xF5 0xD8 0xDF 0xD6 0xD1 0xC4 0xC3 0xCA 0xCD "
    "0x90 0x97 0x9E 0x99 0x8C 0x8B 0x82 0x85 0xA8 0xAF 0xA6 0xA1 0xB4 0xB3 0xBA 0xBD "
    "0xC7 0xC0 0xC9 0xCE 0xDB 0xDC 0xD5 0xD2 0xFF 0xF8 0xF1 0xF6 0xE3 0xE4 0xED 0xEA "
    "0xB7 0xB0 0xB9 0xBE 0xAB 0xAC 0xA5 0xA2 0x8F 0x88 0x81 0x86 0x93 0x94 0x9D 0x9A "
    "0x27 0x20 0x29 0x2E 0x3B 0x3C 0x35 0x32 0x1F 0x18 0x11 0x16 0x03 0x04 0x0D 0x0A "
    "0x57 0x50 0x59 0x5E 0x4B 0x4C 0x45 0x42 0x6F 0x68 0x61 0x66 0x73 0x74 0x7D 0x7A "
    "0x89 0x8E 0x87 0x80 0x95 0x92 0x9B 0x9C 0xB1 0xB6 0xBF 0xB8 0xAD 0xAA 0xA3 0xA4 "
    "0xF9 0xFE 0xF7 0xF0 0xE5 0xE2 0xEB 0xEC 0xC1 0xC6 0xCF 0xC8 0xDD 0xDA 0xD3 0xD4 "
    "0x69 0x6E 0x67 0x60 0x75 0x72 0x7B 0x7C 0x51 0x56 0x5F 0x58 0x4D 0x4A 0x43 0x44 "
    "0x19 0x1E 0x17 0x10 0x05 0x02 0x0B 0x0C 0x21 0x26 0x2F 0x28 0x3D 0x3A 0x33 0x34 "
    "0x4E 0x49 0x40 0x47 0x52 0x55 0x5C 0x5B 0x76 0x71 0x78 0x7F 0x6A 0x6D 0x64 0x63 "
    "0x3E 0x39 0x30 0x37 0x22 0x25 0x2C 0x2B 0x06 0x01 0x08 0x0F 0x1A 0x1D 0x14 0x13 "
    "0xAE 0xA9 0xA0 0xA7 0xB2 0xB5 0xBC 0xBB 0x96 0x91 0x98 0x9F 0x8A 0x8D 0x84 0x83 "
    "0xDE 0xD9 0xD0 0xD7 0xC2 0xC5 0xCC 0xCB 0xE6 0xE1 0xE8 0xEF 0xFA 0xFD 0xF4 0xF3 "
  );
  // clang-format on
  auto actual_v = to_vector((uintmax_t *)ctx.table, ARRAY_SIZE(ctx.table));

  ASSERT_EQ(actual_v, expected_v) << "The generated table is incorrect";

  ASSERT_TRUE(::crcx(&ctx, reinterpret_cast<const uint8_t *>("W"), 1));

  uintmax_t expected_uintmax = 0xa2;
  uintmax_t actual_uintmax = ::crcx_fini(&ctx);

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "The calculated CRC is incorrect";
}

// this test shows that multi-byte input CRC works
TEST(LibCRCx, Sunshine_CRC8) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC8
  // input not reflected
  // result not reflected
  // polynomial 0x07 => x^8 + x^2 + x + 1
  // initial value: 0
  // final xor value: 0
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  ::crcx_ctx ctx = {};
  ASSERT_TRUE(::crcx_init(&ctx, 8, 0x07, 0, 0, false, false));

  ASSERT_TRUE(::crcx(&ctx, &data.front(), data.size()));

  uintmax_t expected_uintmax = 0xf4;
  uintmax_t actual_uintmax = ::crcx_fini(&ctx);

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "The calculated CRC is incorrect";
}

// this test shows that the final xor value is applied
TEST(LibCRCx, Sunshine_CRC8_ITU) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC8_ITU
  // input not reflected
  // result not reflected
  // polynomial 0x07 => x^8 + x^2 + x + 1
  // initial value: 0
  // final xor value: 0x55
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  ::crcx_ctx ctx = {};
  ASSERT_TRUE(::crcx_init(&ctx, 8, 0x07, 0, 0x55, false, false));

  ASSERT_TRUE(::crcx(&ctx, &data.front(), data.size()));

  uintmax_t expected_uintmax = 0xa1;
  uintmax_t actual_uintmax = ::crcx_fini(&ctx);

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "The calculated CRC is incorrect";
}

// this test shows that input and output reflection works
TEST(LibCRCx, Sunshine_CRC8_DARC) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC8_DARC
  // input reflected
  // result reflected
  // polynomial 0x39 => x^8 + 1x^5 + 1x^4 + 1x^3 1x
  // initial value: 0
  // final xor value: 0
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  ::crcx_ctx ctx = {};
  ASSERT_TRUE(::crcx_init(&ctx, 8, 0x39, 0, 0, true, true));

  ASSERT_TRUE(::crcx(&ctx, &data.front(), data.size()));

  uintmax_t expected_uintmax = 0x15;
  uintmax_t actual_uintmax = ::crcx_fini(&ctx);

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "The calculated CRC is incorrect";
}

// this test shows that CRC16 works for one byte messages
TEST(LibCRCx, Sunshine_CRC16_CCIT_ZERO_one_byte) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC16_CCIT_ZERO
  // input not reflected
  // result not reflected
  // polynomial 0x1021
  // initial value: 0
  // final xor value: 0
  // CRC Input Data: 'W'

  const vector<uint8_t> data{uint8_t('W')};

  ::crcx_ctx ctx = {};
  ASSERT_TRUE(::crcx_init(&ctx, 16, 0x1021, 0, 0, false, false));

  ASSERT_EQ(ctx.n, 16);
  ASSERT_EQ(ctx.msb, 0x8000);
  ASSERT_EQ(ctx.mask, 0xffff);
  ASSERT_EQ(ctx.init, 0);
  ASSERT_EQ(ctx.fini, 0);
  ASSERT_EQ(ctx.poly, 0x1021);
  ASSERT_EQ(ctx.lfsr, 0);

  // clang-format off
    auto expected_v = to_vector(
        "0x0000 0x1021 0x2042 0x3063 0x4084 0x50A5 0x60C6 0x70E7 0x8108 0x9129 0xA14A 0xB16B 0xC18C 0xD1AD 0xE1CE 0xF1EF "
        "0x1231 0x0210 0x3273 0x2252 0x52B5 0x4294 0x72F7 0x62D6 0x9339 0x8318 0xB37B 0xA35A 0xD3BD 0xC39C 0xF3FF 0xE3DE "
        "0x2462 0x3443 0x0420 0x1401 0x64E6 0x74C7 0x44A4 0x5485 0xA56A 0xB54B 0x8528 0x9509 0xE5EE 0xF5CF 0xC5AC 0xD58D "
        "0x3653 0x2672 0x1611 0x0630 0x76D7 0x66F6 0x5695 0x46B4 0xB75B 0xA77A 0x9719 0x8738 0xF7DF 0xE7FE 0xD79D 0xC7BC "
        "0x48C4 0x58E5 0x6886 0x78A7 0x0840 0x1861 0x2802 0x3823 0xC9CC 0xD9ED 0xE98E 0xF9AF 0x8948 0x9969 0xA90A 0xB92B "
        "0x5AF5 0x4AD4 0x7AB7 0x6A96 0x1A71 0x0A50 0x3A33 0x2A12 0xDBFD 0xCBDC 0xFBBF 0xEB9E 0x9B79 0x8B58 0xBB3B 0xAB1A "
        "0x6CA6 0x7C87 0x4CE4 0x5CC5 0x2C22 0x3C03 0x0C60 0x1C41 0xEDAE 0xFD8F 0xCDEC 0xDDCD 0xAD2A 0xBD0B 0x8D68 0x9D49 "
        "0x7E97 0x6EB6 0x5ED5 0x4EF4 0x3E13 0x2E32 0x1E51 0x0E70 0xFF9F 0xEFBE 0xDFDD 0xCFFC 0xBF1B 0xAF3A 0x9F59 0x8F78 "
        "0x9188 0x81A9 0xB1CA 0xA1EB 0xD10C 0xC12D 0xF14E 0xE16F 0x1080 0x00A1 0x30C2 0x20E3 0x5004 0x4025 0x7046 0x6067 "
        "0x83B9 0x9398 0xA3FB 0xB3DA 0xC33D 0xD31C 0xE37F 0xF35E 0x02B1 0x1290 0x22F3 0x32D2 0x4235 0x5214 0x6277 0x7256 "
        "0xB5EA 0xA5CB 0x95A8 0x8589 0xF56E 0xE54F 0xD52C 0xC50D 0x34E2 0x24C3 0x14A0 0x0481 0x7466 0x6447 0x5424 0x4405 "
        "0xA7DB 0xB7FA 0x8799 0x97B8 0xE75F 0xF77E 0xC71D 0xD73C 0x26D3 0x36F2 0x0691 0x16B0 0x6657 0x7676 0x4615 0x5634 "
        "0xD94C 0xC96D 0xF90E 0xE92F 0x99C8 0x89E9 0xB98A 0xA9AB 0x5844 0x4865 0x7806 0x6827 0x18C0 0x08E1 0x3882 0x28A3 "
        "0xCB7D 0xDB5C 0xEB3F 0xFB1E 0x8BF9 0x9BD8 0xABBB 0xBB9A 0x4A75 0x5A54 0x6A37 0x7A16 0x0AF1 0x1AD0 0x2AB3 0x3A92 "
        "0xFD2E 0xED0F 0xDD6C 0xCD4D 0xBDAA 0xAD8B 0x9DE8 0x8DC9 0x7C26 0x6C07 0x5C64 0x4C45 0x3CA2 0x2C83 0x1CE0 0x0CC1 "
        "0xEF1F 0xFF3E 0xCF5D 0xDF7C 0xAF9B 0xBFBA 0x8FD9 0x9FF8 0x6E17 0x7E36 0x4E55 0x5E74 0x2E93 0x3EB2 0x0ED1 0x1EF0 "
    );
  // clang-format on
  auto actual_v = to_vector((uintmax_t *)ctx.table, ARRAY_SIZE(ctx.table));

  ASSERT_EQ(actual_v, expected_v) << "The generated table is incorrect";

  ASSERT_TRUE(::crcx(&ctx, &data.front(), data.size()));

  uintmax_t expected_uintmax = 0x2A12;
  uintmax_t actual_uintmax = ::crcx_fini(&ctx);

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "The calculated CRC is incorrect";
}

// this test shows that CRC16 works (incrementally)
TEST(LibCRCx, Sunshine_CRC16_CCIT_ZERO_incremental) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC16_CCIT_ZERO
  // input not reflected
  // result not reflected
  // polynomial 0x1021
  // initial value: 0
  // final xor value: 0
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  // expected values from
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  const vector<uintmax_t> expected{0x2672, 0x20b5, 0x9752, 0xd789, 0x546c,
                                   0x20e4, 0x86d6, 0x9015, 0x31c3};
  ::crcx_ctx ctx = {};

  // This nice thing about CRC's is that you can calculate them
  // incrementally just by varying the LFSR and the input.
  // Below, we're basically just unwinding the loop in crcx() and
  // checking intermediate values.

  // the lfsr value is now 0, then, 0x2672, 0x20b5, ...
  ASSERT_TRUE(::crcx_init(&ctx, 16, 0x1021, 0, 0, false, false));

  ASSERT_TRUE(data.size() == expected.size());
  for (size_t i = 0; i < data.size(); ++i) {
    ::crcx_update(&ctx, data[i]);
    uintmax_t expected_uintmax = expected[i];
    uintmax_t actual_uintmax = ctx.lfsr;
    ASSERT_EQ(actual_uintmax, expected_uintmax)
        << "CRC16 failed at i = " << i << ". "
        << "expected: " << hex << setw(4) << setfill('0') << expected_uintmax
        << " "
        << "actual: " << hex << setw(4) << setfill('0') << actual_uintmax;
  }
}

// this test shows that CRC16 works
TEST(LibCRCx, Sunshine_CRC16_CCIT_ZERO) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC16_CCIT_ZERO
  // input not reflected
  // result not reflected
  // polynomial 0x1021
  // initial value: 0
  // final xor value: 0
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  ::crcx_ctx ctx = {};
  ASSERT_TRUE(::crcx_init(&ctx, 16, 0x1021, 0, 0, false, false));

  // clang-format off
    auto expected_v = to_vector(
        "0x0000 0x1021 0x2042 0x3063 0x4084 0x50A5 0x60C6 0x70E7 0x8108 0x9129 0xA14A 0xB16B 0xC18C 0xD1AD 0xE1CE 0xF1EF "
        "0x1231 0x0210 0x3273 0x2252 0x52B5 0x4294 0x72F7 0x62D6 0x9339 0x8318 0xB37B 0xA35A 0xD3BD 0xC39C 0xF3FF 0xE3DE "
        "0x2462 0x3443 0x0420 0x1401 0x64E6 0x74C7 0x44A4 0x5485 0xA56A 0xB54B 0x8528 0x9509 0xE5EE 0xF5CF 0xC5AC 0xD58D "
        "0x3653 0x2672 0x1611 0x0630 0x76D7 0x66F6 0x5695 0x46B4 0xB75B 0xA77A 0x9719 0x8738 0xF7DF 0xE7FE 0xD79D 0xC7BC "
        "0x48C4 0x58E5 0x6886 0x78A7 0x0840 0x1861 0x2802 0x3823 0xC9CC 0xD9ED 0xE98E 0xF9AF 0x8948 0x9969 0xA90A 0xB92B "
        "0x5AF5 0x4AD4 0x7AB7 0x6A96 0x1A71 0x0A50 0x3A33 0x2A12 0xDBFD 0xCBDC 0xFBBF 0xEB9E 0x9B79 0x8B58 0xBB3B 0xAB1A "
        "0x6CA6 0x7C87 0x4CE4 0x5CC5 0x2C22 0x3C03 0x0C60 0x1C41 0xEDAE 0xFD8F 0xCDEC 0xDDCD 0xAD2A 0xBD0B 0x8D68 0x9D49 "
        "0x7E97 0x6EB6 0x5ED5 0x4EF4 0x3E13 0x2E32 0x1E51 0x0E70 0xFF9F 0xEFBE 0xDFDD 0xCFFC 0xBF1B 0xAF3A 0x9F59 0x8F78 "
        "0x9188 0x81A9 0xB1CA 0xA1EB 0xD10C 0xC12D 0xF14E 0xE16F 0x1080 0x00A1 0x30C2 0x20E3 0x5004 0x4025 0x7046 0x6067 "
        "0x83B9 0x9398 0xA3FB 0xB3DA 0xC33D 0xD31C 0xE37F 0xF35E 0x02B1 0x1290 0x22F3 0x32D2 0x4235 0x5214 0x6277 0x7256 "
        "0xB5EA 0xA5CB 0x95A8 0x8589 0xF56E 0xE54F 0xD52C 0xC50D 0x34E2 0x24C3 0x14A0 0x0481 0x7466 0x6447 0x5424 0x4405 "
        "0xA7DB 0xB7FA 0x8799 0x97B8 0xE75F 0xF77E 0xC71D 0xD73C 0x26D3 0x36F2 0x0691 0x16B0 0x6657 0x7676 0x4615 0x5634 "
        "0xD94C 0xC96D 0xF90E 0xE92F 0x99C8 0x89E9 0xB98A 0xA9AB 0x5844 0x4865 0x7806 0x6827 0x18C0 0x08E1 0x3882 0x28A3 "
        "0xCB7D 0xDB5C 0xEB3F 0xFB1E 0x8BF9 0x9BD8 0xABBB 0xBB9A 0x4A75 0x5A54 0x6A37 0x7A16 0x0AF1 0x1AD0 0x2AB3 0x3A92 "
        "0xFD2E 0xED0F 0xDD6C 0xCD4D 0xBDAA 0xAD8B 0x9DE8 0x8DC9 0x7C26 0x6C07 0x5C64 0x4C45 0x3CA2 0x2C83 0x1CE0 0x0CC1 "
        "0xEF1F 0xFF3E 0xCF5D 0xDF7C 0xAF9B 0xBFBA 0x8FD9 0x9FF8 0x6E17 0x7E36 0x4E55 0x5E74 0x2E93 0x3EB2 0x0ED1 0x1EF0 "
    );
  // clang-format on
  auto actual_v = to_vector((uintmax_t *)ctx.table, ARRAY_SIZE(ctx.table));

  ASSERT_EQ(actual_v, expected_v) << "The generated table is incorrect";

  ASSERT_TRUE(::crcx(&ctx, &data.front(), data.size()));

  uintmax_t expected_uintmax = 0x31c3;
  uintmax_t actual_uintmax = ::crcx_fini(&ctx);

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "The calculated CRC is incorrect";
}

// this test shows that CRC32 works
TEST(LibCRCx, Sunshine_CRC32_POSIX) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC32_POSIX
  // input not reflected
  // result not reflected
  // polynomial 0x4c11db7
  // initial value: 0
  // final xor value: -1
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  ::crcx_ctx ctx = {};
  ASSERT_TRUE(::crcx_init(&ctx, 32, 0x4c11db7, 0, -1, false, false));

  // clang-format off
    auto expected_v = to_vector(
        "0x00000000 0x04C11DB7 0x09823B6E 0x0D4326D9 0x130476DC 0x17C56B6B 0x1A864DB2 0x1E475005 "
        "0x2608EDB8 0x22C9F00F 0x2F8AD6D6 0x2B4BCB61 0x350C9B64 0x31CD86D3 0x3C8EA00A 0x384FBDBD "
        "0x4C11DB70 0x48D0C6C7 0x4593E01E 0x4152FDA9 0x5F15ADAC 0x5BD4B01B 0x569796C2 0x52568B75 "
        "0x6A1936C8 0x6ED82B7F 0x639B0DA6 0x675A1011 0x791D4014 0x7DDC5DA3 0x709F7B7A 0x745E66CD "
        "0x9823B6E0 0x9CE2AB57 0x91A18D8E 0x95609039 0x8B27C03C 0x8FE6DD8B 0x82A5FB52 0x8664E6E5 "
        "0xBE2B5B58 0xBAEA46EF 0xB7A96036 0xB3687D81 0xAD2F2D84 0xA9EE3033 0xA4AD16EA 0xA06C0B5D "
        "0xD4326D90 0xD0F37027 0xDDB056FE 0xD9714B49 0xC7361B4C 0xC3F706FB 0xCEB42022 0xCA753D95 "
        "0xF23A8028 0xF6FB9D9F 0xFBB8BB46 0xFF79A6F1 0xE13EF6F4 0xE5FFEB43 0xE8BCCD9A 0xEC7DD02D "
        "0x34867077 0x30476DC0 0x3D044B19 0x39C556AE 0x278206AB 0x23431B1C 0x2E003DC5 0x2AC12072 "
        "0x128E9DCF 0x164F8078 0x1B0CA6A1 0x1FCDBB16 0x018AEB13 0x054BF6A4 0x0808D07D 0x0CC9CDCA "
        "0x7897AB07 0x7C56B6B0 0x71159069 0x75D48DDE 0x6B93DDDB 0x6F52C06C 0x6211E6B5 0x66D0FB02 "
        "0x5E9F46BF 0x5A5E5B08 0x571D7DD1 0x53DC6066 0x4D9B3063 0x495A2DD4 0x44190B0D 0x40D816BA "
        "0xACA5C697 0xA864DB20 0xA527FDF9 0xA1E6E04E 0xBFA1B04B 0xBB60ADFC 0xB6238B25 0xB2E29692 "
        "0x8AAD2B2F 0x8E6C3698 0x832F1041 0x87EE0DF6 0x99A95DF3 0x9D684044 0x902B669D 0x94EA7B2A "
        "0xE0B41DE7 0xE4750050 0xE9362689 0xEDF73B3E 0xF3B06B3B 0xF771768C 0xFA325055 0xFEF34DE2 "
        "0xC6BCF05F 0xC27DEDE8 0xCF3ECB31 0xCBFFD686 0xD5B88683 0xD1799B34 0xDC3ABDED 0xD8FBA05A "
        "0x690CE0EE 0x6DCDFD59 0x608EDB80 0x644FC637 0x7A089632 0x7EC98B85 0x738AAD5C 0x774BB0EB "
        "0x4F040D56 0x4BC510E1 0x46863638 0x42472B8F 0x5C007B8A 0x58C1663D 0x558240E4 0x51435D53 "
        "0x251D3B9E 0x21DC2629 0x2C9F00F0 0x285E1D47 0x36194D42 0x32D850F5 0x3F9B762C 0x3B5A6B9B "
        "0x0315D626 0x07D4CB91 0x0A97ED48 0x0E56F0FF 0x1011A0FA 0x14D0BD4D 0x19939B94 0x1D528623 "
        "0xF12F560E 0xF5EE4BB9 0xF8AD6D60 0xFC6C70D7 0xE22B20D2 0xE6EA3D65 0xEBA91BBC 0xEF68060B "
        "0xD727BBB6 0xD3E6A601 0xDEA580D8 0xDA649D6F 0xC423CD6A 0xC0E2D0DD 0xCDA1F604 0xC960EBB3 "
        "0xBD3E8D7E 0xB9FF90C9 0xB4BCB610 0xB07DABA7 0xAE3AFBA2 0xAAFBE615 0xA7B8C0CC 0xA379DD7B "
        "0x9B3660C6 0x9FF77D71 0x92B45BA8 0x9675461F 0x8832161A 0x8CF30BAD 0x81B02D74 0x857130C3 "
        "0x5D8A9099 0x594B8D2E 0x5408ABF7 0x50C9B640 0x4E8EE645 0x4A4FFBF2 0x470CDD2B 0x43CDC09C "
        "0x7B827D21 0x7F436096 0x7200464F 0x76C15BF8 0x68860BFD 0x6C47164A 0x61043093 0x65C52D24 "
        "0x119B4BE9 0x155A565E 0x18197087 0x1CD86D30 0x029F3D35 0x065E2082 0x0B1D065B 0x0FDC1BEC "
        "0x3793A651 0x3352BBE6 0x3E119D3F 0x3AD08088 0x2497D08D 0x2056CD3A 0x2D15EBE3 0x29D4F654 "
        "0xC5A92679 0xC1683BCE 0xCC2B1D17 0xC8EA00A0 0xD6AD50A5 0xD26C4D12 0xDF2F6BCB 0xDBEE767C "
        "0xE3A1CBC1 0xE760D676 0xEA23F0AF 0xEEE2ED18 0xF0A5BD1D 0xF464A0AA 0xF9278673 0xFDE69BC4 "
        "0x89B8FD09 0x8D79E0BE 0x803AC667 0x84FBDBD0 0x9ABC8BD5 0x9E7D9662 0x933EB0BB 0x97FFAD0C "
        "0xAFB010B1 0xAB710D06 0xA6322BDF 0xA2F33668 0xBCB4666D 0xB8757BDA 0xB5365D03 0xB1F740B4 "
    );
  // clang-format on
  auto actual_v = to_vector((uintmax_t *)ctx.table, ARRAY_SIZE(ctx.table));

  ASSERT_EQ(actual_v, expected_v) << "The generated table is incorrect";

  ASSERT_TRUE(::crcx(&ctx, &data.front(), data.size()));

  uintmax_t expected_uintmax = 0x765e7680;
  uintmax_t actual_uintmax = ::crcx_fini(&ctx);

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "The calculated CRC is incorrect";
}

// this test shows that CRC64 works
TEST(LibCRCx, Sunshine_CRC64_ECMA_182) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC64_ECMA_182
  // input not reflected
  // result not reflected
  // polynomial 0x4c11db7
  // initial value: 0
  // final xor value: 0
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  ::crcx_ctx ctx = {};
  ASSERT_TRUE(::crcx_init(&ctx, 64, 0x42F0E1EBA9EA3693, 0, 0, false, false));

  // clang-format off
    auto expected_v = to_vector(
        "0x0000000000000000 0x42F0E1EBA9EA3693 0x85E1C3D753D46D26 0xC711223CFA3E5BB5 0x493366450E42ECDF 0x0BC387AEA7A8DA4C 0xCCD2A5925D9681F9 0x8E224479F47CB76A "
        "0x9266CC8A1C85D9BE 0xD0962D61B56FEF2D 0x17870F5D4F51B498 0x5577EEB6E6BB820B 0xDB55AACF12C73561 0x99A54B24BB2D03F2 0x5EB4691841135847 0x1C4488F3E8F96ED4 "
        "0x663D78FF90E185EF 0x24CD9914390BB37C 0xE3DCBB28C335E8C9 0xA12C5AC36ADFDE5A 0x2F0E1EBA9EA36930 0x6DFEFF5137495FA3 0xAAEFDD6DCD770416 0xE81F3C86649D3285 "
        "0xF45BB4758C645C51 0xB6AB559E258E6AC2 0x71BA77A2DFB03177 0x334A9649765A07E4 0xBD68D2308226B08E 0xFF9833DB2BCC861D 0x388911E7D1F2DDA8 0x7A79F00C7818EB3B "
        "0xCC7AF1FF21C30BDE 0x8E8A101488293D4D 0x499B3228721766F8 0x0B6BD3C3DBFD506B 0x854997BA2F81E701 0xC7B97651866BD192 0x00A8546D7C558A27 0x4258B586D5BFBCB4 "
        "0x5E1C3D753D46D260 0x1CECDC9E94ACE4F3 0xDBFDFEA26E92BF46 0x990D1F49C77889D5 0x172F5B3033043EBF 0x55DFBADB9AEE082C 0x92CE98E760D05399 0xD03E790CC93A650A "
        "0xAA478900B1228E31 0xE8B768EB18C8B8A2 0x2FA64AD7E2F6E317 0x6D56AB3C4B1CD584 0xE374EF45BF6062EE 0xA1840EAE168A547D 0x66952C92ECB40FC8 0x2465CD79455E395B "
        "0x3821458AADA7578F 0x7AD1A461044D611C 0xBDC0865DFE733AA9 0xFF3067B657990C3A 0x711223CFA3E5BB50 0x33E2C2240A0F8DC3 0xF4F3E018F031D676 0xB60301F359DBE0E5 "
        "0xDA050215EA6C212F 0x98F5E3FE438617BC 0x5FE4C1C2B9B84C09 0x1D14202910527A9A 0x93366450E42ECDF0 0xD1C685BB4DC4FB63 0x16D7A787B7FAA0D6 0x5427466C1E109645 "
        "0x4863CE9FF6E9F891 0x0A932F745F03CE02 0xCD820D48A53D95B7 0x8F72ECA30CD7A324 0x0150A8DAF8AB144E 0x43A04931514122DD 0x84B16B0DAB7F7968 0xC6418AE602954FFB "
        "0xBC387AEA7A8DA4C0 0xFEC89B01D3679253 0x39D9B93D2959C9E6 0x7B2958D680B3FF75 0xF50B1CAF74CF481F 0xB7FBFD44DD257E8C 0x70EADF78271B2539 0x321A3E938EF113AA "
        "0x2E5EB66066087D7E 0x6CAE578BCFE24BED 0xABBF75B735DC1058 0xE94F945C9C3626CB 0x676DD025684A91A1 0x259D31CEC1A0A732 0xE28C13F23B9EFC87 0xA07CF2199274CA14 "
        "0x167FF3EACBAF2AF1 0x548F120162451C62 0x939E303D987B47D7 0xD16ED1D631917144 0x5F4C95AFC5EDC62E 0x1DBC74446C07F0BD 0xDAAD56789639AB08 0x985DB7933FD39D9B "
        "0x84193F60D72AF34F 0xC6E9DE8B7EC0C5DC 0x01F8FCB784FE9E69 0x43081D5C2D14A8FA 0xCD2A5925D9681F90 0x8FDAB8CE70822903 0x48CB9AF28ABC72B6 0x0A3B7B1923564425 "
        "0x70428B155B4EAF1E 0x32B26AFEF2A4998D 0xF5A348C2089AC238 0xB753A929A170F4AB 0x3971ED50550C43C1 0x7B810CBBFCE67552 0xBC902E8706D82EE7 0xFE60CF6CAF321874 "
        "0xE224479F47CB76A0 0xA0D4A674EE214033 0x67C58448141F1B86 0x253565A3BDF52D15 0xAB1721DA49899A7F 0xE9E7C031E063ACEC 0x2EF6E20D1A5DF759 0x6C0603E6B3B7C1CA "
        "0xF6FAE5C07D3274CD 0xB40A042BD4D8425E 0x731B26172EE619EB 0x31EBC7FC870C2F78 0xBFC9838573709812 0xFD39626EDA9AAE81 0x3A28405220A4F534 0x78D8A1B9894EC3A7 "
        "0x649C294A61B7AD73 0x266CC8A1C85D9BE0 0xE17DEA9D3263C055 0xA38D0B769B89F6C6 0x2DAF4F0F6FF541AC 0x6F5FAEE4C61F773F 0xA84E8CD83C212C8A 0xEABE6D3395CB1A19 "
        "0x90C79D3FEDD3F122 0xD2377CD44439C7B1 0x15265EE8BE079C04 0x57D6BF0317EDAA97 0xD9F4FB7AE3911DFD 0x9B041A914A7B2B6E 0x5C1538ADB04570DB 0x1EE5D94619AF4648 "
        "0x02A151B5F156289C 0x4051B05E58BC1E0F 0x87409262A28245BA 0xC5B073890B687329 0x4B9237F0FF14C443 0x0962D61B56FEF2D0 0xCE73F427ACC0A965 0x8C8315CC052A9FF6 "
        "0x3A80143F5CF17F13 0x7870F5D4F51B4980 0xBF61D7E80F251235 0xFD913603A6CF24A6 0x73B3727A52B393CC 0x31439391FB59A55F 0xF652B1AD0167FEEA 0xB4A25046A88DC879 "
        "0xA8E6D8B54074A6AD 0xEA16395EE99E903E 0x2D071B6213A0CB8B 0x6FF7FA89BA4AFD18 0xE1D5BEF04E364A72 0xA3255F1BE7DC7CE1 0x64347D271DE22754 0x26C49CCCB40811C7 "
        "0x5CBD6CC0CC10FAFC 0x1E4D8D2B65FACC6F 0xD95CAF179FC497DA 0x9BAC4EFC362EA149 0x158E0A85C2521623 0x577EEB6E6BB820B0 0x906FC95291867B05 0xD29F28B9386C4D96 "
        "0xCEDBA04AD0952342 0x8C2B41A1797F15D1 0x4B3A639D83414E64 0x09CA82762AAB78F7 0x87E8C60FDED7CF9D 0xC51827E4773DF90E 0x020905D88D03A2BB 0x40F9E43324E99428 "
        "0x2CFFE7D5975E55E2 0x6E0F063E3EB46371 0xA91E2402C48A38C4 0xEBEEC5E96D600E57 0x65CC8190991CB93D 0x273C607B30F68FAE 0xE02D4247CAC8D41B 0xA2DDA3AC6322E288 "
        "0xBE992B5F8BDB8C5C 0xFC69CAB42231BACF 0x3B78E888D80FE17A 0x7988096371E5D7E9 0xF7AA4D1A85996083 0xB55AACF12C735610 0x724B8ECDD64D0DA5 0x30BB6F267FA73B36 "
        "0x4AC29F2A07BFD00D 0x08327EC1AE55E69E 0xCF235CFD546BBD2B 0x8DD3BD16FD818BB8 0x03F1F96F09FD3CD2 0x41011884A0170A41 0x86103AB85A2951F4 0xC4E0DB53F3C36767 "
        "0xD8A453A01B3A09B3 0x9A54B24BB2D03F20 0x5D45907748EE6495 0x1FB5719CE1045206 0x919735E51578E56C 0xD367D40EBC92D3FF 0x1476F63246AC884A 0x568617D9EF46BED9 "
        "0xE085162AB69D5E3C 0xA275F7C11F7768AF 0x6564D5FDE549331A 0x279434164CA30589 0xA9B6706FB8DFB2E3 0xEB46918411358470 0x2C57B3B8EB0BDFC5 0x6EA7525342E1E956 "
        "0x72E3DAA0AA188782 0x30133B4B03F2B111 0xF7021977F9CCEAA4 0xB5F2F89C5026DC37 0x3BD0BCE5A45A6B5D 0x79205D0E0DB05DCE 0xBE317F32F78E067B 0xFCC19ED95E6430E8 "
        "0x86B86ED5267CDBD3 0xC4488F3E8F96ED40 0x0359AD0275A8B6F5 0x41A94CE9DC428066 0xCF8B0890283E370C 0x8D7BE97B81D4019F 0x4A6ACB477BEA5A2A 0x089A2AACD2006CB9 "
        "0x14DEA25F3AF9026D 0x562E43B4931334FE 0x913F6188692D6F4B 0xD3CF8063C0C759D8 0x5DEDC41A34BBEEB2 0x1F1D25F19D51D821 0xD80C07CD676F8394 0x9AFCE626CE85B507 "
    );
  // clang-format on
  auto actual_v = to_vector((uintmax_t *)ctx.table, ARRAY_SIZE(ctx.table));

  ASSERT_EQ(actual_v, expected_v) << "The generated table is incorrect";

  ASSERT_TRUE(::crcx(&ctx, &data.front(), data.size()));

  uintmax_t expected_uintmax = 0x6c40df5f0b497347;
  uintmax_t actual_uintmax = ::crcx_fini(&ctx);

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "The calculated CRC is incorrect";
}

TEST(LibCRCx, reflect24) {
  uintmax_t expected_uintmax = 0xabcdef;
  uintmax_t actual_uintmax = ::crcx_reflect(0xf7b3d5, 24);

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "expected: " << hex << setw(6) << setfill('0') << expected_uintmax
      << "actual:   " << hex << setw(6) << setfill('0') << actual_uintmax;
}

// BLE CRC polynomial is  x^24 + x^10 + x^9 + x^6 + x^4 + x^3 + x + 1
// This corresponds to 0x[1]00065b
// In the advertising channel, the BLE CRC initializer is 0x555555

const size_t ble_crc_n = 24;
const uintmax_t ble_poly(0x100065b);
const uintmax_t ble_init(0x555555);
const uintmax_t ble_fini(0);
const bool ble_reflect_input = true;
const bool ble_reflect_output = false;

enum {
  ADV_IND = 0x0,
  ADV_NONCONN_IND = 0x2,
  BDADDR_SIZE = 6,
  PDU_AC_LL_HEADER_SIZE = 2,
  ADV_DATA_LEN = 31,
};

// adapted from Zephyr
struct pdu_adv {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  uint8_t type : 4;
  uint8_t rfu : 1;
  uint8_t chan_sel : 1;
  uint8_t tx_addr : 1;
  uint8_t rx_addr : 1;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  uint8_t rx_addr : 1;
  uint8_t tx_addr : 1;
  uint8_t chan_sel : 1;
  uint8_t rfu : 1;
  uint8_t type : 4;
#else
#error "Unsupported endianness"
#endif

  uint8_t len;

  // for adv_ind
  uint8_t addr[BDADDR_SIZE];
  uint8_t data[ADV_DATA_LEN];
} __attribute__((packed));

TEST(LibCRCx, board_example1) {

  vector<uint8_t> rxbuf{{0x2c, 0xc5, 0x22, 0x44, 0x05, 0x6b, 0x8e, 0x55, 0x6b,
                         0xcd, 0xde, 0x1e, 0xb0, 0x6f, 0xc0, 0x4a, 0x85, 0x7b,
                         0x29, 0x32, 0x18, 0x8d, 0x02, 0x0a, 0x00, 0x24, 0x00,
                         0x00, 0x00, 0xf4, 0x01, 0x00, 0xfe, 0xff, 0xff, 0x1f,
                         0x08, 0x10, 0x80, 0xbd, 0xcb, 0x1d, 0x98, 0x94, 0x06}};

  size_t offs = rxbuf[0];
  uint8_t *data = &rxbuf[1];

  uint32_t timestamp = 0;

  timestamp |= data[--offs] << 24;
  timestamp |= data[--offs] << 16;
  timestamp |= data[--offs] << 8;
  timestamp |= data[--offs] << 0;

  ASSERT_EQ(110401565, timestamp);

  int8_t rssi = int8_t(data[--offs]);
  ASSERT_EQ(rssi, -53);

  uint32_t hw_crc = 0;

  hw_crc |= data[--offs] << 16;
  hw_crc |= data[--offs] << 8;
  hw_crc |= data[--offs] << 0;

  size_t len = offs;

  const uint32_t wireshark_crc(0x0801bd);

  ::crcx_ctx ctx = {};
  ASSERT_TRUE(::crcx_init(&ctx, ble_crc_n, ble_poly, ble_init, ble_fini,
                          ble_reflect_input, ble_reflect_output));
  ASSERT_TRUE(::crcx(&ctx, data, len));
  uintmax_t sw_crc = ::crcx_fini(&ctx);

  // Succeeds
  EXPECT_EQ(sw_crc, wireshark_crc)
      << "SW CRC (" << hex << setw(6) << setfill('0') << sw_crc
      << ") does not match Wireshark CRC (" << hex << setw(6) << setfill('0')
      << wireshark_crc << ")";
}

TEST(LibCRCx, nrf_support1) {
  // https://devzone.nordicsemi.com/f/nordic-q-a/679/ble-crc-calculation

  // oddly getting "non-trivial designated initializers not supported" error
  // only on s390x
  pdu_adv pdu = {};
  pdu.type = ADV_IND;
  pdu.len = 9;

  // note, this is least-significant byte first
  pdu.addr[0] = 0x0d;
  pdu.addr[1] = 0xef;
  pdu.addr[2] = 0x84;
  pdu.addr[3] = 0xb7;
  pdu.addr[4] = 0x2d;
  pdu.addr[5] = 0x3c;

  pdu.data[0] = 0x02;
  pdu.data[1] = 0x01;
  pdu.data[2] = 0x05;

  const size_t pdu_len = PDU_AC_LL_HEADER_SIZE + pdu.len;

  ::crcx_ctx ctx = {};
  ASSERT_TRUE(::crcx_init(&ctx, ble_crc_n, ble_poly, ble_init, ble_fini,
                          ble_reflect_input, ble_reflect_output));
  ASSERT_TRUE(::crcx(&ctx, &pdu, pdu_len));
  uintmax_t expected_uintmax = ::crcx_reflect(0xa4e2c2, ctx.n);
  uintmax_t actual_uintmax = ::crcx_fini(&ctx);

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "expected: " << hex << setw(6) << setfill('0') << expected_uintmax
      << " "
      << "actual: " << hex << setw(6) << setfill('0') << actual_uintmax << " ";
}

TEST(LibCRCx, ble_core_52_4_2_1_Legacy_Advertising_PDUs) {
  // https://www.bluetooth.com/specifications/bluetooth-core-specification/

  // oddly getting "non-trivial designated initializers not supported" error
  // only on s390x
  pdu_adv pdu = {};
  pdu.type = ADV_NONCONN_IND;
  pdu.tx_addr = 1;
  pdu.len = 9;

  // note, this is least-significant byte first
  pdu.addr[0] = 0xa6;
  pdu.addr[1] = 0xa5;
  pdu.addr[2] = 0xa4;
  pdu.addr[3] = 0xa3;
  pdu.addr[4] = 0xa2;
  pdu.addr[5] = 0xc1;

  pdu.data[0] = 0x01;
  pdu.data[1] = 0x02;
  pdu.data[2] = 0x03;

  const size_t pdu_len = PDU_AC_LL_HEADER_SIZE + pdu.len;

  ::crcx_ctx ctx = {};
  ASSERT_TRUE(::crcx_init(&ctx, ble_crc_n, ble_poly, ble_init, ble_fini,
                          ble_reflect_input, ble_reflect_output));
  ASSERT_TRUE(::crcx(&ctx, &pdu, pdu_len));
  uintmax_t expected_uintmax = 0xb52dd7;
  uintmax_t actual_uintmax = ::crcx_fini(&ctx);

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "expected: " << hex << setw(6) << setfill('0') << expected_uintmax
      << " "
      << "actual: " << hex << setw(6) << setfill('0') << actual_uintmax << " ";
}
