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

#ifndef __PANA_DEFS_H__
#define __PANA_DEFS_H__

#include "aaa_parser_api.h"
#include "ace/SOCK_Dgram.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Global type and macro definitions

// Global macro defining the current version
// of this implementation.
#define PANA_VERSION     0x1
#define PANA_FLAG_SET    0x1
#define PANA_FLAG_CLR    0x0

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

// Message structure and data definition
#if !defined(ACE_WIN32)
#define UCHAR unsigned char
#endif

// Message type definition based on draft 13
#define PANA_MTYPE_PCI         0x01
#define PANA_MTYPE_PSR         0x02
#define PANA_MTYPE_PSA         0x02
#define PANA_MTYPE_PAR         0x03
#define PANA_MTYPE_PAN         0x03
#define PANA_MTYPE_PRR         0x04
#define PANA_MTYPE_PRA         0x04
#define PANA_MTYPE_PBR         0x05
#define PANA_MTYPE_PBA         0x05
#define PANA_MTYPE_PPR         0x06
#define PANA_MTYPE_PPA         0x06
#define PANA_MTYPE_PTR         0x07
#define PANA_MTYPE_PTA         0x07
#define PANA_MTYPE_PER         0x08
#define PANA_MTYPE_PEA         0x08
#define PANA_MTYPE_PUR         0x09
#define PANA_MTYPE_PUA         0x09

// AVP names
#define PANA_AVPNAME_ALGORITHM        "Algorithm"
#define PANA_AVPNAME_AUTH             "AUTH"
#define PANA_AVPNAME_EAP              "EAP-Payload"
#define PANA_AVPNAME_FAILEDAVP        "Failed-AVP"
#define PANA_AVPNAME_FAILEDMSGHDR     "Failed-Message-Header"
#define PANA_AVPNAME_KEYID            "Key-Id"
#define PANA_AVPNAME_NONCE            "Nonce"
#define PANA_AVPNAME_RESULTCODE       "Result-Code"
#define PANA_AVPNAME_SESSIONLIFETIME  "Session-Lifetime"
#define PANA_AVPNAME_TERMCAUSE        "Termination-Cause"

// Authorization result code values
#define PANA_RCODE_SUCCESS                    0x00
#define PANA_RCODE_AUTHENTICATION_REJECTED    0x01
#define PANA_RCODE_AUTHORIZATION_REJECTED     0x02

// Termination cause values
#define PANA_TERMCAUSE_LOGOUT           1 // (PaC -> PAA)
#define PANA_TERMCAUSE_ADMINISTRATIVE   4 // (PAA -> Pac)
#define PANA_TERMCAUSE_SESSION_TIMEOUT  8 // (PAA -> PaC)

// Protocol Error Result Codes
// These codes are used with PANA-Error-Request messages.  Unless stated
// otherwise, they can be generated by both the PaC and the PAA.
#define PANA_ERROR_MESSAGE_UNSUPPORTED        1001
#define PANA_ERROR_UNABLE_TO_DELIVER          1002
#define PANA_ERROR_INVALID_HDR_BITS           1003
#define PANA_ERROR_INVALID_AVP_FLAGS          1004
#define PANA_ERROR_AVP_UNSUPPORTED            1005
#define PANA_ERROR_INVALID_AVP_DATA           1006
#define PANA_ERROR_MISSING_AVP                1007
#define PANA_ERROR_RESOURCES_EXCEEDED         1008
#define PANA_ERROR_CONTRADICTING_AVPS         1009
#define PANA_ERROR_AVP_NOT_ALLOWED            1010
#define PANA_ERROR_AVP_OCCURS_TOO_MANY_TIMES  1011
#define PANA_ERROR_UNSUPPORTED_VERSION        1012
#define PANA_ERROR_UNABLE_TO_COMPLY           1013
#define PANA_ERROR_INVALID_AVP_LENGTH         1014
#define PANA_ERROR_INVALID_MESSAGE_LENGTH     1015

//
// ==================================================
// The following definitions are for PANA specific
// parsing support. Some basic data types have been
// derived from the generic aaa_parser_api
// ==================================================

//
// PANAAvpCode provides a way of referring to the code number of an AVP.
// It is used as a parameter to the dictionary functions, and a field in
// the AVP struct.
//
typedef ACE_UINT16        PANA_AvpCode;

//
// PANAVendorId provides a way of referring to the vendor identification
// code. It is used when ing callbacks, among others.
//
typedef ACE_UINT32        PANA_VendorId;

//
// PANAAvpFlag provides a way of referring to the AVP flags carried
// in the AVP header. It indicates whether an AVP is vendor or mandatory.
//
typedef ACE_UINT16        PANA_AvpFlag;

//
// Container for IP address and port numbers
//
typedef std::list<ACE_INET_Addr>  PANA_AddressList;

//
//==================================================
// Pre-defined enumration
//==================================================
//

//
// The AVP flags defines the flags set in the AVP header.
// They correspond directly to the avp flags defined in the
// pana-draft-12 specification [1]:
//
typedef enum {
    PANA_AVP_FLAG_NONE                 =   0x0,
    PANA_AVP_FLAG_MANDATORY            =   0x4000,
    PANA_AVP_FLAG_VENDOR_SPECIFIC      =   0x8000,
    PANA_AVP_FLAG_RESERVED             =   0x0000,
} PANA_AvpFlagEnum;

//
//==================================================
// The following definitions are for diameter specific
// data type definitions for storing parsed data.
//==================================================
//

//
// Data type definitions for AAA Parser
//
typedef ACE_INT32                  pana_integer32_t;

typedef ACE_UINT64                 pana_integer64_t;

typedef ACE_UINT32                 pana_unsigned32_t;

typedef ACE_UINT64                 pana_unsigned64_t;

typedef pana_unsigned32_t          pana_enumerated_t;

typedef pana_unsigned32_t          pana_time_t;

typedef std::string                pana_octetstring_t;

typedef pana_octetstring_t         pana_utf8string_t;

typedef class AAAAvpContainerList  pana_grouped_t;

//
// avp_t is a special type used only in this library
// for constructing a raw AVP.  When using this type, specify
// "AVP" as the avp_container type.
// The string contains the entire AVP including AVP header.
//
typedef pana_octetstring_t         pana_avp_t;

typedef struct
{
    public:
        ACE_UINT16               type;
        pana_octetstring_t   value;
} pana_address_t;

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
        static inline void ToAce(pana_address_t &from,
                                 ACE_INET_Addr &to) {
           to.set_address(from.value.data(), 
                          (int)from.value.size());
        }
        static inline void ToAAAAddress(ACE_INET_Addr &from,
                                        pana_address_t &to) {
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

//
// Transport considerations
//
typedef ACE_SOCK_Dgram   PANA_Socket;

#endif /* __PANA_CORE_H__ */
