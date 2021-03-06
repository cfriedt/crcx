#include <vector>

#include <gtest/gtest.h>

#include "crc3x/crc3x.h"

using namespace std;
using namespace crc3x;

// This test shows that the CRC works for a single byte.
// It also tests to make sure that the input parameters are set accordingly.
// Also, it checks that the LUT is generated correctly.
TEST(LibCRC3x, Wikipedia_example1) {
  // https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#Example
  // compute the 8-bit CRC of the ascii character 'W' (0b01010111, 0x57, 87)
  // CRC-8-ATM (HEC) polynomial x^8 + x^2 + x + 1 => '100000111'

  using Crc3x = Crc<uint8_t, 8, 0x7>;

  Crc3x crc(0, 0, false, false);

  // This site has a table generator and gives the same output as the wikipedia
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // clang-format off
  decltype(Crc3x::table) expected_table = {
	0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3,
  };
  // clang-format on

  auto &actual_table = Crc3x::table;

  ASSERT_EQ(actual_table, expected_table) << "The generated table is incorrect";

  crc.update(uint8_t('W'));

  auto expected = 0xa2;
  auto actual = crc.fini();

  EXPECT_EQ(actual, expected) << "The calculated CRC is incorrect";
}

// this test shows that multi-byte input CRC works
TEST(LibCRC3x, Sunshine_CRC8) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC8
  // input not reflected
  // result not reflected
  // polynomial 0x07 => x^8 + x^2 + x + 1
  // initial value: 0
  // final xor value: 0
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  using Crc3x = Crc<uint8_t, 8, 0x7>;

  Crc3x crc(0, 0, false, false);

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  crc.update(data.begin(), data.end());

  auto expected = 0xf4;
  auto actual = crc.fini();

  EXPECT_EQ(actual, expected) << "The calculated CRC is incorrect";
}

// this test shows that the final xor value is applied
TEST(LibCRC3x, Sunshine_CRC8_ITU) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC8_ITU
  // input not reflected
  // result not reflected
  // polynomial 0x07 => x^8 + x^2 + x + 1
  // initial value: 0
  // final xor value: 0x55
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  using Crc3x = Crc<uint8_t, 8, 0x7>;

  Crc3x crc(0, 0x55, false, false);

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  crc.update(data.begin(), data.end());

  auto expected = 0xa1;
  auto actual = crc.fini();

  EXPECT_EQ(actual, expected) << "The calculated CRC is incorrect";
}

// this test shows that input and output reflection works
TEST(LibCRC3x, Sunshine_CRC8_DARC) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC8_DARC
  // input reflected
  // result reflected
  // polynomial 0x39 => x^8 + 1x^5 + 1x^4 + 1x^3 1x
  // initial value: 0
  // final xor value: 0
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  using Crc3x = Crc<uint8_t, 8, 0x39>;

  Crc3x crc(0, 0, true, true);

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  crc.update(data.begin(), data.end());

  auto expected = 0x15;
  auto actual = crc.fini();

  EXPECT_EQ(actual, expected) << "The calculated CRC is incorrect";
}

// this test shows that CRC16 works for one byte messages
TEST(LibCRC3x, Sunshine_CRC16_CCIT_ZERO_one_byte) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC16_CCIT_ZERO
  // input not reflected
  // result not reflected
  // polynomial 0x1021
  // initial value: 0
  // final xor value: 0
  // CRC Input Data: 'W'

  using Crc3x = Crc<uint16_t, 16, 0x1021>;

  Crc3x crc(0, 0, false, false);

  const vector<uint8_t> data{'W'};

  crc.update(data.begin(), data.end());

  auto expected = 0x2a12;
  auto actual = crc.fini();

  EXPECT_EQ(actual, expected) << "The calculated CRC is incorrect";
}

// this test shows that CRC16 works
TEST(LibCRC3x, Sunshine_CRC16_CCIT_ZERO) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC16_CCIT_ZERO
  // input not reflected
  // result not reflected
  // polynomial 0x1021
  // initial value: 0
  // final xor value: 0
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  using Crc3x = Crc<uint16_t, 16, 0x1021>;

  Crc3x crc(0, 0, false, false);

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  crc.update(data.begin(), data.end());

  auto expected = 0x31c3;
  auto actual = crc.fini();

  EXPECT_EQ(actual, expected) << "The calculated CRC is incorrect";
}

// this test shows that CRC32 works
TEST(LibCRC3x, Sunshine_CRC32_POSIX) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC32_POSIX
  // input not reflected
  // result not reflected
  // polynomial 0x4c11db7
  // initial value: 0
  // final xor value: -1
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  using Crc3x = Crc<uint32_t, 32, 0x4c11db7>;

  Crc3x crc(0, -1, false, false);

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  crc.update(data.begin(), data.end());

  auto expected = 0x765e7680;
  auto actual = crc.fini();

  EXPECT_EQ(actual, expected) << "The calculated CRC is incorrect";
}

// this test shows that CRC64 works
TEST(LibCRC3x, Sunshine_CRC64_ECMA_182) {
  // http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
  // CRC64_ECMA_182
  // input not reflected
  // result not reflected
  // polynomial 0x4c11db7
  // initial value: 0
  // final xor value: 0
  // CRC Input Data: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39

  using Crc3x = Crc<uint64_t, 64, 0x42F0E1EBA9EA3693>;

  Crc3x crc(0, 0, false, false);

  const vector<uint8_t> data{0x31, 0x32, 0x33, 0x34, 0x35,
                             0x36, 0x37, 0x38, 0x39};

  crc.update(data.begin(), data.end());

  auto expected = 0x6c40df5f0b497347;
  auto actual = crc.fini();

  EXPECT_EQ(actual, expected) << "The calculated CRC is incorrect";
}

