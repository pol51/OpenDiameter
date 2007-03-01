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

#ifndef __PANA_PRF_PLUS_H__
#define __PANA_PRF_PLUS_H__

#include "ace/OS.h"
#ifndef WIN32
#include <openssl/hmac.h>
#else
// TBD: need windows version
#endif

typedef enum {
   PRF_HMAC_SHA1, 
   PRF_HMAC_MD5
} PANA_TypeHash;

class PANA_PrfPlus {

   protected:
#ifndef WIN32
       EVP_MD *prf_function;           /**< Hash function used to generate keys */
#else
// TBD: need windows version
#endif
       PANA_TypeHash hash_type;

   public:
    
       ACE_UINT16 encr_size;     				/**< Size of the ecryption keys */
       ACE_UINT16 integ_size;    				/**< Size of the integrity keys */
       ACE_UINT16 prf_size;      				/**< Size of the prf keys */

       ACE_UINT16 *array_encr_size;     			/**< Size of the ecryption keys */
       ACE_UINT16 *array_integ_size;    			/**< Size of the integrity keys */
       ACE_UINT16 number_of_protocols;			/**< Number of protocols */


       PANA_PrfPlus(PANA_TypeHash hash);
       /**
        * Calculates PRF function as defined in IKEv2 draft.
        * @param key Key to be used.
        * @param key_length Key length.
        * @param sequence Sequence to calculate PRF.
        * @param sequence_length Sequence length.
        * @param result Buffer where store result.
        */
       virtual void PRF(ACE_Byte *key, 
                        ACE_UINT16 key_length, 
                        ACE_Byte *sequence, 
                        ACE_UINT16 sequence_length, 
                        ACE_Byte *result);
    
       /**
        * Calculates PRF+ function as defined in IKEv2 draft.
        * @param iter Number of iterations.
        * @param key Key to be used.
        * @param key_length Key length.
        * @param sequence Sequence to calculate PRF+.
        * @param sequence_length Sequence length.
        * @param result Buffer where store result.
        */
       virtual void PRF_plus(ACE_Byte iter, 
                             ACE_Byte *key, 
                             ACE_UINT16 key_length, 
                             ACE_Byte *sequence, 
                             ACE_UINT16 sequence_length, 
                             ACE_Byte *result);
    

        virtual ~PANA_PrfPlus();

};

#endif
