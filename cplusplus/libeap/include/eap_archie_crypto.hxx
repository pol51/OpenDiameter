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
// $Id: eap_archie_crypto.hxx,v 1.4 2004/06/17 21:13:36 yohba Exp $

// EAP-Archie cryptographic functions.
// Written by Yoshihiro Ohba (yohba@tari.toshiba.com)

#ifndef __EAP_ARCHIE_CRYPTO_HXX__
#define __EAP_ARCHIE_CRYPTO_HXX__

#include <openssl/aes.h>
#include <string.h>

/* From draft-jwalker-eap-archie-01.txt:

3.1. AES-CBC-MAC-128 and AES-CBC-MAC-96
                                                                               
This section reviews the definition of the AES-CBC-MAC.
                                                                               
Let K denote an AES key, and let AES-Encrypt denote the AES encrypt
primitive. The AES-CBC-MAC-128 of a string S is defined using the
following steps:
                                                                               
a. Let L denote the length of S in octets. Let L' = L + p, where p
   is the unique integer 0 <= p < 16 needed so that L' is a
   multiple of 16.
b. Let n = L'/16, and partition S into substrings, such that
   S = S[1] S[2] ... S[n-1] S'[n], with S[1], S[2], ..., S[n-1]
   consisting of 16 octets each and S'[n] consisting of 16 octets
   if p is 0 and 16-p octets otherwise. Let S[n] = S'[n]0^p, where
   0^p denotes p octets of zero pad.
c. Set IV to 16 zero octets.
d. For i = 1 to n do
      IV = AES-Encrypt(K, S[i] XOR IV)
e. Return IV.
                                                                               
The EAP-Archie protocol uses a variant of AES-CBC-MAC-128 called
AES-CBC-MAC-96. This variant differs from the AES-CBC-MAC-128
described above in only step e, which it replaces by:
                                                                               
e'. Return the first 12 octets of IV.
                                                                               
Note that any padding added in step b is used to compute the MAC value
only, and is never sent as part of an EAP-Archie message.
 */

/// This is a function object that calculates AES-CBC-MAC-{96,128}.
class EapCryptoAES_CBC_MAC
{
public:
  enum MAC_LengthType {
    MAC_Length96,
    MAC_Length128
  };
  /// The AES-CBC-MAC-{96,128} implementation.
  /// \param in the string that stores data to be authenticated.
  /// \param out the string where the resulting MIC is stored.
  /// \param sharedSecret the string that stores a shared secret.
  /// \param lengthType indicates either MAC_Length96 or
  /// MAC_Length128.
  void operator()(std::string& in, std::string& out, 
		  std::string& sharedSecret, MAC_LengthType lengthType)
  {
    // Number of 16-octet blocks.
    int L = (int)in.size();
    int n = (L - 1) / 16 + 1; 

    AES_KEY key;
    AES_set_encrypt_key((const unsigned char*)sharedSecret.data(), 128, &key);

    std::string tmp(16*n, '\0');
    tmp.replace(0, in.size(), in);

    unsigned char IV[16]; 
    ACE_OS::memset(IV, 0, sizeof IV);

    for (int i=0; i<n; i++)
      {
	    const char* S = tmp.data() + 16*i;
	    // Calculate XOR.
	    for (int j=0; j<16; j++) { IV[j] ^= S[j]; }
        AES_encrypt((const unsigned char*)IV, IV, &key);
      }
    out.assign((char*)IV,  (lengthType == MAC_Length96) ? 12 : 16);
  }
};

/* From draft-jwalker-eap-archie-01.txt:

3.2. The Archie-PRF
                                                                               
This section defines the Archie-PRF function.
                                                                               
Let K denote a key, and let Length denote the number of octets
desired. The Archie-PRF of a string S is defined by the following
steps:
                                                                               
a. Output is initialized to the null string.
b. For i = 1 to (Length+15)/16 do
      Output = Output | AES-CBC-MAC-128(K, i | S | Length)
c. Return first Length octets of Output
                                                                               
In step b, the loop index i and the Length are encoded as 32-bit
big-Endian unsigned integers.
                                                                               
 */

/// This is a function object that calculates Archie PRF.
class EapCryptoArchiePRF
{
public:
  void operator()(std::string& in, std::string& out, std::string& sharedSecret,
		  ACE_UINT32 length)
  {
    out.assign("");
    std::string tmp;
    ACE_UINT32 lengthNBO = ntohl(length); // representation of length
					  // in networkbyte order.
    for (ACE_UINT32 i=1; i <= (length+15)/16; i++)
      {
	ACE_UINT32 iNBO = ntohl(i); // representation of i in network
				    // byte order.
	tmp.assign((char*)&iNBO, sizeof iNBO);
	tmp.append(in);
	tmp.append((char*)&lengthNBO, sizeof lengthNBO);
	EapCryptoAES_CBC_MAC mac;
	mac(tmp, tmp, sharedSecret, EapCryptoAES_CBC_MAC::MAC_Length128);
	out.append(tmp);
      }
    out.resize(length);
  }
};

#endif
