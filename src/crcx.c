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

#include "crcx.h"

#undef DEBUG
#if defined(DEBUG)
#include <stdio.h>
#define D(fmt, args...)                                                        \
  printf("%s(): %d: " fmt "\n", __func__, __LINE__, ##args)
#else
#define D(fmt, args...)
#endif

#if defined(DEBUG)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static void crc_print_table(const struct crcx_ctx *ctx) {
  const size_t nibbles_per_entry = ctx->n / 4;
  const size_t cols_per_row = 8;
  const size_t cols = cols_per_row;
  const size_t rows = ARRAY_SIZE(ctx->table) / cols_per_row;

  char fmt[16];
  snprintf(fmt, sizeof(fmt), "%%0%" PRIx64 "x ", nibbles_per_entry);

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

uintmax_t crcx_reflect(const uintmax_t x, const uint8_t n) {
  D("x: %" PRIx64 " n: %u", x, n);

  uintmax_t y = 0;
  for (size_t i = 0; i < n; ++i) {
    bool bit = (x >> i) & 1;
    y |= bit << ((n - 1) - i);
  }

  D("y: %" PRIx64, y);

  return y;
}

void crcx_generate_table(struct crcx_ctx *ctx) {
  uintmax_t *table = (uintmax_t *)ctx->table;

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
}

#define SET(t, k, v) *((t *)(&(k))) = v

bool crcx_init(struct crcx_ctx *ctx) {
  D("");

  if (0 == ctx->n || ctx->n > 8 * sizeof(uintmax_t) || 0 != ctx->n % 8) {
    D("invlalid value for ctx->n: %u", ctx->n);
    return false;
  }

  memset((uintmax_t *)ctx->table, 0, sizeof(ctx->table));

  SET(uintmax_t, ctx->msb, 1 << (ctx->n - 1));
  if (ctx->n == 8 * sizeof(uintmax_t)) {
    SET(uintmax_t, ctx->mask, -1);
  } else {
    SET(uintmax_t, ctx->mask, (1 << ctx->n) - 1);
  }

  SET(uintmax_t, ctx->init, ctx->init & ctx->mask);
  SET(uintmax_t, ctx->fini, ctx->fini & ctx->mask);

  ctx->lfsr = ctx->init;

  crcx_generate_table(ctx);

  return true;
}

bool crcx_init_args(struct crcx_ctx *ctx, uint8_t n, uintmax_t init,
                    uintmax_t fini, uintmax_t poly, bool reflect_input,
                    bool reflect_output) {
  D("");

  SET(uintmax_t, ctx->n, n);
  SET(uintmax_t, ctx->init, init);
  SET(uintmax_t, ctx->fini, fini);
  SET(uintmax_t, ctx->poly, poly);
  SET(bool, ctx->reflect_input, reflect_input);
  SET(bool, ctx->reflect_output, reflect_output);

  return crcx_init(ctx);
}

void crcx_fini(struct crcx_ctx *ctx) {
  D("");

  ctx->lfsr ^= ctx->fini;
  ctx->lfsr &= ctx->mask;

  if (ctx->reflect_output) {
    ctx->lfsr = crcx_reflect(ctx->lfsr, ctx->n);
  }
}

void crcx_update(struct crcx_ctx *ctx, const uint8_t data) {
  D("");

  uint8_t d;

  if (ctx->reflect_input) {
    d = crcx_reflect(data, 8);
  } else {
    d = data;
  }

  uint8_t idx = ctx->lfsr ^ d;
  ctx->lfsr = (ctx->lfsr << 8) ^ ctx->table[idx];
  ctx->lfsr &= ctx->mask;
}

uintmax_t crcx(struct crcx_ctx *ctx, const void *data, const size_t len) {
  D("");

  if (!crcx_init(ctx)) {
    return -1;
  }

  for (size_t i = 0; i < len; ++i) {
    crcx_update(ctx, ((const uint8_t *)data)[i]);
  }

  crcx_fini(ctx);

  return ctx->lfsr;
}
