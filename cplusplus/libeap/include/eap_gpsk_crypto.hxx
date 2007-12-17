/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2007 Open Diameter Project                          */
/*                                                                        */
/* This library is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU Lesser General Public License as         */
/* published by the Free Software Foundation; either version 2.1 of the   */
/* License, or (at your option) any later version.                        */
/*                                                                        */
/* This library is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      */
/* Lesser General Public License for more details.                        */
/*                                                                        */
/* You should have received a copy of the GNU Lesser General Public       */
/* License along with this library; if not, write to the Free Software    */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307    */
/* USA.                                                                   */
/*                                                                        */
/* In addition, when you copy and redistribute some or the entire part of */
/* the source code of this software with or without modification, you     */
/* MUST include this copyright notice in each copy.                       */
/*                                                                        */
/* If you make any changes that are appeared to be useful, please send    */
/* sources that include the changed part to                               */
/* diameter-developers@lists.sourceforge.net so that we can reflect your  */
/* changes to one unified version of this software.                       */
/*                                                                        */
/* END_COPYRIGHT                                                          */

#ifndef __EAP_GPSK_CRYPTO_HXX__
#define __EAP_GPSK_CRYPTO_HXX__

#include <openssl/aes.h>
#include <string.h>


/* From RFC 4493
   AES-CMAC uses the Advanced Encryption Standard [NIST-AES] as a
   building block.  To generate a MAC, AES-CMAC takes a secret key, a
   message of variable length, and the length of the message in octets
   as inputs and returns a fixed-bit string called a MAC.

   The core of AES-CMAC is the basic CBC-MAC.  For a message, M, to be
   authenticated, the CBC-MAC is applied to M.  There are two cases of
   operation in CMAC.  Figure 2.1 illustrates the operation of CBC-MAC
   in both cases.  If the size of the input message block is equal to a
   positive multiple of the block size (namely, 128 bits), the last
   block shall be exclusive-OR'ed with K1 before processing.  Otherwise,
   the last block shall be padded with 10^i (notation is described in
   section 2.1) and exclusive-OR'ed with K2.  The result of the previous



Song, et al.                 Informational                      [Page 4]

RFC 4493                 The AES-CMAC Algorithm                June 2006


   process will be the input of the last encryption.  The output of
   AES-CMAC provides data integrity of the whole input message.

 +-----+     +-----+     +-----+     +-----+     +-----+     +---+----+
 | M_1 |     | M_2 |     | M_n |     | M_1 |     | M_2 |     |M_n|10^i|
 +-----+     +-----+     +-----+     +-----+     +-----+     +---+----+
    |           |           |   +--+    |           |           |   +--+
    |     +--->(+)    +--->(+)<-|K1|    |     +--->(+)    +--->(+)<-|K2|
    |     |     |     |     |   +--+    |     |     |     |     |   +--+
 +-----+  |  +-----+  |  +-----+     +-----+  |  +-----+  |  +-----+
 |AES_K|  |  |AES_K|  |  |AES_K|     |AES_K|  |  |AES_K|  |  |AES_K|
 +-----+  |  +-----+  |  +-----+     +-----+  |  +-----+  |  +-----+
    |     |     |     |     |           |     |     |     |     |
    +-----+     +-----+     |           +-----+     +-----+     |
                            |                                   |
                         +-----+                              +-----+
                         |  T  |                              |  T  |
                         +-----+                              +-----+

             (a) positive multiple block length         (b) otherwise

          Figure 2.1.  Illustration of the two cases of AES-CMAC

   AES_K is AES-128 with key K.
   The message M is divided into blocks M_1,...,M_n,
   where M_i is the i-th message block.
   The length of M_i is 128 bits for i = 1,...,n-1, and
   the length of the last block, M_n, is less than or equal to 128 bits.
   K1 is the subkey for the case (a), and
   K2 is the subkey for the case (b).
   K1 and K2 are generated by the subkey generation algorithm
   described in section 2.3.

2.4.  MAC Generation Algorithm

   The MAC generation algorithm, AES-CMAC(), takes three inputs, a
   secret key, a message, and the length of the message in octets.  The
   secret key, denoted by K, is just the key for AES-128.  The message
   and its length in octets are denoted by M and len, respectively.  The
   message M is denoted by the sequence of M_i, where M_i is the i-th
   message block.  That is, if M consists of n blocks, then M is written
   as

    -   M = M_1 || M_2 || ... || M_{n-1} || M_n

   The length of M_i is 128 bits for i = 1,...,n-1, and the length of
   the last block M_n is less than or equal to 128 bits.

   The output of the MAC generation algorithm is a 128-bit string,
   called a MAC, which is used to validate the input message.  The MAC
   is denoted by T, and we write T := AES-CMAC(K,M,len).  Validating the
   MAC provides assurance of the integrity and authenticity of the
   message from the source.

   It is possible to truncate the MAC.  According to [NIST-CMAC], at
   least a 64-bit MAC should be used as protection against guessing
   attacks.  The result of truncation should be taken in most
   significant bits first order.

   The block length of AES-128 is 128 bits (16 octets).  There is a
   special treatment if the length of the message is not a positive
   multiple of the block length.  The special treatment is to pad M with
   the bit-string 10^i to adjust the length of the last block up to the
   block length.

   For an input string x of r-octets, where 0 <= r < 16, the padding
   function, padding(x), is defined as follows:

   -   padding(x) = x || 10^i      where i is 128-8*r-1

   That is, padding(x) is the concatenation of x and a single '1',
   followed by the minimum number of '0's, so that the total length is
   equal to 128 bits.

   Figure 2.3 describes the MAC generation algorithm.






Song, et al.                 Informational                      [Page 7]

RFC 4493                 The AES-CMAC Algorithm                June 2006


   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   +                   Algorithm AES-CMAC                              +
   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   +                                                                   +
   +   Input    : K    ( 128-bit key )                                 +
   +            : M    ( message to be authenticated )                 +
   +            : len  ( length of the message in octets )             +
   +   Output   : T    ( message authentication code )                 +
   +                                                                   +
   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   +   Constants: const_Zero is 0x00000000000000000000000000000000     +
   +              const_Bsize is 16                                    +
   +                                                                   +
   +   Variables: K1, K2 for 128-bit subkeys                           +
   +              M_i is the i-th block (i=1..ceil(len/const_Bsize))   +
   +              M_last is the last block xor-ed with K1 or K2        +
   +              n      for number of blocks to be processed          +
   +              r      for number of octets of last block            +
   +              flag   for denoting if last block is complete or not +
   +                                                                   +
   +   Step 1.  (K1,K2) := Generate_Subkey(K);                         +
   +   Step 2.  n := ceil(len/const_Bsize);                            +
   +   Step 3.  if n = 0                                               +
   +            then                                                   +
   +                 n := 1;                                           +
   +                 flag := false;                                    +
   +            else                                                   +
   +                 if len mod const_Bsize is 0                       +
   +                 then flag := true;                                +
   +                 else flag := false;                               +
   +                                                                   +
   +   Step 4.  if flag is true                                        +
   +            then M_last := M_n XOR K1;                             +
   +            else M_last := padding(M_n) XOR K2;                    +
   +   Step 5.  X := const_Zero;                                       +
   +   Step 6.  for i := 1 to n-1 do                                   +
   +                begin                                              +
   +                  Y := X XOR M_i;                                  +
   +                  X := AES-128(K,Y);                               +
   +                end                                                +
   +            Y := M_last XOR X;                                     +
   +            T := AES-128(K,Y);                                     +
   +   Step 7.  return T;                                              +
   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                      Figure 2.3.  Algorithm AES-CMAC





Song, et al.                 Informational                      [Page 8]

RFC 4493                 The AES-CMAC Algorithm                June 2006


   In step 1, subkeys K1 and K2 are derived from K through the subkey
   generation algorithm.

   In step 2, the number of blocks, n, is calculated.  The number of
   blocks is the smallest integer value greater than or equal to the
   quotient determined by dividing the length parameter by the block
   length, 16 octets.

   In step 3, the length of the input message is checked.  If the input
   length is 0 (null), the number of blocks to be processed shall be 1,
   and the flag shall be marked as not-complete-block (false).
   Otherwise, if the last block length is 128 bits, the flag is marked
   as complete-block (true); else mark the flag as not-complete-block
   (false).

   In step 4, M_last is calculated by exclusive-OR'ing M_n and one of
   the previously calculated subkeys.  If the last block is a complete
   block (true), then M_last is the exclusive-OR of M_n and K1.
   Otherwise, M_last is the exclusive-OR of padding(M_n) and K2.

   In step 5, the variable X is initialized.

   In step 6, the basic CBC-MAC is applied to M_1,...,M_{n-1},M_last.

   In step 7, the 128-bit MAC, T := AES-CMAC(K,M,len), is returned.

   If necessary, the MAC is truncated before it is returned.

*/
class EapCryptoAES_CMAC_128
{
  /// The AES-CMAC-128 implementation.
  /// \param K the string that stores key
  /// \param M the string that stores the message
  /// \param len the length of the message M
  /// \param T the message authentication code
  void operator()(std::string& K, std::string& M, size_t len, std::string &T)
  {
    const ACE_Byte const_Rb[]   = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x8,0x7 };

