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

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "crcx/crcx.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifndef MAX
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#endif

#if defined(DEBUG)
#include <stdio.h>
#define D(fmt, args...)                                                        \
  printf("%s(): %d: " fmt "\n", __func__, __LINE__, ##args)
#else
#define D(fmt, args...)
#endif

#if defined(DEBUG)

static void crc_print_table(const struct crcx_ctx *ctx) {
  const size_t nibbles_per_entry = ctx->n / 4;
  const size_t cols_per_row = 8;
  const size_t cols = cols_per_row;
  const size_t rows = ARRAY_SIZE(ctx->table) / cols_per_row;

  char fmt[16];
  snprintf(fmt, sizeof(fmt), "%%0%" PRIxMAX "x ", nibbles_per_entry);

  for (size_t row = 0; row < rows; ++row) {
    for (size_t col = 0; col < cols; ++col) {
      printf(fmt, ctx->table[row * cols + col]);
    }
    putchar('\n');
  }
}
#else
static inline void crc_print_table(const struct crcx_ctx *ctx) { (void)ctx; }
#endif

// This can probably be done nibble-wise with a LUT and a bunch of shift in
// constant time followed by a shift corresponding to the leading number of
// zeros
uintmax_t crcx_reflect(const uintmax_t x, const uint8_t n) {

  const uint8_t _n = MIN(n, 8 * sizeof(uintmax_t));

  uintmax_t y = 0;

  for (size_t i = 0; i < _n; ++i) {
    bool bit = (x >> i) & 1;
    y |= bit << ((_n - 1) - i);
  }

  return y;
}

bool crcx_valid(const struct crcx_ctx *ctx) {

  if (NULL == ctx) {
    D("ctx is NULL");
    return false;
  }

  if (0 == ctx->poly) {
    D("0 is not a valid polynomial");
    return false;
  }

  if (0 == ctx->n || ctx->n > 8 * sizeof(uintmax_t) || 0 != ctx->n % 8) {
    D("invalid value for ctx->n: %u", ctx->n);
    return false;
  }

  if (ctx->msb != ((uintmax_t)1 << (ctx->n - 1))) {
    D("invalid value for ctx->msb: expected: %" PRIxMAX " actual: %" PRIxMAX,
      (uintmax_t)1 << (ctx->n - 1), ctx->msb);
    return false;
  }

  if (ctx->n == 8 * sizeof(uintmax_t)) {
    if (ctx->mask != (uintmax_t)-1) {
      D("invalid value for ctx->msb: expected: %" PRIxMAX " actual: %" PRIxMAX,
        (uintmax_t)-1, ctx->mask);
      return false;
    }
  } else {
    if (ctx->mask != (1ULL << ctx->n) - 1) {
      D("invalid value for ctx->msb: expected: %" PRIxMAX " actual: %" PRIxMAX,
        ((uintmax_t)1 << ctx->n) - 1, ctx->mask);
      return false;
    }
  }

  // highest bit pos (0-indexed). depends on poly not being zero
  const uint8_t highest_bit_pos =
      8 * sizeof(uintmax_t) - __builtin_clzll(ctx->poly) - 1;
  if (ctx->n < highest_bit_pos) {
    D("invalid polynomial %" PRIxMAX " for an %u-bit CRC", ctx->poly, ctx->n);
    return false;
  }

  return true;
}

bool crcx_generate_table(struct crcx_ctx *ctx) {
  uintmax_t *table = (uintmax_t *)ctx->table;

  if (!crcx_valid(ctx)) {
    return false;
  }

  table[0] = 0;
  uintmax_t crc = ctx->msb;
  for (size_t i = 1; i < 256; i <<= 1) {
    if (crc & ctx->msb) {
      crc <<= 1;
      crc ^= ctx->poly;
    } else {
      crc <<= 1;
    }
    crc &= ctx->mask;
    for (size_t j = 0; j < i; ++j) {
      table[i ^ j] = crc ^ table[j];
    }
  }

  crc_print_table(ctx);

  return true;
}

#define SET(t, k, v) *((t *)(&(k))) = (v)

bool crcx_init(struct crcx_ctx *ctx, uint8_t n, uintmax_t poly, uintmax_t init,
               uintmax_t fini, bool reflect_input, bool reflect_output) {

  SET(uint8_t, ctx->n, n);
  SET(uintmax_t, ctx->poly, poly);
  SET(bool, ctx->reflect_input, reflect_input);
  SET(bool, ctx->reflect_output, reflect_output);

  SET(uintmax_t, ctx->msb, (uintmax_t)1 << (ctx->n - 1));
  if (ctx->n == 8 * sizeof(uintmax_t)) {
    SET(uintmax_t, ctx->mask, (uintmax_t)(-1));
  } else {
    SET(uintmax_t, ctx->mask, ((uintmax_t)1 << ctx->n) - 1);
  }

  SET(uintmax_t, ctx->init, init & ctx->mask);
  SET(uintmax_t, ctx->fini, fini & ctx->mask);

  if (!crcx_valid(ctx)) {
    return false;
  }

  ctx->lfsr = ctx->init;
  memset((uintmax_t *)ctx->table, 0, sizeof(ctx->table));
  return crcx_generate_table(ctx);
}

uintmax_t crcx_fini(struct crcx_ctx *ctx) {

  if (!crcx_valid(ctx)) {
    return -1;
  }

  ctx->lfsr ^= ctx->fini;
  ctx->lfsr &= ctx->mask;
  D("lfsr: %" PRIxMAX, ctx->lfsr);

  if (ctx->reflect_output) {
    ctx->lfsr = crcx_reflect(ctx->lfsr, ctx->n);
    D("lfsr: %" PRIxMAX, ctx->lfsr);
  }

  return ctx->lfsr;
}

void crcx_update(struct crcx_ctx *ctx, uint8_t data) {

  if (ctx->reflect_input) {
    D("reflecting: %02x => %02x", data, (uint8_t)crcx_reflect(data, 8));
    data = crcx_reflect(data, 8);
  }

  // https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#Multi-bit_computation
  uint8_t upper_byte = (ctx->lfsr >> (ctx->n - 8));
  uint8_t idx = data ^ upper_byte;

  ctx->lfsr <<= 8;
  ctx->lfsr &= ctx->mask;
  ctx->lfsr ^= ctx->table[idx];

  D("data: %u lfsr: %" PRIxMAX, data, ctx->lfsr);
}

bool crcx(struct crcx_ctx *ctx, const void *data, const size_t len) {

  if (!crcx_valid(ctx)) {
    return false;
  }

  for (size_t i = 0; i < len; ++i) {
    crcx_update(ctx, ((const uint8_t *)data)[i]);
  }

  return true;
}
