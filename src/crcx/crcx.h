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

/**
 * @file
 *
 * LibCRCx - The C API
 *
 * Typical use cases of libcrcx would be something like the following:
 *
 * @code{.c}
 * #include <crcx/crcx.h>
 * int main() {
 *   const char data[] = { 'W' };
 *   // for embedded, consider making this either static or heap allocated (e.g. with malloc(3))
 *   struct crcx_ctx ctx = {};
 *   // Initialize an 8-bit CRC with no initializer or finalizer, where
 *   // neither the input or output is reflected (in that order).
 *   crcx_init(&ctx, 8, 0, 0, 0x07, false, false);
 *   crcx(&ctx, data, sizeof(data));
 *   uintmax_t crc = crcx_fini(&ctx);
 *   // The value of crc should be 0xa2.
 *   return 0;
 * }
 * @endcode
 *
 */

#ifndef CRCX_H_
#define CRCX_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

/**
 * CRC context
 *
 * This structure contains all of the necessary context to compute the cyclic
 * redundancy check for binary data.
 * The "normal" polynomial representation is used. It only works for CRC's that
 * are a multiple of 8 bits in length.
 *
 * @see <a href="https://en.wikipedia.org/wiki/Cyclic_redundancy_check">Cyclic
 * redundancy check (CRC)</a>
 * @see <a
 * href="https://en.wikipedia.org/wiki/Cyclic_redundancy_check#Polynomial_representations_of_cyclic_redundancy_checks">Polynomial
 * representations of cyclic redundancy checks</a>
 */
struct crcx_ctx {
  // clang-format off
  const uint8_t n;             ///< number of bits in CRC. For E.g. CRC-8 use 8
  const uintmax_t init;        ///< initial value stored in the lfsr
  const uintmax_t fini;        ///< final value xor'ed with the lfsr
  const uintmax_t poly;        ///< the polynomial used for CRC calculations
  const uintmax_t mask;        ///< a bitmask used internally that is set to (1 << @n) - 1
  const uintmax_t msb;         ///< a bitmask used internally that is set to 1 << (@n - 1)
  const bool reflect_input;    ///< perform a bitwise reversal of each input byte
  const bool reflect_output;   ///< perform a bitwise reversal of the result of the CRC calculation
  const uintmax_t table[256];  ///< a table used to store CRC values for individual bytes
  uintmax_t lfsr;              ///< the modeled linear feedback shift register
  // clang-format on
};

/**
 * Reflect the least-significant @p n bits of @p x
 *
 * This function truncates @p n to the number of bits contained in a uintmax_t.
 *
 * It does not fail.
 *
 * @param x  the variable containing the bits to reflect
 * @param n  the number of bits to reflect
 * @return   x with the least-significant @p n bits of @p x reflected
 */
uintmax_t crcx_reflect(const uintmax_t x, const uint8_t n);

/**
 * Check a CRC context for validity
 *
 * @param ctx the CRC context to check
 *
 * @return true if @p ctx is valid, otherwise false
 */
bool crcx_valid(const struct crcx_ctx *ctx);

/**
 * Generate the CRC Lookup Table (LUT)
 *
 * @param ctx the CRC context
 *
 * @return true if the table is generated, otherwise false
 *
 * @see <a
 * href="https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#Generating_the_tables">Computation
 * of cyclic redundancy checks: Generating the tables</a>
 */
bool crcx_generate_table(struct crcx_ctx *ctx);

/**
 * Initialize a CRC context with the given parameters.
 *
 * Warning: @p init and @p fini will be truncated to @p n bits
 *
 * @param ctx             the CRC context to initialize
 * @param n               number of bits in CRC. For E.g. CRC-8 use 8.
 * @param init            initial value stored in the @ref crcx_ctx.lfsr
 * @param fini            final value xor'ed with the @ref crcx_ctx.lfsr
 * @param poly            the polynomial used for CRC calculations
 * @param reflect_input   perform a bitwise reversal of each input byte
 * @param reflect_output  perform a bitwise reversal of the result of the CRC
 * calculation
 *
 * @return true on success, otherwise false
 */
bool crcx_init(struct crcx_ctx *ctx, uint8_t n, uintmax_t init, uintmax_t fini,
               uintmax_t poly, bool reflect_input, bool reflect_output);

/**
 * Finalize a CRC context
 *
 * This function xor's the @ref crcx_ctx.fini value with the @ref crcx_ctx.lfsr.
 *
 * It also reflects the @ref crcx_ctx.lfsr if @ref crcx_ctx.reflect_output is
 * set.
 *
 * @param ctx  ctx the CRC context to initialize
 *
 * @return the result of the CRC calculation or -1 on error
 */
uintmax_t crcx_fini(struct crcx_ctx *ctx);

/**
 * Update the CRC calculation with new @p data
 *
 * Warning: This function does not do any input validation.
 *
 * Typical ueses should be via @ref crcx.
 *
 * @param ctx   the CRC context to update
 * @param data  the data for which the CRC should be updated
 */
void crcx_update(struct crcx_ctx *ctx, const uint8_t data);

/**
 * Compute the CRC
 *
 * This function should be called after @ref crcx_init and before @ref
 * crcx_fini.
 *
 * It validates input with @ref crcx_valid and then calls @ref crcx_update
 * @p len times, once for each item in @p data.
 *
 * Typical uses cases of CRCx will use this function along with @ref crcx_init
 * and @ref crcx_fini. However, doing so is simply a shorthand. It is entirely
 * possible
 * to call @ref crcx_update directly without the use of this function.
 *
 * @param ctx   the CRC context to use
 * @param data  the data for which the CRC should be calculated
 * @param len   the length of @p data
 *
 * @return      true on success, otherwise false
 *
 * @see <a href="https://en.wikipedia.org/wiki/Cyclic_redundancy_check">Cyclic
 * Redundancy Check</a>
 */
bool crcx(struct crcx_ctx *ctx, const void *data, const size_t len);

__END_DECLS

#endif /* CRCX_H_ */
