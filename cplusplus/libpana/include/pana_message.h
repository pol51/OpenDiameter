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

#ifndef __PANA_MESSAGES_H__
#define __PANA_MESSAGES_H__

#include <list>
#include "ace/Basic_Types.h"
#include "ace/Message_Block.h"
#include "pana_defs.h"
#include "pana_exports.h"
#include "pana_device_id.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define PANA_DICT_PROTOCOL_ID  1

/*!
 * PANA AVP header
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

    6.2.  PANA Header

    A summary of the PANA header format is shown below.  The fields are
    transmitted in network byte order.


        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |    Version    |   Reserved    |        Message Length         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |            Flags              |         Message Type          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                      Sequence Number                          |
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
    |R L r r r r r r r r r r r r r r|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        R(equest)

            If set, the message is a request.  If cleared, the message is
            an answer.

        L(stateLess handshake)

            When the L-flag is set in a PANA-Start-Request message it
            indicates that the PAA is performing stateless handshake.
            Cookie AVP MUST be included in both the PANA-Start-Request and
            the PANA-Start-Answer messages when performing stateless
            handshake.

        r(eserved)

            These flag bits are reserved for future use, and MUST be set to
            zero, and ignored by the receiver.

    Message Type

        The Message Type field is two octets, and is used in order to
        communicate the message type with the message.  The 16-bit address
        space is managed by IANA [ianaweb].  PANA uses its own address
        space for this field.

    Sequence Number

        The Sequence Number field contains a 32 bit value.

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
          ACE_UINT16 stateless : 1;  // Stateless flag
          ACE_UINT16 reserved  : 14;
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
       inline ACE_UINT32 &seq() {
           return m_SeqNum;
       }

    protected:
       // flat header members
       UCHAR m_Version;
       ACE_UINT16 m_Length;
       PANA_MsgHeader::Flags m_Flags;
       ACE_UINT16 m_Type;
       ACE_UINT32 m_SeqNum;
};

// PANA Message definition
class PANA_Message : public PANA_MsgHeader
{
     public:
         PANA_Message() :
             m_SrcPort(0) {
         }
         virtual ~PANA_Message() {
             m_AvpList.releaseContainers();
         }
         AAAAvpContainerList &avpList() {
             return m_AvpList;
         }
         PANA_DeviceIdContainer &srcDevices() {
             return m_SrcDevices;
         }
         ACE_UINT32 &srcPort() {
             return m_SrcPort;
         }

     protected:
         AAAAvpContainerList m_AvpList;
         PANA_DeviceIdContainer m_SrcDevices;
         ACE_UINT32 m_SrcPort;
};

#endif /* __PANA_MESSAGE_H__ */
