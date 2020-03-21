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
 * CRCx - yet another CRC Library
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
 * Generate the CRC Lookup Table (LUT)
 *
 * @param ctx the CRC context
 *
 * @see <a
 * href="https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#Generating_the_tables">Computation
 * of cyclic redundancy checks: Generating the tables</a>
 */
void crcx_generate_table(struct crcx_ctx *ctx);

/**
 * Initialize a CRC context where most of the parameters have already been set.
 *
 * This function initializes the @ref crcx_ctx.lfsr with the contents of @ref
 * crcx_ctx.init and also generates the CRC lookup table.
 *
 * @param ctx the CRC context to initialize
 *
 * @return true on success, otherwise false
 */
bool crcx_init(struct crcx_ctx *ctx);

/**
 * Initialize a CRC context with the given parameters.
 *
 * @param ctx             ctx the CRC context to initialize
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
bool crcx_init_args(struct crcx_ctx *ctx, uint8_t n, uintmax_t init,
                    uintmax_t fini, uintmax_t poly, bool reflect_input,
                    bool reflect_output);

/**
 * Finalize a CRC context
 *
 * This function xor's the @ref crcx_ctx.fini value with the @ref crcx_ctx.lfsr.
 *
 * It also reflects the @ref crcx_ctx.lfsr if @ref crcx_ctx.reflect_output is
 * set.
 *
 * @param ctx  ctx the CRC context to initialize
 */
void crcx_fini(struct crcx_ctx *ctx);

/**
 * Update a CRC context
 *
 * This function iterates the CRC computation once, overwriting the @ref
 * crcx_ctx.lfsr.
 *
 * @param ctx   ctx the CRC context to update
 * @param data  a single byte of new data
 */
void crcx_update(struct crcx_ctx *ctx, const uint8_t data);

/**
 * Compute the CRC
 *
 * @param ctx   ctx the CRC context to use
 * @param data  the data for which the CRC should be calculated
 * @param len   the length of @p data
 *
 * @return      the CRC for the provided @p data
 * @return      -1 in case a parameter is invalid
 *
 * @see <a href="https://en.wikipedia.org/wiki/Cyclic_redundancy_check">Cyclic
 * Redundancy Check</a>
 */
uintmax_t crcx(struct crcx_ctx *ctx, const void *data, const size_t len);

__END_DECLS

#endif /* CRCX_H_ */