    unsigned char X[16],Y[16], M_last[16], padded[16];
    unsigned char K1[16], K2[16], mac[16];

    int n, i, flag;

    generateSubkey(K.data(), K1, K2);

    n = (len + 15) / 16; /* n is number of rounds */

    if ( n == 0 ) {
        n = 1;
        flag = 0;
    } else {
        if ( (length % 16) == 0 ) { /* last block is a complete block */
            flag = 1;
        } else { /* last block is not complete block */
            flag = 0;
        }
    }

    if ( flag ) { /* last block is complete block */
        xor_128(&M.data()[16*(n-1)], K1, M_last);
    } else {
        padding(&M.data()[16*(n-1)], padded, length % 16);
        xor_128(padded, K2, M_last);
    }

    for ( i = 0; i < 16; i++ ) {
        X[i] = 0;
    }

    AES_KEY key;
    AES_set_encrypt_key(K.data(), 128, &key);

    for ( i = 0; i < n - 1; i++ ) {
        xor_128(X, &M.data()[16*i], Y); /* Y := Mi (+) X  */
        AES_encrypt(Y, X, &key); /* X := AES-128(KEY, Y); */
    }

    xor_128(X, M_last, Y);
    AES_encrypt(Y, X, &key); /* X := AES-128(KEY, Y); */

