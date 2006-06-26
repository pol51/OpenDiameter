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

#ifndef __PANA_DEFS_H__
#define __PANA_DEFS_H__

#include "diameter_parser_api.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Global type and macro definitions

// Global macro defining the current version
// of this implementation.
#define PANA_VERSION    1

// Use this macro to control (compile-time only) the
// maximum raw PANA message packet size that can be
// sent or received.
#define PANA_MAX_MESSAGE_SIZE 1024

// Use this macro to control (compile-time only) the
// minimum number of pre-allocated messages that will
// be stored in the message pool. This value can over-ridden
// via configuration entries if the value in the config
// files is greater than this minimum value
#define PANA_MIN_MESSAGE_COUNT 100

//
// MPA support
//
// #define PANA_MPA_SUPPORT 1

//
// General debuggging
//
// #define PANA_DEBUG  1

// Message structure and data definition
#if !defined(ACE_WIN32)
#define UCHAR unsigned char
#endif

// Message type definition based on draft 11
#define PANA_MTYPE_PDI         0x01
#define PANA_MTYPE_PSR         0x02
#define PANA_MTYPE_PSA         0x02
#define PANA_MTYPE_PAR         0x03
#define PANA_MTYPE_PAN         0x03
#define PANA_MTYPE_PRAR        0x04
#define PANA_MTYPE_PRAA        0x04
#define PANA_MTYPE_PBR         0x05
#define PANA_MTYPE_PBA         0x05
#define PANA_MTYPE_PPR         0x06
#define PANA_MTYPE_PPA         0x06
#define PANA_MTYPE_PTR         0x07
#define PANA_MTYPE_PTA         0x07
#define PANA_MTYPE_PER         0x08
#define PANA_MTYPE_PEA         0x08
#define PANA_MTYPE_PFER        0x09
#define PANA_MTYPE_PFEA        0x09
#define PANA_MTYPE_PUR         0x0A
#define PANA_MTYPE_PUA         0x0A

// AVP names
#define PANA_AVPNAME_ALGORITHM        "Algorithm"
#define PANA_AVPNAME_AUTH             "AUTH"
#define PANA_AVPNAME_COOKIE           "Cookie"
#define PANA_AVPNAME_EAP              "EAP-Payload"
#define PANA_AVPNAME_RESULTCODE       "Result-Code"
#define PANA_AVPNAME_NAPINFO          "NAP-Information"
#define PANA_AVPNAME_SESSIONID        "Session-Id"
#define PANA_AVPNAME_NONCE            "Nonce"
#define PANA_AVPNAME_ISPINFO          "ISP-Information"
#define PANA_AVPNAME_PROVIDERID       "Provider-Identifier"
#define PANA_AVPNAME_PROVIDERNAME     "Provider-Name"
#define PANA_AVPNAME_KEYID            "Key-Id"
#define PANA_AVPNAME_SESSIONLIFETIME  "Session-Lifetime"
#define PANA_AVPNAME_PROTECTIONCAP    "Protection-Capability"
#define PANA_AVPNAME_DEVICEID         "Device-Id"
#define PANA_AVPNAME_PPAC             "PPAC"
#define PANA_AVPNAME_TERMCAUSE        "Termination-Cause"
#define PANA_AVPNAME_NOTIFICATION     "Notification"
#define PANA_AVPNAME_DHCP             "Dhcp-Avp"
#if defined(PANA_MPA_SUPPORT)
#define PANA_AVPNAME_PACIP            "PAC-Ip-Address"
#endif

// Protection capabilities
#define PANA_PCAP_UNKNOWN               0
#define PANA_PCAP_L2                    1
#define PANA_PCAP_IPSEC                 2

// Authorization result code values
#define PANA_SUCCESS                    2001
#define PANA_AUTHENTICATION_REJECTED    4001
#define PANA_AUTHORIZATION_REJECTED     5003

// Termination cause values
#define PANA_TERMCAUSE_LOGOUT           1 // (PaC -> PAA)
#define PANA_TERMCAUSE_ADMINISTRATIVE   4 // (PAA -> Pac)
#define PANA_TERMCAUSE_SESSION_TIMEOUT  8 // (PAA -> PaC)

// Protocol Error Result Codes
#define PANA_MESSAGE_UNSUPPORTED               3001
#define PANA_UNABLE_TO_DELIVER                 3002
#define PANA_INVALID_HDR_BITS                  3008
#define PANA_INVALID_AVP_BITS                  3009
#define PANA_AVP_UNSUPPORTED                   5001
#define PANA_UNKNOWN_SESSION_ID                5002
#define PANA_INVALID_AVP_VALUE                 5004
#define PANA_MISSING_AVP                       5005
#define PANA_RESOURCES_EXCEEDED                5006
#define PANA_CONTRADICTING_AVPS                5007
#define PANA_AVP_NOT_ALLOWED                   5008
#define PANA_AVP_OCCURS_TOO_MANY_TIMES         5009
#define PANA_UNSUPPORTED_VERSION               5011
#define PANA_UNABLE_TO_COMPLY                  5012
#define PANA_INVALID_AVP_LENGTH                5014
#define PANA_INVALID_MESSAGE_LENGTH            5015
#define PANA_PROTECTION_CAPABILITY_UNSUPPORTED 5016
#define PANA_PPAC_CAPABILITY_UNSUPPORTED       5017
#define PANA_INVALID_IP_ADDRESS                5018

