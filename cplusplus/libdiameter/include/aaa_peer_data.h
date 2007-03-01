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

#ifndef __AAA_PEER_DATA_H__
#define __AAA_PEER_DATA_H__

#include "aaa_transport_collector.h"
#include "aaa_transport_ace.h"
#include "aaa_data_defs.h"

typedef Diameter_ACE_Transport<ACE_SOCK_Acceptor,
                               ACE_SOCK_Connector,
                               ACE_SOCK_Stream,
                               ACE_INET_Addr,
                               IPPROTO_TCP> DiameterTransportTCP;
typedef Diameter_ACE_Transport<ACE_SOCK_SEQPACK_Acceptor,
                               ACE_SOCK_SEQPACK_Connector,
                               ACE_SOCK_SEQPACK_Association,
                               ACE_Multihomed_INET_Addr,
                               IPPROTO_SCTP> DiameterTransportSCTP;

typedef Diameter_IO_Connector<DiameterTransportTCP,
                              ACE_INET_Addr,
                              DiameterRxMsgCollector> DiameterTcpConnector;
typedef Diameter_IO_Connector<DiameterTransportSCTP,
                              ACE_Multihomed_INET_Addr,
                              DiameterRxMsgCollector> DiameterSctpConnector;

typedef Diameter_IO_Acceptor<DiameterTransportTCP,
                             ACE_INET_Addr,
                             DiameterRxMsgCollector> DiameterTcpAcceptor;
typedef Diameter_IO_Acceptor<DiameterTransportSCTP,
                             ACE_Multihomed_INET_Addr,
                             DiameterRxMsgCollector> DiameterSctpAcceptor;

typedef Diameter_ACE_TransportAddress DiameterIpAddress;

typedef enum {
    DIAMETER_PEER_CONN_INITIATOR = 0,
    DIAMETER_PEER_CONN_RESPONDER,
    DIAMETER_PEER_CONN_MAX,
} DIAMETER_PEER_CONN;

typedef enum {
    DIAMETER_PEER_TTYPE_SCTP = 0,
    DIAMETER_PEER_TTYPE_TCP,
    DIAMETER_PEER_TTYPE_MAX
} DIAMETER_PEER_TTYPE;

typedef enum {
    AAA_DISCONNECT_REBOOTING       = 0,
    AAA_DISCONNECT_BUSY            = 1,
    AAA_DISCONNECT_DONTWANTTOTALK  = 2,
    AAA_DISCONNECT_UNKNOWN         = 1000,
    AAA_DISCONNECT_TRANSPORT       = 10001,
    AAA_DISCONNECT_TIMEOUT         = 10002,
} DIAMETER_DISCONNECT_CAUSE;

typedef struct
{
   AAA_ProtectedMap<diameter_unsigned32_t, diameter_unsigned32_t> m_LastTxHopId;
   AAA_ProtectedMap<diameter_unsigned32_t, diameter_unsigned32_t> m_LastTxEndId;
   AAA_ProtectedQueue<diameter_unsigned32_t> m_LastRxHopId;
   AAA_ProtectedQueue<diameter_unsigned32_t> m_LastRxEndId;
} DiameterMsgId;

typedef struct
{
   diameter_octetstring_t m_Host;
   diameter_octetstring_t m_Realm;
   diameter_utf8string_t m_ProductName; 
   diameter_unsigned32_t m_VendorId;
   diameter_unsigned32_t m_InbandSecurityId;
   diameter_unsigned32_t m_OriginStateId;
   diameter_unsigned32_t m_FirmwareRevision;
   DiameterApplicationIdLst m_SupportedVendorIdLst;
   DiameterApplicationIdLst m_AuthAppIdLst;
   DiameterHostIpLst m_HostIpLst;
   DiameterApplicationIdLst m_AcctAppIdLst;
   DiameterVendorSpecificIdLst m_VendorSpecificId;
   DiameterMsgId m_MsgId;
} DiameterPeerCapabilities;

typedef struct
{
   diameter_octetstring_t m_Identity;
   diameter_unsigned32_t  m_Port;
   DIAMETER_DISCONNECT_CAUSE  m_DisconnectCause;
   int m_Expiration;
   bool m_Static;
   bool m_TLS;
   bool m_UseSctp;
   std::auto_ptr<Diameter_IO_Base> m_IOInitiator;
   std::auto_ptr<Diameter_IO_Base> m_IOResponder;
   DiameterPeerCapabilities m_PeerCapabilities; 
} DiameterPeerData;

#endif /* __AAA_PEER_DATA_H__ */