    for ( i=0; i<16; i++ ) {
        mac[i] = X[i];
    }
    T.assign(mac, sizeof(mac));
  }

protected:
/*
2.3.  Subkey Generation Algorithm

   The subkey generation algorithm, Generate_Subkey(), takes a secret
   key, K, which is just the key for AES-128.

   The outputs of the subkey generation algorithm are two subkeys, K1
   and K2.  We write (K1,K2) := Generate_Subkey(K).

   Subkeys K1 and K2 are used in both MAC generation and MAC
   verification algorithms.  K1 is used for the case where the length of
   the last block is equal to the block length.  K2 is used for the case
   where the length of the last block is less than the block length.

   Figure 2.2 specifies the subkey generation algorithm.

   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   +                    Algorithm Generate_Subkey                      +
   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   +                                                                   +
   +   Input    : K (128-bit key)                                      +
   +   Output   : K1 (128-bit first subkey)                            +
   +              K2 (128-bit second subkey)                           +
   +-------------------------------------------------------------------+
   +                                                                   +
   +   Constants: const_Zero is 0x00000000000000000000000000000000     +
   +              const_Rb   is 0x00000000000000000000000000000087     +
   +   Variables: L          for output of AES-128 applied to 0^128    +
   +                                                                   +
   +   Step 1.  L := AES-128(K, const_Zero);                           +
   +   Step 2.  if MSB(L) is equal to 0                                +
   +            then    K1 := L << 1;                                  +
   +            else    K1 := (L << 1) XOR const_Rb;                   +
   +   Step 3.  if MSB(K1) is equal to 0                               +
   +            then    K2 := K1 << 1;                                 +
   +            else    K2 := (K1 << 1) XOR const_Rb;                  +
   +   Step 4.  return K1, K2;                                         +
   +                                                                   +
   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                Figure 2.2.  Algorithm Generate_Subkey

   In step 1, AES-128 with key K is applied to an all-zero input block.

   In step 2, K1 is derived through the following operation:

   If the most significant bit of L is equal to 0, K1 is the left-shift
   of L by 1 bit.

   Otherwise, K1 is the exclusive-OR of const_Rb and the left-shift of L
   by 1 bit.

   In step 3, K2 is derived through the following operation:

   If the most significant bit of K1 is equal to 0, K2 is the left-shift
   of K1 by 1 bit.

   Otherwise, K2 is the exclusive-OR of const_Rb and the left-shift of
   K1 by 1 bit.

   In step 4, (K1,K2) := Generate_Subkey(K) is returned.

   The mathematical meaning of the procedures in steps 2 and 3,
   including const_Rb, can be found in [OMAC1a].
*/
  void generateSubkeys(unsigned char *K, unsigned char *K1, unsigned char *K2)
  {
    const unsigned char const_Zero[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    const unsigned char const_Rb[]   = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x8,0x7 };
    unsigned char L[16], tmp[16];

    AES_KEY key;
    AES_set_encrypt_key(K, 128, &key);
    AES_encrypt(const_Zero, L, &key);

    if ((L[0] & 0x80) == 0) {
      leftShift(L, K1);
    }
    else {
      leftShift(L, tmp);
      xor_128(tmp, const_Rb, K1);
    }

    if ((K1[0] & 0x80) == 0) {
      leftShift(K1, K2);
    }
    else {
      leftShift(K1, tmp);
      xor_128(tmp, const_Rb, K2);
    }
  }

  void leftShift(char *input, char *output)
  {
    int i;
    unsigned char overflow = 0;
    for ( i=15; i>=0; i-- ) {
        output[i] = input[i] << 1;
        output[i] |= overflow;
        overflow = (input[i] & 0x80)?1:0;
    }
  }

  void xor_128(unsigned char *a, unsigned char *b, unsigned char *out)
  {
    int i;
    for (i=0;i<16; i++)
    {
        out[i] = a[i] ^ b[i];
    }
  }

  void padding ( unsigned char *lastb, unsigned char *pad, int length )
  {
    int j;

    /* original last block */
    for ( j = 0; j < 16; j++ ) {
       if ( j < length ) {
           pad[j] = lastb[j];
       } else if ( j == length ) {
           pad[j] = 0x80;
       } else {
           pad[j] = 0x00;
       }
    }
  }
};