//
// Result codes based on RFC3588
//
//  1xxx (Informational)
//  2xxx (Success)
//  3xxx (Protocol Errors)
//  4xxx (Transient Failures)
//  5xxx (Permanent Failure)
//
#define PANA_RCODE_RANGE(x,y,z)          (((x) >= (y)) && ((x) <= (z)))
#define PANA_RCODE_INFORMATIONAL(x)      PANA_RCODE_RANGE(x, 1000, 1999) 
#define PANA_RCODE_SUCCESS(x)            PANA_RCODE_RANGE(x, 2000, 2999)
#define PANA_RCODE_PROTOCOL_ERROR(x)     PANA_RCODE_RANGE(x, 3000, 3999)
#define PANA_RCODE_TRANSIENT_FAILURE(x)  PANA_RCODE_RANGE(x, 4000, 4999)
#define PANA_RCODE_PERMANENT_FAILURE(x)  PANA_RCODE_RANGE(x, 5000, 5999)

// 
// Simple queue type
//
template<class T>
class PANA_SimpleQueue
{
   public:
       void Enqueue(T arg) {
           m_Queue.push_back(arg);
       }
       T Dequeue() {
           T arg = m_Queue.front();
           m_Queue.pop_front();
           return arg;
       }
       bool Empty() {
           return m_Queue.empty();
       }
   private:
       std::list<T> m_Queue;
};

//
// Message block guard class
//
class PANA_MsgBlockGuard
{
   public:
       PANA_MsgBlockGuard() : 
           m_Block(0) {
       }
       PANA_MsgBlockGuard(AAAMessageBlock *b, 
                          bool clone = false) :
           m_Block(0) {
           m_Block = (clone) ? Clone(b) : b;
       }
       virtual ~PANA_MsgBlockGuard() {
	   if (m_Block) {
	       m_Block->Release();
	   }
       }
       AAAMessageBlock *operator=(AAAMessageBlock *b) {
           if (m_Block) {
	       m_Block->Release();
	   }
           m_Block = b;
           return m_Block;
       }
       PANA_MsgBlockGuard &operator=(PANA_MsgBlockGuard &b) {
           (*this) = b();
           return (*this);
       }
       AAAMessageBlock *operator()() {
	   return m_Block;
       }
       AAAMessageBlock *Release() {
	   AAAMessageBlock *tmp = m_Block;
           m_Block = 0;
           return tmp;
       }
       AAAMessageBlock *Clone(AAAMessageBlock *b) {
           if (m_Block) {
	       m_Block->Release();
	   }
           if (b) {
              // deep copy only
              m_Block = AAAMessageBlock::Acquire
                     (ACE_UINT32(b->size()));
              if (m_Block) {
	          ACE_OS::memcpy(m_Block->base(),
                                 b->base(),
                                 b->size());
                  m_Block->wr_ptr(b->size());               
              }
	   }
           return m_Block;
       }

   private:
       AAAMessageBlock *m_Block;
};

//
// Flag attributes
//
template<class T>
class PANA_ScholarValue
{
   public:
       PANA_ScholarValue() :
           m_IsSet(false) {
       }
       bool IsSet() {
           return m_IsSet;
       }
       T &Get() {
           return m_Value;
       }
       void Set(T &v) {
           m_IsSet = true;
           m_Value = v;
       }
       PANA_ScholarValue<T> &operator=
           (PANA_ScholarValue<T> &v) {
           Set(v.Get());
           return *this;
       }
       T &operator=(T &v) {
           Set(v);
           return m_Value;
       }
       void Reset() {
           m_IsSet = false;
       }
   protected:
       T m_Value;
       bool m_IsSet;
};

//
// Converters
//
class PANA_AddrConverter
{
    public:
        static inline void ToAce(diameter_address_t &from,
                                 ACE_INET_Addr &to) {
           to.set_address(from.value.data(), 
                          (int)from.value.size());
        }
        static inline void ToAAAAddress(ACE_INET_Addr &from,
                                        diameter_address_t &to) {
#if defined (ACE_HAS_IPV6)
           if (from.get_type() == AF_INET6) {
               sockaddr_in6 *in = (sockaddr_in6*)from.get_addr();
               to.value.assign((char*)&in->sin6_addr.s6_addr,
                   sizeof(struct in6_addr));
               to.type = AAA_ADDR_FAMILY_IPV6;
            }
            else if (from.get_type() == AF_INET) { 
#else
            if (from.get_type() == AF_INET) { 
#endif
	       ACE_UINT32 ipAddr = from.get_ip_address();
               to.value.assign((char*)&ipAddr, sizeof(ACE_UINT32));
               to.type = AAA_ADDR_FAMILY_IPV4;
            }
        }
};

#endif /* __PANA_CORE_H__ */
