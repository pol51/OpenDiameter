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

#ifndef __AAA_PEER_DATA_H__
#define __AAA_PEER_DATA_H__

#include "aaa_transport_collector.h"
#include "aaa_transport_ace.h"
#include "aaa_data_defs.h"

typedef AAA_ACE_Transport<ACE_SOCK_Acceptor,
                          ACE_SOCK_Connector,
                          ACE_SOCK_Stream> AAA_TransportTCP;
typedef AAA_ACE_Transport<ACE_SSL_SOCK_Acceptor,
                          ACE_SSL_SOCK_Connector,
                          ACE_SSL_SOCK_Stream> AAA_TransportTLS;

typedef AAA_IO_Connector<AAA_TransportTCP, AAA_MsgCollector> AAA_TcpConnector;
typedef AAA_IO_Connector<AAA_TransportTLS, AAA_MsgCollector> AAA_TlsConnector;

typedef AAA_IO_Acceptor<AAA_TransportTCP, AAA_MsgCollector> AAA_TcpAcceptor;
typedef AAA_IO_Acceptor<AAA_TransportTLS, AAA_MsgCollector> AAA_TlsAcceptor;

typedef AAA_ACE_TransportAddress AAA_IpAddress;

typedef enum {
    AAA_PEER_CONN_INITIATOR = 0,
    AAA_PEER_CONN_RESPONDER,
    AAA_PEER_CONN_MAX,
} AAA_PEER_CONN;

typedef enum {
    AAA_PEER_TTYPE_TLS = 0,
    AAA_PEER_TTYPE_TCP,
    AAA_PEER_TTYPE_MAX
} AAA_PEER_TTYPE;

typedef enum {
    AAA_DISCONNECT_REBOOTING       = 0,
    AAA_DISCONNECT_BUSY            = 1,
    AAA_DISCONNECT_DONTWANTTOTALK  = 2,
    AAA_DISCONNECT_UNKNOWN         = 1000,
    AAA_DISCONNECT_TRANSPORT       = 10001,
    AAA_DISCONNECT_TIMEOUT         = 10002,
} AAA_DISCONNECT_CAUSE;

typedef std::list<diameter_address_t*> AAA_HostIpLst;

typedef struct
{
   diameter_unsigned32_t m_LastTxHopId;
   diameter_unsigned32_t m_LastTxEndId;
   diameter_unsigned32_t m_LastRxHopId;
   diameter_unsigned32_t m_LastRxEndId;
} MsgId;

typedef struct
{
   diameter_octetstring_t m_Host;
   diameter_octetstring_t m_Realm;
   AAA_HostIpLst m_HostIpLst;
   diameter_unsigned32_t m_VendorId;
   diameter_utf8string_t m_ProductName; 
   diameter_unsigned32_t m_OriginStateId;
   AAA_ApplicationIdLst m_SupportedVendorIdLst;
   AAA_ApplicationIdLst m_AuthAppIdLst;
   diameter_unsigned32_t m_InbandSecurityId;
   AAA_ApplicationIdLst m_AcctAppIdLst;
   AAA_VendorSpecificIdLst m_VendorSpecificId;
   diameter_unsigned32_t m_FirmwareRevision;
   MsgId m_MsgId;
} AAA_PeerCapabilities;

typedef struct
{
   diameter_octetstring_t m_Identity;
   diameter_unsigned32_t  m_Port;
   AAA_DISCONNECT_CAUSE  m_DisconnectCause;
   int m_Expiration;
   bool m_Static;
   bool m_TLS;
   std::auto_ptr<AAA_IO_Base> m_IOInitiator;
   std::auto_ptr<AAA_IO_Base> m_IOResponder;
   AAA_PeerCapabilities m_PeerCapabilities; 
} AAA_PeerData;

#endif /* __AAA_PEER_DATA_H__ */

