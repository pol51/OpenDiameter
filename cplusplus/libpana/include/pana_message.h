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

#ifndef __PANA_MESSAGES_H__
#define __PANA_MESSAGES_H__

#include <list>
#include "ace/Basic_Types.h"
#include "ace/Message_Block.h"
#include "pana_defs.h"
#include "pana_exports.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define PANA_DICT_PROTOCOL_ID  1

/*!
   6.3.  AVP Header

      Each AVP of type OctetString MUST be padded to align on a 32-bit
      boundary, while other AVP types align naturally.  A number of
      zero-valued bytes are added to the end of the AVP Data field till a
      word boundary is reached.  The length of the padding is not reflected
      in the AVP Length field [RFC3588].

      The fields in the AVP header are sent in network byte order.  The
      format of the header is:

       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           AVP Code            |           AVP Flags           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |          AVP Length           |            Reserved           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                         Vendor-Id (opt)                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |    Data ...
      +-+-+-+-+-+-+-+-+

      AVP Code

         The AVP Code, together with the optional Vendor ID field,
         identifies attribute that follows.  If the V-bit is not set, the
         Vendor ID is not present and the AVP Code refers to an IETF
         attribute.

      AVP Flags

         The AVP Flags field is two octets.  The following bits are
         assigned:

       0                   1
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |V M r r r r r r r r r r r r r r|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

         V(endor)

            The 'V' bit, known as the Vendor-Specific bit, indicates
            whether the optional Vendor-Id field is present in the AVP
            header.  When set the AVP Code belongs to the specific vendor
            code address space.

         M(andatory)

            The 'M' Bit, known as the Mandatory bit, indicates whether
            support of the AVP is required.  If an AVP with the 'M' bit set
            is received by the PaC or PAA and either the AVP or its value
            is unrecognized, the message MUST be rejected and the receiver
            MUST send a PANA-Error-Request message.  If the AVP was
            unrecognized the PANA-Error-Request message result code MUST be
            PANA_AVP_UNSUPPORTED.  If the AVP value was unrecognized the
            PANA-Error-Request message result code MUST be
            PANA_INVALID_AVP_DATA.  In either case the PANA-Error-Request
            message MUST carry a Failed-AVP AVP containing the offending
            mandatory AVP.  AVPs with the 'M' bit cleared are informational
            only and a receiver that receives a message with such an AVP
            that is not recognized, or whose value is not recognized, MAY
            simply ignore the AVP.

         r(eserved)

            These flag bits are reserved for future use, and MUST be set to
            zero, and ignored by the receiver.

      AVP Length

         The AVP Length field is two octets, and indicates the number of
         octets in this AVP including the AVP Code, AVP Length, AVP Flags,
         and the AVP data.

      Reserved

         This two-octet field is reserved for future use, and MUST be set
         to zero, and ignored by the receiver.

      Vendor-Id

         The Vendor-Id field is present if the 'V' bit is set in the AVP
         Flags field.  The optional four-octet Vendor-Id field contains the
         IANA assigned "SMI Network Management Private Enterprise Codes"
         [ianaweb] value, encoded in network byte order.  Any vendor
         wishing to implement a vendor-specific PANA AVP MUST use their own
         Vendor-Id along with their privately managed AVP address space,
         guaranteeing that they will not collide with any other vendor's
         vendor-specific AVP(s), nor with future IETF applications.

      Data

         The Data field is zero or more octets and contains information
         specific to the Attribute.  The format and length of the Data
         field is determined by the AVP Code and AVP Length fields.

      Unless otherwise noted, AVPs defined in this document will have the
      following default AVP Flags field settings: The 'M' bit MUST be set.
      The 'V' bit MUST NOT be set.
 */
class PANA_AvpHeader
{
    public:
        typedef struct {
            ACE_UINT16    vendor;        // Vendor flag
            ACE_UINT16    mandatory;     // Mandatory flag
            ACE_UINT16    reserved;      // reserved
        } Flags;