TEST(LibCRC3x, reflect24) {
  uintmax_t expected_uintmax = 0xabcdef;
  uintmax_t actual_uintmax = reflect(0xf7b3d5, 24);

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "expected: " << hex << setw(6) << setfill('0') << expected_uintmax
      << "actual:   " << hex << setw(6) << setfill('0') << actual_uintmax;
}

// BLE CRC polynomial is  x^24 + x^10 + x^9 + x^6 + x^4 + x^3 + x + 1
// This corresponds to 0x[1]00065b
// In the advertising channel, the BLE CRC initializer is 0x555555

const size_t ble_crc_n = 24;
const uintmax_t ble_poly(0x00065b);
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

TEST(LibCRC3x, board_example1) {

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

  using Crc3x = Crc<uint32_t, ble_crc_n, uint32_t(ble_poly)>;
  Crc3x crc(uint32_t(ble_init), uint32_t(ble_fini), ble_reflect_input,
            ble_reflect_output);
  crc.update(data, data + len);
  uintmax_t sw_crc = crc.fini();

  EXPECT_EQ(sw_crc, wireshark_crc)
      << "SW CRC (" << hex << setw(6) << setfill('0') << sw_crc
      << ") does not match Wireshark CRC (" << hex << setw(6) << setfill('0')
      << wireshark_crc << ")";
}

TEST(LibCRC3x, nrf_support1) {
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

  using Crc3x = Crc<uint32_t, ble_crc_n, uint32_t(ble_poly)>;
  Crc3x crc(uint32_t(ble_init), uint32_t(ble_fini), ble_reflect_input,
            ble_reflect_output);
  crc.update((uint8_t *)&pdu, ((uint8_t *)&pdu) + pdu_len);

  uintmax_t expected_uintmax = reflect(0xa4e2c2, ble_crc_n);
  uintmax_t actual_uintmax = crc.fini();

  EXPECT_EQ(actual_uintmax, expected_uintmax)
      << "expected: " << hex << setw(6) << setfill('0') << expected_uintmax
      << " "
      << "actual: " << hex << setw(6) << setfill('0') << actual_uintmax << " ";
}

TEST(LibCRC3x, ble_core_52_4_2_1_Legacy_Advertising_PDUs) {
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

  using Crc3x = Crc<uint32_t, ble_crc_n, uint32_t(ble_poly)>;
  Crc3x crc(uint32_t(ble_init), uint32_t(ble_fini), ble_reflect_input,
            ble_reflect_output);
  crc.update((uint8_t *)&pdu, ((uint8_t *)&pdu) + pdu_len);

  auto expected = 0xb52dd7;
  auto actual = crc.fini();

  EXPECT_EQ(actual, expected)
      << "expected: " << hex << setw(6) << setfill('0') << expected << " "
      << "actual: " << hex << setw(6) << setfill('0') << actual << " ";
}

// see https://tools.ietf.org/html/rfc1662

// The table in rfc1662 is "reflected", using the terminology of 
// http://www.sunshine2k.de/coding/javascript/crc/crc_js.html

// Currently, CRCx does not use reflected tables
// It is likely far more computationally efficient to do so when bits should be processed MSB first.

// At risk of generalizing, it would seem to be the case that using "reflected tables" is an optimization
// that avoids having to reflect input bits and reflect output bits.

// A side effect of not using reflected tables seems to be that the check at the end becoes zero
// instead of 0xf0b8.

constexpr size_t hdlc_crc_n = 16;
constexpr uint16_t hdlc_poly = 0x1021;
constexpr uint16_t hdlc_init = 0xffff;
constexpr uint16_t hdlc_check = 0; // 0xf0b8 <-- only when using "reflected tables"
constexpr uint16_t hdlc_fini = 0;
constexpr bool hdlc_reflect_input = true;
constexpr bool hdlc_reflect_output = true;

TEST(LibCRC3x, rfc1662_fcs16) {
  // https://tools.ietf.org/html/rfc1662

  // Created using the example code in Section C.2 of the above RFC
  // with the string "Hello, HDLC world!"
  // With HDLC, the first step is to calculate the CRC of the message using the initializer 0xffff.
  // That results in the CRC-16 0x0530.
  // Then, append the CRC to the message (MSB first).
  // Then, every received message can be verified against the constant 0xf0b8
  // When using the same algorithm and table shown in the RFC, that can all be verified to work fine.
  string msg = "Hello, HDLC world!";
  vector<uint8_t> data(msg.begin(), msg.end());
  uint16_t fcs = 0x0530;

  // first, let's verify that we can get the same crc by *not* using the algorithm and table in rfc1662
  using Crc3x = Crc<uint16_t, hdlc_crc_n, hdlc_poly>;

  Crc3x crc(hdlc_init, hdlc_fini, hdlc_reflect_input, hdlc_reflect_output);

  crc.update(data.begin(), data.end());

  auto expected = fcs;
  auto actual = crc.fini();

  ASSERT_EQ(actual, expected) << "The calculated CRC is incorrect";

  data.push_back(fcs);
  data.push_back(fcs >> 8);

  crc.update(data.begin(), data.end());

  // a side-effect of *not* using "reflected tables" is that input & output must be reflected
  // which is computationally expensive, and also that the check at the end is no longer 0xf0b8 but 0.

  expected = hdlc_check;
  actual = crc.fini();

  EXPECT_EQ(actual, expected) << "HDLC check failed";
}
