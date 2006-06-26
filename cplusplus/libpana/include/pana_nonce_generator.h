/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2004 Open Diameter Project                          */
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

#ifndef __PANA_NONCE_GENERATOR_H__
#define __PANA_NONCE_GENERATOR_H__

#include "ace/System_Time.h"
#include "pana_defs.h"

class PANA_NonceGenerator
{
    public:
       static inline void Get(diameter_octetstring_t &nonce) {
          ACE_UINT32 ltime;
          ACE_UINT32 buffer[4]; // 128-bit nonce       

          ACE_System_Time::get_local_system_time(ltime);
          ACE_OS::srand(ltime + ACE_OS::rand());
          for (int i = 0; 
               i < sizeof(buffer)/sizeof(ACE_UINT32); 
               i++) {
             buffer[i] = ACE_UINT32(ACE_OS::rand());
	  }
          nonce.assign((char*)buffer, sizeof(buffer));
       }
};

#endif // __PANA_MD5_H__
