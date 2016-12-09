/* ===================================================================== */
/* This file is part of Daredevil                                        */
/* Daredevil is a side-channel analysis tool                             */
/* Copyright (C) 2016                                                    */
/* Original author:   Paul Bottinelli <paulbottinelli@hotmail.com>       */
/* Contributors:      Joppe Bos <joppe_bos@hotmail.com>                  */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* any later version.                                                    */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/* ===================================================================== */
#include "utils.h"
#include "aes.h"

/* Compute the number of bits set (Hamming weight or poulation count) in a
 * 16 bit integer. See the Bit Twiddling Hacks page:
 * http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
 */
uint8_t HW(uint16_t v)
{
  uint8_t c;                      // c accumulates the total bits set in v
  for (c = 0; v; c++) v &= v - 1; // clear the least significant bit set
  return c;
}

unsigned char inv_sbox[256] = 
 {
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
 };

/* pt, ct are 16 bytes, key is subbyte guess, bnum is 0-16
 *
 */
static uint16_t leakage_last_round(uint8_t * pt, uint8_t * ct, uint8_t key, uint16_t * sbox, int bnum)
{
    // leakage for last round
    // e6 19 d4 c4 7d ad 82 9d c0 a6 13 ed 36 6d 3a 3b
    uint8_t INVSHIFT_undo[16] = {0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11};

    uint8_t state10 = ct[INVSHIFT_undo[bnum]];
    uint8_t state9 = inv_sbox[ct[bnum] ^ key];

    return (state10 ^ state9);
}

// fd440e270817906ba386ab195a1a5394
static uint16_t leakage_first_round(uint8_t * pt, uint8_t * ct, uint8_t key, uint16_t * sbox, int bnum)
{
    return sbox[ pt[bnum] ^ key ];
}

#define LEAKAGE_FUNC(a,b,c,d,e) leakage_first_round(a,b,c,d,e)

static int warned = 0;

/* Given the messages (m), use the bytenum-th byte to construct
 * the guesses for round R with the specified sbox.
 */
  template <class TypeGuess>
int construct_guess_AES (TypeGuess ***guess, Matrix *m, Matrix * ct, uint32_t n_m, uint32_t bytenum, uint32_t R, uint16_t * sbox, uint32_t n_keys, int8_t bit) {
  TypeGuess **mem = NULL;
  TypeGuess **memct = NULL;
  uint32_t i, j, nrows = 0, nrows_ct = 0;

  if (R != 0) {
    fprintf (stderr, "[ERROR]: construct_guess_AES: Currently only round 0 is supported.\n");
    return -1;
  }

  for (i=0; i < n_m; i++) {
    if (m[i].n_columns <= bytenum) {
      fprintf (stderr, "[ERROR]: construct_guess_AES: m ncolumns (%d) <= bytenum (%d).\n", m[i].n_columns, bytenum);
      return -1;
    }
    nrows += m[i].n_rows;
  }

  if (ct == NULL || ct[0].n_columns <= bytenum)
  {
      if (!warned)
      {
          fprintf (stderr, "[WARNING]: construct_guess_AES: No ciphertexts provided.\n", ct[0].n_columns, bytenum);
          warned = 1;
      }
  }
  else
  {
      if (import_matrices(&memct, ct, 1, 0) < 0) {
          fprintf (stderr, "[ERROR]: Importing matrix.\n");
          return -1;
      }
  }

  if (import_matrices(&mem, m, n_m, 0) < 0) {
    fprintf (stderr, "[ERROR]: Importing matrix.\n");
    return -1;
  }


  if (*guess == NULL) {
    if (allocate_matrix<TypeGuess> (guess, n_keys, nrows) < 0) {
      fprintf (stderr, "[ERROR]: Allocating memory for guesses.\n");
      free_matrix (&mem, nrows);
      return -1;
    }
  }


  for (i=0; i < nrows; i++) {
      int k;
      //printf("row %d: ",i);
      for (k=0; k < 16; k++)
      {
          //printf("%02hhx ",(uint8_t*)memct[i][k]);
      }
      //printf("\n");
    for (j=0; j < n_keys; j++) {
        if (bit == -1) { /* No individual bits. */
          (*guess)[j][i] = HW((TypeGuess) ((LEAKAGE_FUNC((uint8_t*)mem[i], (uint8_t*)memct[i], j, sbox, bytenum) >> bit)&1));
        } else if (bit >= 0 && bit < 8) {
          (*guess)[j][i] = (TypeGuess) ((LEAKAGE_FUNC((uint8_t*)mem[i], (uint8_t*)memct[i], j, sbox, bytenum) >> bit)&1);
        }
    }
  }
  free_matrix (&mem, nrows);
  return 0;
}


template int construct_guess_AES (uint8_t ***guess, Matrix *m, Matrix *ct, uint32_t n_m, uint32_t bytenum, uint32_t R, uint16_t * sbox, uint32_t n_keys, int8_t bit);
template int construct_guess_AES ( int8_t ***guess, Matrix *m, Matrix *ct, uint32_t n_m, uint32_t bytenum, uint32_t R, uint16_t * sbox, uint32_t n_keys, int8_t bit);