/*
6. Generalized Key Derivation Function (GKDF)


   Each ciphersuite needs to specify a key derivation function.  The
   ciphersuites defined in this document make use of the Generalized Key
   Derivation Function (GKDF) that utilizes the MAC function defined in
   the ciphersuite.  Future ciphersuites can use any other formally
   specified KDF that takes as arguments a key and a seed value, and
   produces at least 128+2*KS octets of output.

   GKDF has the following structure:

   GKDF-X(Y, Z)

   X  length, in octets, of the desired output
   Y  secret key
   Z  inputString


   GKDF-X (Y, Z)
   {
     n = ceiling integer of ( X / KS );
        // determine number of output blocks

     M_0 = "";
     result = "";

     for i = 1 to n {
       M_i = MAC_Y (i || Z);
       result = result || M_i;
     }

     return truncate(result, X)
   }

   Note that the variable 'i' in M_i is represented as a 2-octet value
   in network byte order.

*/
class EapCryptoAES_CMAC_128_GKDF
{
  /// The AES-CMAC-128 implementation.
  /// \param Y the string that stores the secret
  /// \param Z the string that stores the input
  /// \param T the output
  /// \param X desired output lenght
  void operator()(std::string& Y, std::string& Z, std::string &output, int X)
  {
     unsigned char mi[2];
     std::string M_i, result;

     EapCryptoAES_CMAC_128 cmac;
     ACE_UINT_16 i, n = (X + 15) / 16; /* n is number of rounds , key size and output block are equal*/

     for (i = 1; i <= n; i ++) {

        *(ACE_UINT16*)mi = ACE_HTONS(i);
        std::string tmp(mi);
        tmp.append(Z);
        cmac(Y, tmp, tmp.size(), M_i);

        result.append(M_i);
     }

     output.assign(M_i.data(), X);
  }
}

#endif
