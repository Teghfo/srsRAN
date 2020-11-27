/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2020 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

/*!
 * \file ldpc_avx2_consts.h
 * \brief Declaration of constants and masks for the AVX2-based implementation
 *   of the LDPC encoder and decoder.
 *
 * \author David Gregoratti
 * \date 2020
 *
 * \copyright Software Radio Systems Limited
 *
 */

#ifndef LDPC_AVX2_CONSTS_H
#define LDPC_AVX2_CONSTS_H

#include <immintrin.h>

#include "../utils_avx2.h"

/*!
 * \brief Packed 8-bit zeros.
 */
static const __m256i zero_epi8 = {0, 0, 0, 0};

/*!
 * \brief Packed 8-bit ones.
 */
static const __m256i one_epi8 = {0x0101010101010101LL,
                                 0x0101010101010101LL,
                                 0x0101010101010101LL,
                                 0x0101010101010101LL};

/*!
 * \brief Packed 8-bit 127 (that is \f$2^7 - 1\f$).
 */
static const __m256i infty8_epi8 = {0x7F7F7F7F7F7F7F7FLL,
                                    0x7F7F7F7F7F7F7F7FLL,
                                    0x7F7F7F7F7F7F7F7FLL,
                                    0x7F7F7F7F7F7F7F7FLL};
/*!
 * \brief Packed 8-bit --127 (that is \f$-2^7 + 1\f$).
 */
static const __m256i neg_infty8_epi8 = {0x8181818181818181LL,  // NOLINT
                                        0x8181818181818181LL,  // NOLINT
                                        0x8181818181818181LL,  // NOLINT
                                        0x8181818181818181LL}; // NOLINT

/*!
 * \brief Packed 8-bit 63 (that is \f$2^6 - 1\f$).
 */
static const __m256i infty7_epi8 = {0x3F3F3F3F3F3F3F3FLL,
                                    0x3F3F3F3F3F3F3F3FLL,
                                    0x3F3F3F3F3F3F3F3FLL,
                                    0x3F3F3F3F3F3F3F3FLL};
/*!
 * \brief Packed 8-bit --63 (that is \f$-2^6 + 1\f$).
 */
static const __m256i neg_infty7_epi8 = {0xC1C1C1C1C1C1C1C1LL,  // NOLINT
                                        0xC1C1C1C1C1C1C1C1LL,  // NOLINT
                                        0xC1C1C1C1C1C1C1C1LL,  // NOLINT
                                        0xC1C1C1C1C1C1C1C1LL}; // NOLINT

/*!
 * \brief Identifies even-indexed 8-bit packets.
 */
static const __m256i mask_even_epi8 = {0x00FF00FF00FF00FF,
                                       0x00FF00FF00FF00FF,
                                       0x00FF00FF00FF00FF,
                                       0x00FF00FF00FF00FF}; // NOLINT

/*!
 * \brief Mask needed for node rotation: mask_least_epi8[i] marks the bits
 * corresponding to the \b i least significant chars.
 */
static const __m256i mask_least_epi8[SRSLTE_AVX2_B_SIZE + 1] = {
    {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0x00000000000000FF, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0x000000000000FFFF, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0x0000000000FFFFFF, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0x00000000FFFFFFFF, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0x000000FFFFFFFFFF, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0x0000FFFFFFFFFFFF, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0x00FFFFFFFFFFFFFF, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0x00000000000000FF, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0x000000000000FFFF, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0x0000000000FFFFFF, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0x000000FFFFFFFFFF, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0x0000FFFFFFFFFFFF, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0x00FFFFFFFFFFFFFF, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0000000000000000, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x00000000000000FF, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x000000000000FFFF, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0000000000FFFFFF, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x000000FFFFFFFFFF, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0000FFFFFFFFFFFF, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x00FFFFFFFFFFFFFF, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0000000000000000},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x00000000000000FF},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x000000000000FFFF},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0000000000FFFFFF},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x000000FFFFFFFFFF},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0000FFFFFFFFFFFF},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x00FFFFFFFFFFFFFF},  // NOLINT
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF}}; // NOLINT

/*!
 * \brief Mask needed for node rotation: mask_most_epi8[i] marks the bits
 * corresponding to the SRSLTE_AVX2_B_SIZE - \b i most significant chars.
 */
static const __m256i mask_most_epi8[SRSLTE_AVX2_B_SIZE + 1] = {
    {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0xFFFFFFFFFFFFFF00, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0xFFFFFFFFFFFF0000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0xFFFFFFFFFF000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0xFFFFFFFF00000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0xFFFFFF0000000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0xFFFF000000000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0xFF00000000000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0xFFFFFFFFFFFFFF00, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0xFFFFFFFFFFFF0000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0xFFFFFFFFFF000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0xFFFFFFFF00000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0xFFFFFF0000000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0xFFFF000000000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0xFF00000000000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0xFFFFFFFFFFFFFF00, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0xFFFFFFFFFFFF0000, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0xFFFFFFFFFF000000, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0xFFFFFFFF00000000, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0xFFFFFF0000000000, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0xFFFF000000000000, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0xFF00000000000000, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0xFFFFFFFFFFFFFF00},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0xFFFFFFFFFFFF0000},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0xFFFFFFFFFF000000},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0xFFFFFFFF00000000},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0xFFFFFF0000000000},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0xFFFF000000000000},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0xFF00000000000000},  // NOLINT
    {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000}}; // NOLINT

#endif // LDPC_AVX2_CONSTS_H