    public:
        PANA_AvpHeader() :
            m_Code(0),
            m_Length(0),
            m_Vendor(0),
            m_pValue(0) {
            memset(&m_Flags, 0, sizeof(PANA_AvpHeader::Flags));
        }

    public:
        ACE_UINT16                 m_Code;       // AVP code
        PANA_AvpHeader::Flags      m_Flags;      // AVP flags
        ACE_UINT16                 m_Length;     // AVP length
        ACE_UINT32                 m_Vendor;     // Vendor code
        char*                      m_pValue;     // Value
        AAAAvpParseType            m_ParseType;  // Positional parse type
}; 

/*
   6.2.  PANA Message Header

      A summary of the PANA message header format is shown below.  The
      fields are transmitted in network byte order.


       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |    Version    |   Reserved    |        Message Length         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |             Flags             |         Message Type          |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                      Session Identifier                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                        Sequence Number                        |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  AVPs ...
      +-+-+-+-+-+-+-+-+-+-+-+-+-

      Version

         This Version field MUST be set to 1 to indicate PANA Version 1.

      Reserved

         This 8-bit field is reserved for future use, and MUST be set to
         zero, and ignored by the receiver.

      Message Length

         The Message Length field is two octets and indicates the length of
         the PANA message including the header fields.

      Flags

         The Flags field is two octets.  The following bits are assigned:

       0                   1
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |R r r r r r r r r r r r r r r r|
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

         R(equest)

            If set, the message is a request.  If cleared, the message is
            an answer.

         r(eserved)

            These flag bits are reserved for future use, and MUST be set to
            zero, and ignored by the receiver.

      Message Type

         The Message Type field is two octets, and is used in order to
         communicate the message type with the message.  The 16-bit address
         space is managed by IANA [ianaweb].

      Session Identifier

         This field contains a 32 bit session identifier.

      Sequence Number

         This field contains contains a 32 bit sequence number.

      AVPs

         AVPs are a method of encapsulating information relevant to the
         PANA message.  See section Section 6.3 for more information on
         AVPs.
 */
class PANA_MsgHeader
{
    public:
       typedef struct {
          ACE_UINT16 request   : 1;  // Request flag
          ACE_UINT16 reserved  : 15; // reserved
       } Flags;

       // Default header length definition 
       typedef enum {
          HeaderLength = 16 // length in octet
       };

    public:
       PANA_MsgHeader() {
           m_Version = PANA_VERSION;
           m_Length  = 0;
           m_Type    = 0;
           m_SessionId = 0;
           m_SeqNum  = 0;
           ACE_OS::memset(&m_Flags, 0, sizeof(PANA_MsgHeader::Flags));
       }
       virtual ~PANA_MsgHeader() {
       }
       inline UCHAR &version() {
           return m_Version; 
       }
       inline ACE_UINT16 &length() {
           return m_Length;
       }
       inline PANA_MsgHeader::Flags &flags() {
           return m_Flags;
       }
       inline ACE_UINT16 &type() {
           return m_Type;
       }
       inline ACE_UINT32 &sessionId() {
           return m_SessionId;
       }
       inline ACE_UINT32 &seq() {
           return m_SeqNum;
       }

    protected:
       // flat header members
       UCHAR m_Version;
       ACE_UINT16 m_Length;
       PANA_MsgHeader::Flags m_Flags;
       ACE_UINT16 m_Type;
       ACE_UINT32 m_SessionId;
       ACE_UINT32 m_SeqNum;
};

// PANA Message definition
class PANA_Message :
     public PANA_MsgHeader
{
     public:
         virtual ~PANA_Message() {
             m_AvpList.releaseContainers();
         }
         AAAAvpContainerList &avpList() {
             return m_AvpList;
         }
         ACE_INET_Addr &srcAddress() {
             return m_SrcAddress;
         }
         ACE_INET_Addr &destAddress() {
             return m_DestAddress;
         }

     protected:
         AAAAvpContainerList m_AvpList;
         ACE_INET_Addr m_SrcAddress;
         ACE_INET_Addr m_DestAddress;
};

#endif /* __PANA_MESSAGE_H__ */
