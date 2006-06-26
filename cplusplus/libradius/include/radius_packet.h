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

#ifndef __RADIUS_PACKET_H__
#define __RADIUS_PACKET_H__

#include "radius_attribute.h"

#define RADIUS_PKT_MIN_SIZE        20
#define RADIUS_PKT_MAX_SIZE        4096
#define RADIUS_NUM_VALID_MSG       9  // must match below

typedef enum {
   RADIUS_CODE_ACCESS_REQUEST = 1,
   RADIUS_CODE_ACCESS_ACCEPT  = 2,
   RADIUS_CODE_ACCESS_REJECT  = 3,
   RADIUS_CODE_ACCT_REQUEST   = 4,
   RADIUS_CODE_ACCT_RESPONSE  = 5,
   RADIUS_CODE_ACCESS_CHLLNG  = 11,
   RADIUS_CODE_STATUS_SERVER  = 12,
   RADIUS_CODE_STATUS_CLIENT  = 13,
   RADIUS_CODE_RESERVED       = 255
} RADIUS_CODES;

class RADIUS_PktHeader
{
   public:
       enum {
          AuthenticatorLen = 16
       };

   public:
       RADIUS_PktHeader() {
       	  Clear();
       }

       RADIUS_OCTET &Code() {
          return m_Code;
       }
       RADIUS_OCTET &Id() {
          return m_Identifier;
       }
       RADIUS_SHORT &Length() {
          return m_Length;
       }
       RADIUS_OCTET *Authenticator() {
          return m_Authenticator;
       }
       void Clear() {
          m_Code = 0;
          m_Identifier = 0;
          m_Length = 0;
          ACE_OS::memset(m_Authenticator, 0, 
                         sizeof(m_Authenticator));
       }

   protected:
       RADIUS_OCTET m_Code;
       RADIUS_OCTET m_Identifier;
       RADIUS_SHORT m_Length;
       RADIUS_OCTET m_Authenticator[AuthenticatorLen];
};

class RADIUS_Packet : public RADIUS_PktHeader
{
   public:
       RADIUS_AttributeList &Attributes() {
          return m_Attributes;
       }
       ACE_INET_Addr &SourceAddress() {
       	  return m_SrcAddress;
       }

   protected:
       ACE_INET_Addr m_SrcAddress;
       RADIUS_AttributeList m_Attributes;
};

#endif /* __RADIUS_PACKET_H__ */
