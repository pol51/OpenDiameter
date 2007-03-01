
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

#include "aaa_data_defs.h"
#include "aaa_peer_fsm.h"
#include "aaa_route_msg_router.h"

#define DIAMETER_PEER_CONNECT_ATTEMPT_TOUT 30

DiameterPeerStateTable DiameterPeerStateMachine::m_StateTable;

class AAA_ApplicationIdLookup
{
   public:
      static inline bool Find(diameter_unsigned32_t id,
                              DiameterApplicationIdLst &lst) {
          DiameterApplicationIdLst::iterator i = lst.begin();
          for (; i!= lst.end(); i++) {
              if ((id == DIAMETER_RELAY_APPLICATION_ID) ||
                  (*i == DIAMETER_RELAY_APPLICATION_ID) ||
                  (*i == id)) {
                  return true;
              }
          }
          return false;
      }
      static inline bool Find(diameter_unsigned32_t id,
                              DiameterApplicationIdLst *lst[],
                              int count) {
          for (int i=0; i<count; i++) {
              if (Find(id, *(lst[i]))) {
                  return true;
              }
          }
          return false;
      }
      static inline bool Find(diameter_unsigned32_t id,
                              DiameterVendorSpecificIdLst &lst) {
          DiameterVendorSpecificIdLst::iterator i = lst.begin();
          for (; i != lst.end(); i++) {
              if (((*i).authAppId == id) ||
                  ((*i).acctAppId == id) ||
                  ((*i).authAppId == DIAMETER_RELAY_APPLICATION_ID) ||
                  ((*i).acctAppId == DIAMETER_RELAY_APPLICATION_ID) ||
                  (id == DIAMETER_RELAY_APPLICATION_ID) ||
                  (id == DIAMETER_RELAY_APPLICATION_ID)) {
                  return true;
              }
          }
          return false;
      }
};

void DiameterPeerR_ISendConnReq::operator()(DiameterPeerStateMachine &fsm)
{
    fsm.ScheduleTimer(DIAMETER_PEER_EV_TIMEOUT,
                      DIAMETER_PEER_CONNECT_ATTEMPT_TOUT,
                      0,
                      DIAMETER_PEER_EV_TIMEOUT);
}

void DiameterPeerR_AcceptSendCEA::operator()(DiameterPeerStateMachine &fsm)
{
    fsm.PeerData().m_IOResponder = fsm.m_CurrentPeerEventParam->m_IO;
    std::auto_ptr<DiameterMsg> cer = fsm.m_CurrentPeerEventParam->m_Msg;

    fsm.DisassembleCE(*cer);

    std::string message;
    diameter_unsigned32_t rcode;
    bool valid = fsm.ValidatePeer(rcode, message);
    fsm.SendCEA(rcode, message);

    if (! valid) {
        AAA_LOG((LM_INFO, "(%P|%t) %s in connection attempt, discarding\n",
                   message.c_str()));
        fsm.Cleanup();
    }
    else {
        fsm.CancelTimer(DIAMETER_PEER_EV_TIMEOUT);
        fsm.ScheduleTimer(DIAMETER_PEER_EV_WATCHDOG,
                          DIAMETER_CFG_TRANSPORT()->watchdog_timeout,
                          0,
                          DIAMETER_PEER_EV_WATCHDOG);
        fsm.StopReConnect();
        fsm.Connected();
    }
}

void DiameterPeerI_SendCER::operator()(DiameterPeerStateMachine &fsm)
{
    AAA_LOG((LM_DEBUG, "(%P|%t) Connection attempt accepted\n"));
    fsm.PeerData().m_IOInitiator = fsm.m_CurrentPeerEventParam->m_IO;
    fsm.SendCER();
}

void DiameterPeer_ConnNack::operator()(DiameterPeerStateMachine &fsm)
{
    fsm.Cleanup();
    fsm.DoReConnect();
    fsm.Error(AAA_UNABLE_TO_COMPLY);
}

void DiameterPeer_Cleanup::operator()(DiameterPeerStateMachine &fsm)
{
    fsm.Cleanup();
}

void DiameterPeer_ReConnect::operator()(DiameterPeerStateMachine &fsm)
{
    AAA_LOG((LM_INFO,
            "(%P|%t) Retrying peer connection. Number of attemps %d\n", fsm.m_ReconnectAttempt));

    fsm.StopReConnect();
    reinterpret_cast<DiameterPeerEntry*>(&fsm)->Start();
}

void DiameterPeerR_Accept::operator()(DiameterPeerStateMachine &fsm)
{
    fsm.PeerData().m_IOResponder = fsm.m_CurrentPeerEventParam->m_IO;
    std::auto_ptr<DiameterMsg> cer = fsm.m_CurrentPeerEventParam->m_Msg;

    fsm.DisassembleCE(*cer);

    std::string message;
    diameter_unsigned32_t rcode;
    if (! fsm.ValidatePeer(rcode, message)) {
        AAA_LOG((LM_INFO, "(%P|%t) %s during election, discarding\n",
                   message.c_str()));
        fsm.Cleanup();
    }
    else {
        AAA_LOG((LM_DEBUG, "(%P|%t) *** Peer capabilities accepted ***\n"));
    }
}

void DiameterPeer_Error::operator()(DiameterPeerStateMachine &fsm)
{
    AAA_LOG((LM_INFO, "(%P|%t) Peer connection did not complete, closing\n"));
    fsm.Cleanup();
    fsm.Error(AAA_LIMITED_SUCCESS);
}

void DiameterPeer_ProcessCEA::operator()(DiameterPeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> cea = fsm.m_CurrentPeerEventParam->m_Msg;

    DiameterMsgResultCode rcode(*cea);
    if (rcode.InterpretedResultCode() ==
        DiameterMsgResultCode::RCODE_SUCCESS) {
        fsm.DisassembleCE(*cea);
        fsm.StopReConnect();
        fsm.CancelTimer(DIAMETER_PEER_EV_TIMEOUT);
        fsm.ScheduleTimer(DIAMETER_PEER_EV_WATCHDOG,
                          DIAMETER_CFG_TRANSPORT()->watchdog_timeout,
                          0,
                          DIAMETER_PEER_EV_WATCHDOG);
        AAA_LOG((LM_DEBUG, "(%P|%t) *** Local capabilities accepted by peer ***\n"));
        fsm.Connected();
    }
    else {
       DiameterUtf8AvpContainerWidget errorMsg(cea->acl);
       diameter_utf8string_t *strMsg = errorMsg.GetAvp
           (DIAMETER_AVPNAME_ERRORMESSAGE);
       if (strMsg) {
           AAA_LOG((LM_INFO,
                 "(%P|%t) Peer returned an error on CEA: %s\n",
                      strMsg->c_str()));
       }
       fsm.Cleanup();
    }
}

void DiameterPeerR_AcceptElect::operator()(DiameterPeerStateMachine &fsm)
{
    fsm.PeerData().m_IOResponder = fsm.m_CurrentPeerEventParam->m_IO;
    std::auto_ptr<DiameterMsg> cer = fsm.m_CurrentPeerEventParam->m_Msg;

    fsm.DisassembleCE(*cer);

    std::string message;
    diameter_unsigned32_t rcode;
    if (! fsm.ValidatePeer(rcode, message)) {
        AAA_LOG((LM_INFO, "(%P|%t) %s during election, discarding\n",
                   message.c_str()));
        fsm.Cleanup(DiameterPeerStateMachine::CLEANUP_ALL &
                    ~DiameterPeerStateMachine::CLEANUP_IO_I);
    }
    else {
        fsm.Elect();
    }
}

void DiameterPeerI_SendCERElect::operator()(DiameterPeerStateMachine &fsm)
{
    fsm.PeerData().m_IOInitiator = fsm.m_CurrentPeerEventParam->m_IO;    
    fsm.SendCER();
    fsm.Elect();
}

void DiameterPeerR_SendCEA::operator()(DiameterPeerStateMachine &fsm)
{
    fsm.PeerData().m_IOResponder = fsm.m_CurrentPeerEventParam->m_IO;    
    std::string message = "Capabilities negotiation completed successfully (win-election)";
    fsm.SendCEA(AAA_SUCCESS, message);
    fsm.CancelTimer(DIAMETER_PEER_EV_TIMEOUT);
    fsm.ScheduleTimer(DIAMETER_PEER_EV_WATCHDOG,
                      DIAMETER_CFG_TRANSPORT()->watchdog_timeout,
                      0,
                      DIAMETER_PEER_EV_WATCHDOG);
    AAA_LOG((LM_DEBUG, "(%P|%t) %s\n", message.c_str()));
    fsm.Connected();
}

void DiameterPeerR_SendCEAOpen::operator()(DiameterPeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> cer = fsm.m_CurrentPeerEventParam->m_Msg;

    fsm.DisassembleCE(*cer);

    std::string message;
    diameter_unsigned32_t rcode;
    if (! fsm.ValidatePeer(rcode, message)) {
        AAA_LOG((LM_INFO, "(%P|%t) %s during cap re-negotiation\n",
                   message.c_str()));
    }
    else {
        AAA_LOG((LM_DEBUG, "(%P|%t) *** Peer capabilities accepted ***\n"));
    }
    fsm.SendCEA(rcode, message);
}

void DiameterPeerR_DisconnectResp::operator()(DiameterPeerStateMachine &fsm)
{
    AAA_LOG((LM_DEBUG, "(%P|%t) Disconnecting responder\n"));
    DIAMETER_IO_GC().ScheduleForDeletion(fsm.PeerData().m_IOResponder);
}

void DiameterPeerR_DisconnectIOpen::operator()(DiameterPeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> cea = fsm.m_CurrentPeerEventParam->m_Msg;

    DiameterMsgResultCode rcode(*cea);
    if (rcode.InterpretedResultCode() ==
        DiameterMsgResultCode::RCODE_SUCCESS) {
        fsm.DisassembleCE(*cea);
        fsm.CancelTimer(DIAMETER_PEER_EV_TIMEOUT);
        fsm.ScheduleTimer(DIAMETER_PEER_EV_WATCHDOG,
                          DIAMETER_CFG_TRANSPORT()->watchdog_timeout,
                          0,
                          DIAMETER_PEER_EV_WATCHDOG);
        DIAMETER_IO_GC().ScheduleForDeletion(fsm.PeerData().m_IOResponder);
        AAA_LOG((LM_DEBUG, "(%P|%t) *** Initiator capabilities accepted ***\n"));
        fsm.Connected();
    }
    else {
       DiameterUtf8AvpContainerWidget errorMsg(cea->acl);
       diameter_utf8string_t *strMsg = errorMsg.GetAvp
           (DIAMETER_AVPNAME_ERRORMESSAGE);
       if (strMsg) {
           AAA_LOG((LM_INFO,
                 "(%P|%t) Peer returned an error on CEA: %s\n",
                      strMsg->c_str()));
       }
       fsm.Cleanup();
    }
}

void DiameterPeerR_Reject::operator()(DiameterPeerStateMachine &fsm)
{
    AAA_LOG((LM_DEBUG, "(%P|%t) Responder connection attempt rejected\n"));
    std::auto_ptr<Diameter_IO_Base> io = fsm.m_CurrentPeerEventParam->m_IO;
    std::auto_ptr<DiameterMsg> msg = fsm.m_CurrentPeerEventParam->m_Msg;
}

void DiameterPeerI_DisconnectSendCEA::operator()(DiameterPeerStateMachine &fsm)
{
    std::string message = "Capabilities negotiation completed successfully (win-election)";
    DIAMETER_IO_GC().ScheduleForDeletion(fsm.PeerData().m_IOInitiator);
    fsm.SendCEA(AAA_SUCCESS, message);
    fsm.CancelTimer(DIAMETER_PEER_EV_TIMEOUT);
    fsm.ScheduleTimer(DIAMETER_PEER_EV_WATCHDOG,
                      DIAMETER_CFG_TRANSPORT()->watchdog_timeout,
                      0,
                      DIAMETER_PEER_EV_WATCHDOG);
    AAA_LOG((LM_DEBUG, "(%P|%t) %s\n", message.c_str()));
    fsm.Connected();
}

void DiameterPeerR_SendMessage::operator()(DiameterPeerStateMachine &fsm)
{
#if ASYNC_SEND
    boost::shared_ptr<DiameterMsg> msg = fsm.DequeueSendMsg();
    if (fsm.Send(*msg, fsm.PeerData().m_IOResponder.get()) < 0) {
        AAA_LOG((LM_INFO, "(%P|%t) Error sending message: %d\n",
                   msg->hdr.code));
    }
#endif
}

void DiameterPeer_Process::operator()(DiameterPeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> msg = fsm.m_CurrentPeerEventParam->m_Msg;
    DiameterMsgQuery query(*msg);
    if (query.IsRequest()) {
        DIAMETER_MSG_ROUTER()->RequestMsg(msg,
               reinterpret_cast<DiameterPeerEntry*>(&fsm));
    }
    else {
        DIAMETER_MSG_ROUTER()->AnswerMsg(msg,
               reinterpret_cast<DiameterPeerEntry*>(&fsm));
    }
}

void DiameterPeerProcessDWRSendDWA::operator()(DiameterPeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> dwr = fsm.m_CurrentPeerEventParam->m_Msg;
    fsm.DisassembleDW(*dwr);

    // TBD: do failover here
    
    std::string message = "Successful device watchdog";
    fsm.SendDWA(AAA_SUCCESS, message);
}

void DiameterPeer_ProcessDWA::operator()(DiameterPeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> dwa = fsm.m_CurrentPeerEventParam->m_Msg;

    DiameterMsgResultCode rcode(*dwa);
    if (rcode.InterpretedResultCode() ==
        DiameterMsgResultCode::RCODE_SUCCESS) {
        fsm.DisassembleDW(*dwa);
    }
    else {
        DiameterUtf8AvpContainerWidget errorMsg(dwa->acl);
        diameter_utf8string_t *strMsg = errorMsg.GetAvp
            (DIAMETER_AVPNAME_ERRORMESSAGE);
        if (strMsg) {
            AAA_LOG((LM_INFO,
                  "(%P|%t) Peer returned an error on Watchdog: %s, closing peer\n",
                       strMsg->c_str()));
        }
        fsm.Cleanup();
    }
}

void DiameterPeerI_SendMessage::operator()(DiameterPeerStateMachine &fsm)
{
#if ASYNC_SEND
    boost::shared_ptr<DiameterMsg> msg = fsm.DequeueSendMsg();
    if (fsm.Send(*msg, fsm.PeerData().m_IOInitiator.get()) < 0) {
        AAA_LOG((LM_INFO, "(%P|%t) Error sending message: %d\n",
                   msg->hdr.code));
    }
#endif
}

void DiameterPeerI_SendCEA::operator()(DiameterPeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> cer = fsm.m_CurrentPeerEventParam->m_Msg;

    fsm.DisassembleCE(*cer);

    std::string message;
    diameter_unsigned32_t rcode;
    if (! fsm.ValidatePeer(rcode, message)) {
        AAA_LOG((LM_INFO, "(%P|%t) %s during cap re-negotiation\n",
                   message.c_str()));
    }
    fsm.SendCEA(rcode, message);
}

void DiameterPeerI_SendDPR::operator()(DiameterPeerStateMachine &fsm)
{
    fsm.CancelTimer(DIAMETER_PEER_EV_WATCHDOG);
    fsm.ScheduleTimer(DIAMETER_PEER_EV_TIMEOUT,
                      DIAMETER_PEER_CONNECT_ATTEMPT_TOUT,
                      0,
                      DIAMETER_PEER_EV_TIMEOUT);
    fsm.SendDPR(true);
}

void DiameterPeerR_SendDPR::operator()(DiameterPeerStateMachine &fsm)
{
    fsm.CancelTimer(DIAMETER_PEER_EV_WATCHDOG);
    fsm.ScheduleTimer(DIAMETER_PEER_EV_TIMEOUT,
                      DIAMETER_PEER_CONNECT_ATTEMPT_TOUT,
                      0,
                      DIAMETER_PEER_EV_TIMEOUT);
    fsm.SendDPR(false);
}

void DiameterPeerI_SendDPADisconnect::operator()(DiameterPeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> dpr = fsm.m_CurrentPeerEventParam->m_Msg;

    AAA_LOG((LM_INFO, "(%P|%t) Peer initiator requested termination, disconnecting\n"));
    fsm.SendDPA(true, AAA_SUCCESS);

    DiameterUInt32AvpContainerWidget cause(dpr->acl);
    diameter_unsigned32_t *uint32 = cause.GetAvp(DIAMETER_AVPNAME_DISCONNECT_CAUSE);

    fsm.Cleanup();
    fsm.Disconnected
         ((uint32) ? *uint32 : AAA_DISCONNECT_UNKNOWN);
}

void DiameterPeerR_SendDPADisconnect::operator()(DiameterPeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> dpr = fsm.m_CurrentPeerEventParam->m_Msg;

    AAA_LOG((LM_INFO, "(%P|%t) Peer responder requested termination, disconnecting\n"));
    fsm.SendDPA(false, AAA_SUCCESS);

    DiameterUInt32AvpContainerWidget cause(dpr->acl);
    diameter_unsigned32_t *uint32 = cause.GetAvp(DIAMETER_AVPNAME_DISCONNECT_CAUSE);

    fsm.Cleanup();
    fsm.Disconnected
         ((uint32) ? *uint32 : AAA_DISCONNECT_UNKNOWN);
}

void DiameterPeer_Disconnect::operator()(DiameterPeerStateMachine &fsm)
{
    AAA_LOG((LM_INFO, "(%P|%t) General disconnection\n"));
    fsm.Cleanup();
    fsm.DoReConnect();
    fsm.Disconnected(AAA_DISCONNECT_TRANSPORT);
}

void DiameterPeer_DisconnectDPA::operator()(DiameterPeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> dpa = fsm.m_CurrentPeerEventParam->m_Msg;

    DiameterMsgResultCode rcode(*dpa);
    if (rcode.InterpretedResultCode() ==
        DiameterMsgResultCode::RCODE_SUCCESS) {
        fsm.DisassembleDP(*dpa);
    }
    else {
        DiameterUtf8AvpContainerWidget errorMsg(dpa->acl);
        diameter_utf8string_t *strMsg = errorMsg.GetAvp
            (DIAMETER_AVPNAME_ERRORMESSAGE);
        if (strMsg) {
            AAA_LOG((LM_INFO,
               "(%P|%t) Peer returned an error on Watchdog: %s, closing peer\n",
                    strMsg->c_str()));
        }
    }

    fsm.Disconnected
        (fsm.PeerData().m_DisconnectCause);
    fsm.Cleanup();

    if (fsm.PeerData().m_DisconnectCause == AAA_DISCONNECT_REBOOTING) {
        fsm.DoReConnect();
    }
}

void DiameterPeer_Watchdog::operator()(DiameterPeerStateMachine &fsm)
{
    fsm.SendDWR();
}

void DiameterPeerStateMachine::AssembleCE(DiameterMsg &msg,
                                      bool request)
{
   DiameterMsgHeader &h = msg.hdr;
   ACE_OS::memset(&msg.hdr, 0, sizeof(msg.hdr));
   h.ver = DIAMETER_PROTOCOL_VERSION;
   h.length = 0;
   h.flags.r = request ? DIAMETER_FLAG_SET : DIAMETER_FLAG_CLR;
   h.flags.p = DIAMETER_FLAG_CLR;
   h.flags.e = DIAMETER_FLAG_CLR;
   h.code = DIAMETER_MSGCODE_CAPABILITIES_EXCHG;
   h.appId = DIAMETER_BASE_APPLICATION_ID;
   MsgIdTxMessage(msg);

   DiameterIdentityAvpWidget originHost(DIAMETER_AVPNAME_ORIGINHOST);
   DiameterIdentityAvpWidget originRealm(DIAMETER_AVPNAME_ORIGINREALM);
   DiameterAddressAvpWidget  hostIp(DIAMETER_AVPNAME_HOSTIP);
   DiameterUInt32AvpWidget   vendorId(DIAMETER_AVPNAME_VENDORID);
   DiameterUtf8AvpWidget     product(DIAMETER_AVPNAME_PRODUCTNAME);
   DiameterUInt32AvpWidget   originStateId(DIAMETER_AVPNAME_ORIGINSTATEID);
   DiameterUInt32AvpWidget   supportedVendorId(DIAMETER_AVPNAME_SUPPORTEDVENDORID);
   DiameterUInt32AvpWidget   authId(DIAMETER_AVPNAME_AUTHAPPID);
   DiameterUInt32AvpWidget   acctId(DIAMETER_AVPNAME_ACCTAPPID);
   DiameterGroupedAvpWidget  vendorSpecificId(DIAMETER_AVPNAME_VENDORAPPID);
   DiameterUInt32AvpWidget   firmware(DIAMETER_AVPNAME_FIRMWAREREV);
   DiameterUInt32AvpWidget   inbandSecId(DIAMETER_AVPNAME_INBANDSECID);

   originHost.Get() = DIAMETER_CFG_TRANSPORT()->identity;
   originRealm.Get() = DIAMETER_CFG_TRANSPORT()->realm;
   firmware.Get() = DIAMETER_CFG_GENERAL()->version;
   inbandSecId.Get() = m_Data.m_TLS;

   // Host-IP-Address
   DiameterIpAddress tool;
   ACE_INET_Addr identityAddr(0, DIAMETER_CFG_TRANSPORT()->identity.c_str(),
#ifdef ACE_HAS_IPV6
                              (DIAMETER_CFG_TRANSPORT()->use_ipv6) ? AF_INET6 : AF_INET);
#else /* ! ACE_HAS_IPV6 */
                               AF_INET);
#endif /* ! ACE_HAS_IPV6 */
   diameter_address_t &ipAvp = hostIp.Get();
   ipAvp.type = AAA_ADDRESS_IP;
   ipAvp.value.assign((char*)tool.GetAddressPtr(identityAddr),
                      tool.GetAddressSize(identityAddr));

   if (TransportProtocolInUse() == IPPROTO_SCTP) {
       if (DIAMETER_CFG_TRANSPORT()->advertised_hostname.size() > 0) {
           std::list<std::string>::iterator i =
               DIAMETER_CFG_TRANSPORT()->advertised_hostname.begin();
           for (; i != DIAMETER_CFG_TRANSPORT()->advertised_hostname.end(); i++) {
               ACE_INET_Addr hostAddrs;
               if (! hostAddrs.set(0, (*i).c_str(),
#ifdef ACE_HAS_IPV6
                                   (DIAMETER_CFG_TRANSPORT()->use_ipv6) ? AF_INET6 : AF_INET)) {
#else /* ! ACE_HAS_IPV6 */
                                   AF_INET)) {
#endif /* ! ACE_HAS_IPV6 */
                   diameter_address_t &hostIpAvp = hostIp.Get();
                   hostIpAvp.type = AAA_ADDRESS_IP;
                   hostIpAvp.value.assign((char*)tool.GetAddressPtr(hostAddrs),
                                      tool.GetAddressSize(hostAddrs));
               }
           }
       }
   }

   vendorId.Get() = DIAMETER_CFG_GENERAL()->vendor;
   product.Get() = DIAMETER_CFG_GENERAL()->product;
   originStateId.Get() = DIAMETER_CFG_RUNTIME()->originStateId;

   DiameterApplicationIdLst *idList[] = {
       &DIAMETER_CFG_GENERAL()->supportedVendorIdLst,
       &DIAMETER_CFG_GENERAL()->authAppIdLst,
       &DIAMETER_CFG_GENERAL()->acctAppIdLst
   };
   DiameterUInt32AvpWidget *widgets[] = {
       &supportedVendorId,
       &authId,
       &acctId
   };
   for (unsigned int i=0; i<sizeof(widgets)/sizeof(DiameterUInt32AvpWidget*); i++) {
       DiameterApplicationIdLst::iterator x = idList[i]->begin();
       for (; x != idList[i]->end(); x++) {
           widgets[i]->Get() = *x;
       }
       if (! widgets[i]->empty()) {
           msg.acl.add((*widgets[i])());
       }
   }

   DiameterVendorSpecificIdLst::iterator y =
       DIAMETER_CFG_GENERAL()->vendorSpecificId.begin();
   for (; y != DIAMETER_CFG_GENERAL()->vendorSpecificId.end();
        y++) {
       DiameterDataVendorSpecificApplicationId vid = *y;
       diameter_grouped_t &grp = vendorSpecificId.Get();

       if (vid.authAppId > 0) {
           DiameterUInt32AvpWidget gAuthId(DIAMETER_AVPNAME_AUTHAPPID, vid.authAppId);
           grp.add(gAuthId());
       }
       if (vid.acctAppId > 0) {
           DiameterUInt32AvpWidget gAcctId(DIAMETER_AVPNAME_ACCTAPPID, vid.acctAppId);
           grp.add(gAcctId());
       }
       if (vid.vendorId > 0) {
           DiameterUInt32AvpWidget gVendorId(DIAMETER_AVPNAME_VENDORID, vid.vendorId);
           grp.add(gVendorId());
       }
   }

   if (! vendorSpecificId.empty()) {
       msg.acl.add(vendorSpecificId());
   }

   msg.acl.add(originHost());
   msg.acl.add(originRealm());
   msg.acl.add(hostIp());
   msg.acl.add(vendorId());
   msg.acl.add(product());
   msg.acl.add(originStateId());
   msg.acl.add(firmware());
   msg.acl.add(inbandSecId());
}

void DiameterPeerStateMachine::DisassembleCE(DiameterMsg &msg)
{
   int ndx; 
   DiameterPeerCapabilities &cap = m_Data.m_PeerCapabilities;

   DiameterIdentityAvpContainerWidget originHost(msg.acl);
   DiameterIdentityAvpContainerWidget originRealm(msg.acl);
   DiameterAddressAvpContainerWidget hostIp(msg.acl);
   DiameterUInt32AvpContainerWidget vendorId(msg.acl);
   DiameterUtf8AvpContainerWidget product(msg.acl);
   DiameterUInt32AvpContainerWidget originState(msg.acl);
   DiameterUInt32AvpContainerWidget supportedVendorId(msg.acl);
   DiameterUInt32AvpContainerWidget authAppId(msg.acl);
   DiameterUInt32AvpContainerWidget inbandSecId(msg.acl);
   DiameterUInt32AvpContainerWidget acctAppId(msg.acl);
   DiameterGroupedAvpContainerWidget vendorSpecificId(msg.acl);
   DiameterUInt32AvpContainerWidget firmware(msg.acl);

   diameter_identity_t *identity = originHost.GetAvp(DIAMETER_AVPNAME_ORIGINHOST);
   cap.m_Host.assign((identity) ? identity->data() : "",
                     (identity) ? identity->length() : 0);

   identity = originRealm.GetAvp(DIAMETER_AVPNAME_ORIGINREALM);
   cap.m_Realm.assign((identity) ? identity->data() : "",
                      (identity) ? identity->length() : 0);

   while (! cap.m_HostIpLst.empty()) {
       diameter_address_t *ip = cap.m_HostIpLst.front();
       cap.m_HostIpLst.pop_front();
       delete ip;
   }

   diameter_address_t *address = hostIp.GetAvp(DIAMETER_AVPNAME_HOSTIP);
   for (ndx=1; address; ndx++) {
       diameter_address_t *newAddr = new diameter_address_t;
       *newAddr = *address;
       cap.m_HostIpLst.push_back(newAddr);
       address = hostIp.GetAvp(DIAMETER_AVPNAME_HOSTIP, ndx);
   }

   diameter_unsigned32_t *uint32 = vendorId.GetAvp(DIAMETER_AVPNAME_VENDORID);
   cap.m_VendorId = (uint32) ? *uint32 : 0;

   diameter_utf8string_t *utf8str = product.GetAvp(DIAMETER_AVPNAME_PRODUCTNAME);
   cap.m_ProductName.assign((utf8str) ? utf8str->data() : "",
                            (utf8str) ? utf8str->length() : 0);

   uint32 = originState.GetAvp(DIAMETER_AVPNAME_ORIGINSTATEID);
   cap.m_OriginStateId = (uint32) ? *uint32 : 0;

   uint32 = firmware.GetAvp(DIAMETER_AVPNAME_FIRMWAREREV);
   cap.m_FirmwareRevision = (uint32) ? *uint32 : 0;

   uint32 = inbandSecId.GetAvp(DIAMETER_AVPNAME_INBANDSECID);
   cap.m_InbandSecurityId = (uint32) ? *uint32 : 0;

   DiameterApplicationIdLst *idList[] = {
       &cap.m_SupportedVendorIdLst,
       &cap.m_AuthAppIdLst,
       &cap.m_AcctAppIdLst
   };
   DiameterUInt32AvpContainerWidget *widgets[] = {
       &supportedVendorId,
       &authAppId,
       &acctAppId
   };
   char *avpNames[] = {
       DIAMETER_AVPNAME_SUPPORTEDVENDORID,
       DIAMETER_AVPNAME_AUTHAPPID,
       DIAMETER_AVPNAME_ACCTAPPID
   };
   for (unsigned int i=0;
        i<sizeof(idList)/sizeof(DiameterApplicationIdLst*);
        i++) {
       while (! idList[i]->empty()) {
           idList[i]->pop_front();
       }
   }
   for (unsigned int i=0;
        i<sizeof(widgets)/sizeof(DiameterUInt32AvpContainerWidget*);
        i++) {
       uint32 = widgets[i]->GetAvp(avpNames[i]);
       for (ndx=1; uint32; ndx++) {
           idList[i]->push_back(*uint32);
           uint32 = widgets[i]->GetAvp(avpNames[i], ndx);
       }
   }

   while (! cap.m_VendorSpecificId.empty()) {
       cap.m_VendorSpecificId.pop_front();
   }
   diameter_grouped_t *grouped = vendorSpecificId.GetAvp(DIAMETER_AVPNAME_VENDORAPPID);
   for (ndx=1; grouped; ndx++) {
       DiameterDataVendorSpecificApplicationId vsid = { 0, 0, 0 };
       DiameterUInt32AvpContainerWidget gAuthId(*grouped);
       DiameterUInt32AvpContainerWidget gAcctId(*grouped);
       DiameterUInt32AvpContainerWidget gVendorId(*grouped);

       uint32 = gVendorId.GetAvp(DIAMETER_AVPNAME_VENDORID);
       vsid.vendorId = (uint32) ? *uint32 : 0;
       uint32 = gAuthId.GetAvp(DIAMETER_AVPNAME_AUTHAPPID);
       if (uint32) {
           vsid.authAppId = *uint32;
       }
       else {
           vsid.authAppId = 0;
           uint32 = gAcctId.GetAvp(DIAMETER_AVPNAME_ACCTAPPID);
           if (uint32) {
               vsid.acctAppId = *uint32;
           }
           else {
               vsid.acctAppId = 0;
               AAA_LOG((LM_INFO, "(%P|%t) WARNING: Peer advertising Vendor-Specific-App-Id with no Auth or Acct app Id AVPs\n"));
           }
       }

       if ((vsid.acctAppId != 0) || (vsid.authAppId != 0)) {
           cap.m_VendorSpecificId.push_back(vsid);
       }
       grouped = vendorSpecificId.GetAvp(DIAMETER_AVPNAME_VENDORAPPID, ndx);
   }

   DumpPeerCapabilities();
}

void DiameterPeerStateMachine::AssembleDW(DiameterMsg &msg, bool request)
{
   DiameterMsgHeader &h = msg.hdr;
   ACE_OS::memset(&msg.hdr, 0, sizeof(msg.hdr));
   h.ver = DIAMETER_PROTOCOL_VERSION;
   h.length = 0;
   h.flags.r = request ? DIAMETER_FLAG_SET : DIAMETER_FLAG_CLR;
   h.flags.p = DIAMETER_FLAG_CLR;
   h.flags.e = DIAMETER_FLAG_CLR;
   h.code = DIAMETER_MSGCODE_WATCHDOG;
   h.appId = DIAMETER_BASE_APPLICATION_ID;
   MsgIdTxMessage(msg);

   DiameterIdentityAvpWidget originHost(DIAMETER_AVPNAME_ORIGINHOST);
   DiameterIdentityAvpWidget originRealm(DIAMETER_AVPNAME_ORIGINREALM);
   DiameterUInt32AvpWidget   originStateId(DIAMETER_AVPNAME_ORIGINSTATEID);

   originHost.Get() = DIAMETER_CFG_TRANSPORT()->identity;
   originRealm.Get() = DIAMETER_CFG_TRANSPORT()->realm;
   originStateId.Get() = DIAMETER_CFG_RUNTIME()->originStateId;

   msg.acl.add(originHost());
   msg.acl.add(originRealm());
   msg.acl.add(originStateId());
}

void DiameterPeerStateMachine::DisassembleDW(DiameterMsg &msg)
{
   diameter_identity_t Host;
   diameter_identity_t Realm;

   DiameterIdentityAvpContainerWidget originHost(msg.acl);
   DiameterIdentityAvpContainerWidget originRealm(msg.acl);
   DiameterUInt32AvpContainerWidget originState(msg.acl);

   diameter_identity_t *identity = originHost.GetAvp(DIAMETER_AVPNAME_ORIGINHOST);
   Host.assign((identity) ? identity->data() : "",
               (identity) ? identity->length() : 0);

   identity = originRealm.GetAvp(DIAMETER_AVPNAME_ORIGINREALM);
   Realm.assign((identity) ? identity->data() : "",
                (identity) ? identity->length() : 0);

   diameter_unsigned32_t *uint32 = originState.GetAvp(DIAMETER_AVPNAME_ORIGINSTATEID);
   diameter_unsigned32_t OriginStateId = (uint32) ? *uint32 : 0;

   AAA_LOG((LM_INFO, "(%P|%t) Watchdog msg from [%s.%s], state=%d, time=%d\n",
             Host.c_str(), Realm.c_str(), OriginStateId, time(0)));
}

void DiameterPeerStateMachine::AssembleDP(DiameterMsg &msg,
                                      bool request)
{
   DiameterMsgHeader &h = msg.hdr;
   ACE_OS::memset(&msg.hdr, 0, sizeof(msg.hdr));
   h.ver = DIAMETER_PROTOCOL_VERSION;
   h.length = 0;
   h.flags.r = request ? DIAMETER_FLAG_SET : DIAMETER_FLAG_CLR;
   h.flags.p = DIAMETER_FLAG_CLR;
   h.flags.e = DIAMETER_FLAG_CLR;
   h.code = DIAMETER_MSGCODE_DISCONNECT_PEER;
   h.appId = DIAMETER_BASE_APPLICATION_ID;
   MsgIdTxMessage(msg);

   DiameterIdentityAvpWidget originHost(DIAMETER_AVPNAME_ORIGINHOST);
   DiameterIdentityAvpWidget originRealm(DIAMETER_AVPNAME_ORIGINREALM);

   originHost.Get() = DIAMETER_CFG_TRANSPORT()->identity;
   originRealm.Get() = DIAMETER_CFG_TRANSPORT()->realm;

   msg.acl.add(originHost());
   msg.acl.add(originRealm());

}

void DiameterPeerStateMachine::DisassembleDP(DiameterMsg &msg)
{
   diameter_identity_t Host;
   diameter_identity_t Realm;

   DiameterIdentityAvpContainerWidget originHost(msg.acl);
   DiameterIdentityAvpContainerWidget originRealm(msg.acl);

   diameter_identity_t *identity = originHost.GetAvp(DIAMETER_AVPNAME_ORIGINHOST);
   Host.assign((identity) ? identity->data() : "",
               (identity) ? identity->length() : 0);

   identity = originRealm.GetAvp(DIAMETER_AVPNAME_ORIGINREALM);
   Realm.assign((identity) ? identity->data() : "",
                (identity) ? identity->length() : 0);
              
   AAA_LOG((LM_INFO, "(%P|%t) Disconnect msg from [%s.%s]\n",
              Host.c_str(), Realm.c_str()));
}

void DiameterPeerStateMachine::SendCER()
{
    /*
       5.3.1.  Capabilities-Exchange-Request

          The Capabilities-Exchange-Request (CER), indicated by the Command-
          Code set to 257 and the Command Flags' 'R' bit set, is sent to
          exchange local capabilities.  Upon detection of a transport failure,
          this message MUST NOT be sent to an alternate peer.

          When Diameter is run over SCTP [SCTP], which allows for connections
          to span multiple interfaces and multiple IP addresses, the
          Capabilities-Exchange-Request message MUST contain one Host-IP-
          Address AVP for each potential IP address that MAY be locally used
          when transmitting Diameter messages.


          Message Format

             <CER> ::= < Diameter Header: 257, REQ >
                       { Origin-Host }
                       { Origin-Realm }
                    1* { Host-IP-Address }
                       { Vendor-Id }
                       { Product-Name }
                       [ Origin-State-Id ]
                     * [ Supported-Vendor-Id ]
                     * [ Auth-Application-Id ]
                     * [ Inband-Security-Id ]
                     * [ Acct-Application-Id ]
                     * [ Vendor-Specific-Application-Id ]
                       [ Firmware-Revision ]
                     * [ AVP ]
    */

   std::auto_ptr<DiameterMsg> msg(new DiameterMsg);
   AssembleCE(*msg);
   if (m_TxMsgCollector.Send(msg, m_Data.m_IOInitiator.get(), true) == 0) {
       AAA_LOG((LM_INFO, "(%P|%t) Sent CER\n"));
   }
}

void DiameterPeerStateMachine::SendCEA(diameter_unsigned32_t rcode,
                                   std::string &message)
{
    /*
      5.3.2.  Capabilities-Exchange-Answer

         The Capabilities-Exchange-Answer (CEA), indicated by the Command-Code
         set to 257 and the Command Flags' 'R' bit cleared, is sent in
         response to a CER message.

         When Diameter is run over SCTP [SCTP], which allows connections to
         span multiple interfaces, hence, multiple IP addresses, the
         Capabilities-Exchange-Answer message MUST contain one Host-IP-Address
         AVP for each potential IP address that MAY be locally used when
         transmitting Diameter messages.

         Message Format

            <CEA> ::= < Diameter Header: 257 >
                     { Result-Code }
                      { Origin-Host }
                      { Origin-Realm }
                   1* { Host-IP-Address }
                      { Vendor-Id }
                      { Product-Name }
                      [ Origin-State-Id ]
                      [ Error-Message ]
                    * [ Failed-AVP ]
                    * [ Supported-Vendor-Id ]
                    * [ Auth-Application-Id ]
                    * [ Inband-Security-Id ]
                    * [ Acct-Application-Id ]
                    * [ Vendor-Specific-Application-Id ]
                      [ Firmware-Revision ]
                    * [ AVP ]
    */

   std::auto_ptr<DiameterMsg> msg(new DiameterMsg);
   AssembleCE(*msg, false);

   DiameterUInt32AvpWidget resultCode(DIAMETER_AVPNAME_RESULTCODE, rcode);
   msg->acl.add(resultCode());

   DiameterMsgResultCode::RCODE interpretedRcode = 
       DiameterMsgResultCode::InterpretedResultCode(rcode);
   if ((message.length() > 0) && 
       (interpretedRcode != DiameterMsgResultCode::RCODE_NOT_PRESENT) &&
       (interpretedRcode != DiameterMsgResultCode::RCODE_SUCCESS)) {
       DiameterUtf8AvpWidget errorMsg(DIAMETER_AVPNAME_ERRORMESSAGE);
       errorMsg.Get() = message.data();
       msg->acl.add(errorMsg());
   }

   // check resulting state to determine
   // which IO to use
   Diameter_IO_Base *io = (state == DIAMETER_PEER_ST_I_OPEN) ?
                      m_Data.m_IOInitiator.get() :
                      m_Data.m_IOResponder.get();
   if (m_TxMsgCollector.Send(msg, io, true) == 0) {
       AAA_LOG((LM_INFO, "(%P|%t) Sent CEA: rcode=%d\n",
                 rcode));
   }
}

void DiameterPeerStateMachine::SendDWR()
{
    /*
       5.5.1.  Device-Watchdog-Request

          The Device-Watchdog-Request (DWR), indicated by the Command-Code set
          to 280 and the Command Flags' 'R' bit set, is sent to a peer when no
          traffic has been exchanged between two peers (see Section 5.5.3).
          Upon detection of a transport failure, this message MUST NOT be sent
          to an alternate peer.

          Message Format

             <DWR>  ::= < Diameter Header: 280, REQ >
                        { Origin-Host }
                        { Origin-Realm }
                        [ Origin-State-Id ]
    */
   std::auto_ptr<DiameterMsg> msg(new DiameterMsg);
   AssembleDW(*msg);

   // check resulting state to determine
   // which IO to use
   Diameter_IO_Base *io = (state == DIAMETER_PEER_ST_I_OPEN) ?
                      m_Data.m_IOInitiator.get() :
                      m_Data.m_IOResponder.get();
   if (m_TxMsgCollector.Send(msg, io, true) < 0) {
       AAA_LOG((LM_INFO, "(%P|%t) Failed sending DWR\n"));
   }
}

void DiameterPeerStateMachine::SendDWA(diameter_unsigned32_t rcode,
                                   std::string &message)
{
    /*
       5.5.2.  Device-Watchdog-Answer

          The Device-Watchdog-Answer (DWA), indicated by the Command-Code set
          to 280 and the Command Flags' 'R' bit cleared, is sent as a response
          to the Device-Watchdog-Request message.

          Message Format

             <DWA>  ::= < Diameter Header: 280 >
                        { Result-Code }
                        { Origin-Host }
                        { Origin-Realm }
                        [ Error-Message ]
                      * [ Failed-AVP ]
                        [ Original-State-Id ]
    */

   std::auto_ptr<DiameterMsg> msg(new DiameterMsg);
   AssembleDW(*msg, false);

   DiameterUInt32AvpWidget resultCode(DIAMETER_AVPNAME_RESULTCODE, rcode);
   msg->acl.add(resultCode());

   if ((message.length() > 0) && 
       (DiameterMsgResultCode::InterpretedResultCode(rcode) == DiameterMsgResultCode::RCODE_SUCCESS)) {
       DiameterUtf8AvpWidget errorMsg(DIAMETER_AVPNAME_ERRORMESSAGE);
       errorMsg.Get() = message.data();
       msg->acl.add(errorMsg());
   }

   // check resulting state to determine
   // which IO to use
   Diameter_IO_Base *io = (state == DIAMETER_PEER_ST_I_OPEN) ?
                      m_Data.m_IOInitiator.get() :
                      m_Data.m_IOResponder.get();
   if (m_TxMsgCollector.Send(msg, io, true) < 0) {
       AAA_LOG((LM_INFO, "(%P|%t) Failed sending DWA: rcode=%d\n",
                 rcode));
   }
}

void DiameterPeerStateMachine::SendDPR(bool initiator)
{
    /*
      5.4.1.  Disconnect-Peer-Request

         The Disconnect-Peer-Request (DPR), indicated by the Command-Code set
         to 282 and the Command Flags' 'R' bit set, is sent to a peer to
         inform its intentions to shutdown the transport connection.  Upon
         detection of a transport failure, this message MUST NOT be sent to an
         alternate peer.

         Message Format

            <DPR>  ::= < Diameter Header: 282, REQ >
                       { Origin-Host }
                       { Origin-Realm }
                       { Disconnect-Cause }
    */
   std::auto_ptr<DiameterMsg> msg(new DiameterMsg);
   AssembleDP(*msg);

   DiameterUInt32AvpWidget disCause(DIAMETER_AVPNAME_DISCONNECT_CAUSE);
   disCause.Get() = diameter_unsigned32_t(m_Data.m_DisconnectCause);
   msg->acl.add(disCause());

   Diameter_IO_Base *io = (initiator) ?
                      m_Data.m_IOInitiator.get() :
                      m_Data.m_IOResponder.get();
   if (m_TxMsgCollector.Send(msg, io, true) < 0) {
       AAA_LOG((LM_INFO, "(%P|%t) Failed sending Disconnect\n"));
   }
}

void DiameterPeerStateMachine::SendDPA(bool initiator,
                                       diameter_unsigned32_t rcode)
{
    /*
      5.4.2.  Disconnect-Peer-Answer

         The Disconnect-Peer-Answer (DPA), indicated by the Command-Code set
         to 282 and the Command Flags' 'R' bit cleared, is sent as a response
         to the Disconnect-Peer-Request message.  Upon receipt of this
         message, the transport connection is shutdown.

         Message Format

            <DPA>  ::= < Diameter Header: 282 >
                       { Result-Code }
                       { Origin-Host }
                       { Origin-Realm }
                       [ Error-Message ]
                     * [ Failed-AVP ]

    */
   std::auto_ptr<DiameterMsg> msg(new DiameterMsg);
   AssembleDP(*msg, false);

   DiameterUInt32AvpWidget resultCode(DIAMETER_AVPNAME_RESULTCODE, rcode);
   msg->acl.add(resultCode());

   Diameter_IO_Base *io = (initiator) ?
                      m_Data.m_IOInitiator.get() :
                      m_Data.m_IOResponder.get();
   if (m_TxMsgCollector.Send(msg, io, true) < 0) {
       AAA_LOG((LM_INFO, "(%P|%t) Failed sending DWA: rcode=%d\n",
                 rcode));
   }
}

void DiameterPeerStateMachine::Elect()
{
   /*
      5.6.4.  The Election Process

         The election is performed on the responder.  The responder compares
         the Origin-Host received in the CER sent by its peer with its own
         Origin-Host.  If the local Diameter entity's Origin-Host is higher
         than the peer's, a Win-Election event is issued locally.

         The comparison proceeds by considering the shorter OctetString to be
         padded with zeros so that it length is the same as the length of the
         longer, then performing an octet-by-octet unsigned comparison with
         the first octet being most significant.  Any remaining octets are
         assumed to have value 0x80.
   */
   AAA_LOG((LM_DEBUG, "(%P|%t) Election occurring ...\n"));
   std::string localHost = DIAMETER_CFG_TRANSPORT()->identity;
   std::string peerHost = m_Data.m_PeerCapabilities.m_Host;
   if (localHost.length() < peerHost.length()) {
       localHost += std::string(peerHost.length() - localHost.length(), char(0x08));
   }
   else if (localHost.length() > peerHost.length()) {
       peerHost += std::string(localHost.length() - peerHost.length(), char(0x08));
   }
   else if (localHost == peerHost) {
       // advertising the same hostname
       Cleanup();
       return;
   }
   for (unsigned int i=0; i<localHost.size(); i++) {
       if (localHost[i] == peerHost[i]) {
           continue;
       }
       else if (localHost[i] > peerHost[i]) {
           Notify(DIAMETER_PEER_EV_WIN_ELECTION);
           AAA_LOG((LM_INFO, "(%P|%t) ***** Local peer wins election *****\n"));
           return;
       }
       else {
           break;
       }
   }
   AAA_LOG((LM_INFO, "(%P|%t) ***** Peer (%s) wins election *****\n",
              peerHost.c_str()));
}

void DiameterPeerStateMachine::Cleanup(unsigned int flags)
{
   DiameterPeerCapabilities &cap = m_Data.m_PeerCapabilities;

   cap.m_Host = "";
   cap.m_Realm = "";

   while (! cap.m_HostIpLst.empty()) {
       diameter_address_t *ip = cap.m_HostIpLst.front();
       cap.m_HostIpLst.pop_front();
       delete ip;
   }

   cap.m_VendorId = 0;
   cap.m_ProductName = "";
   cap.m_OriginStateId = 0;

   DiameterApplicationIdLst *idList[] = {
       &cap.m_SupportedVendorIdLst,
       &cap.m_AuthAppIdLst,
       &cap.m_AcctAppIdLst
   };
   for (unsigned int i=0;
        i<sizeof(idList)/sizeof(DiameterApplicationIdLst*);
        i++) {
       while (! idList[i]->empty()) {
           idList[i]->pop_front();
       }
   }

   while (! cap.m_VendorSpecificId.empty()) {
       DiameterVendorSpecificIdLst::iterator x =
           cap.m_VendorSpecificId.begin();
       cap.m_VendorSpecificId.pop_front();
   }

   cap.m_InbandSecurityId = 0;
   cap.m_FirmwareRevision = 0;

   StopReConnect();
   CancelTimer(DIAMETER_PEER_EV_TIMEOUT);
   CancelTimer(DIAMETER_PEER_EV_WATCHDOG);
   CancelTimer(DIAMETER_PEER_CONNECT_ATTEMPT_TOUT);

   if (flags & CLEANUP_FSM) {
       AAA_StateMachineWithTimer<DiameterPeerStateMachine>::Stop();
   }

   if (flags & CLEANUP_IO_R) {
       DIAMETER_IO_GC().ScheduleForDeletion(m_Data.m_IOResponder);
   }
   if (flags & CLEANUP_IO_I) {
       DIAMETER_IO_GC().ScheduleForDeletion(m_Data.m_IOInitiator);
   }

   if (flags & CLEANUP_FSM) {
       m_CurrentPeerEventParam->m_Msg.reset();
       DIAMETER_IO_GC().ScheduleForDeletion(m_CurrentPeerEventParam->m_IO);
       AAA_StateMachineWithTimer<DiameterPeerStateMachine>::Start();
   }

   m_CleanupSignal.signal();
}

void DiameterPeerStateMachine::DumpPeerCapabilities()
{
   DiameterPeerCapabilities &cap = m_Data.m_PeerCapabilities;

   AAA_LOG((LM_INFO, "(%P|%t) Peer Capabilities\n"));
   AAA_LOG((LM_INFO, "(%P|%t)             Hostname : %s\n", cap.m_Host.c_str()));
   AAA_LOG((LM_INFO, "(%P|%t)                Realm : %s\n", cap.m_Realm.c_str()));

   DiameterHostIpLst::iterator x = cap.m_HostIpLst.begin();
   for (; x != cap.m_HostIpLst.end(); x++) {
       AAA_LOG((LM_INFO, "(%P|%t)              Host IP : type=%d, %s\n", (*x)->type,
                  inet_ntoa(*((struct in_addr*)(*x)->value.c_str()))));
   }

   AAA_LOG((LM_INFO, "(%P|%t)             VendorId : %d\n", cap.m_VendorId));
   AAA_LOG((LM_INFO, "(%P|%t)         Product Name : %s\n", cap.m_ProductName.c_str()));
   AAA_LOG((LM_INFO, "(%P|%t)           Orig State : %d\n", cap.m_OriginStateId));

   DiameterApplicationIdLst *idList[] = {
       &cap.m_SupportedVendorIdLst,
       &cap.m_AuthAppIdLst,
       &cap.m_AcctAppIdLst
   };
   char *label[] = { "Supported Vendor Id",
                     "Auth Application Id",
                     "Acct Application Id" };
   for (unsigned int i=0;
        i < sizeof(idList)/sizeof(DiameterApplicationIdLst*);
        i++) {
       DiameterApplicationIdLst::iterator x = idList[i]->begin();
       for (; x != idList[i]->end(); x++) {
           if ((i > 0) && ((*x) == DIAMETER_RELAY_APPLICATION_ID)) {
               AAA_LOG((LM_INFO, "(%P|%t)  %s : Relay\n",
                       label[i]));
           }
           else {
               AAA_LOG((LM_INFO, "(%P|%t)  %s : %d\n",
                       label[i], *x));
           }
       }
   }

   DiameterVendorSpecificIdLst::iterator y = cap.m_VendorSpecificId.begin();
   for (; y != cap.m_VendorSpecificId.end(); y++) {
       AAA_LOG((LM_INFO, "(%P|%t)  Vendor Specific Id : "));
        if ((*y).vendorId > 0) {
            AAA_LOG((LM_INFO, "(%P|%t)      Vendor=%d, ", (*y)));
        }
        else {
            AAA_LOG((LM_INFO, "(%P|%t)      Vendor=--- "));
        }
       if ((*y).authAppId > 0) {
           if ((*y).authAppId == DIAMETER_RELAY_APPLICATION_ID) {
               AAA_LOG((LM_INFO, " Auth=Relay"));
           }
           else {
               AAA_LOG((LM_INFO, " Auth=%d ", (*y).authAppId));
           }
       }
       if ((*y).acctAppId > 0) {
           if ((*y).authAppId == DIAMETER_RELAY_APPLICATION_ID) {
               AAA_LOG((LM_INFO, " Acct=Relay"));
           }
           else {
               AAA_LOG((LM_INFO, " Acct=%d ", (*y).acctAppId));
           }
       }
       AAA_LOG((LM_INFO, "%s\n", (((*y).authAppId == 0) && ((*y).acctAppId == 0)) ? "---" : ""));
   }

   AAA_LOG((LM_INFO, "(%P|%t)           Inband Sec : %d\n", cap.m_InbandSecurityId));
   AAA_LOG((LM_INFO, "(%P|%t)         Firmware Ver : %d\n", cap.m_FirmwareRevision));
}

bool DiameterPeerStateMachine::ValidatePeer(diameter_unsigned32_t &rcode,
                                        std::string &message)
{
   /*
      5.3.  Capabilities Exchange

      When two Diameter peers establish a transport connection, they MUST
      exchange the Capabilities Exchange messages, as specified in the peer
      state machine (see Section 5.6).  This message allows the discovery
      of a peer's identity and its capabilities (protocol version number,
      supported Diameter applications, security mechanisms, etc.)

      The receiver only issues commands to its peers that have advertised
      support for the Diameter application that defines the command.  A
      Diameter node MUST cache the supported applications in order to
      ensure that unrecognized commands and/or AVPs are not unnecessarily
      sent to a peer.

      A receiver of a Capabilities-Exchange-Req (CER) message that does not
      have any applications in common with the sender MUST return a
      Capabilities-Exchange-Answer (CEA) with the Result-Code AVP set to
      AAA_NO_COMMON_APPLICATION, and SHOULD disconnect the transport
      layer connection.  Note that receiving a CER or CEA from a peer
      advertising itself as a Relay (see Section 2.4) MUST be interpreted
      as having common applications with the peer.
   */
   bool found = false;
   DiameterPeerCapabilities &cap = m_Data.m_PeerCapabilities;

   DiameterApplicationIdLst *localIdList[] = {
       &DIAMETER_CFG_GENERAL()->authAppIdLst,
       &DIAMETER_CFG_GENERAL()->acctAppIdLst
   };
   DiameterApplicationIdLst *idList[] = {
       &cap.m_AuthAppIdLst,
       &cap.m_AcctAppIdLst
   };
   for (unsigned int i=0;
        i < sizeof(idList)/sizeof(DiameterApplicationIdLst*);
        i++) {
       DiameterApplicationIdLst::iterator x = idList[i]->begin();
       for (; x != idList[i]->end(); x++) {
           if (AAA_ApplicationIdLookup::Find(*x, localIdList,
               sizeof(localIdList)/sizeof(DiameterApplicationIdLst*))) {
               found = true;
               break;
           }
           if (AAA_ApplicationIdLookup::Find
               (*x, DIAMETER_CFG_GENERAL()->vendorSpecificId)) {
               found = true;
               break;
           }
       }
       if (found) {
           break;
       }
   }
   if (! found) {
       DiameterVendorSpecificIdLst::iterator i = cap.m_VendorSpecificId.begin();
       for (; i != cap.m_VendorSpecificId.end(); i++) {
           if (AAA_ApplicationIdLookup::Find((*i).authAppId, localIdList,
                sizeof(localIdList)/sizeof(DiameterApplicationIdLst*))) {
               found = true;
               break;
           }
           if (AAA_ApplicationIdLookup::Find((*i).acctAppId, localIdList,
                sizeof(localIdList)/sizeof(DiameterApplicationIdLst*))) {
               found = true;
               break;
           }
           if (AAA_ApplicationIdLookup::Find
               ((*i).authAppId, DIAMETER_CFG_GENERAL()->vendorSpecificId)) {
               found = true;
               break;
           }
           if (AAA_ApplicationIdLookup::Find
               ((*i).acctAppId, DIAMETER_CFG_GENERAL()->vendorSpecificId)) {
               found = true;
               break;
           }
       }
   }
   if (! found) {
       rcode = AAA_NO_COMMON_APPLICATION;
       message = "No common application id";
       return false;
   }

   /*
      Similarly, a receiver of a Capabilities-Exchange-Req (CER) message
      that does not have any security mechanisms in common with the sender
      MUST return a Capabilities-Exchange-Answer (CEA) with the Result-Code
      AVP set to AAA_NO_COMMON_SECURITY, and SHOULD disconnect the
      transport layer connection.
   */
   // INTEROP FIXES
   if (m_Data.m_TLS != cap.m_InbandSecurityId) {
       rcode = AAA_NO_COMMON_SECURITY;
       message = "No matching in-band security id";
       return false;
   }

   /*
       The CER and CEA messages MUST NOT be proxied, redirected or relayed.

       Since the CER/CEA messages cannot be proxied, it is still possible
       that an upstream agent receives a message for which it has no
       available peers to handle the application that corresponds to the
       Command-Code.  In such instances, the 'E' bit is set in the answer

       message (see Section 7.) with the Result-Code AVP set to
       AAA_UNABLE_TO_DELIVER to inform the downstream to take action
       (e.g., re-routing request to an alternate peer).

       With the exception of the Capabilities-Exchange-Request message, a
       message of type Request that includes the Auth-Application-Id or
       Acct-Application-Id AVPs, or a message with an application-specific
       command code, MAY only be forwarded to a host that has explicitly
       advertised support for the application (or has advertised the Relay
       Application Identifier).
   */
   rcode = AAA_SUCCESS;
   message = "Capabilities negotiation completed successfully";
   return true;
}

void DiameterPeerStateMachine::MsgIdTxMessage(DiameterMsg &msg)
{
   if (msg.hdr.flags.r) {
       diameter_unsigned32_t hopId = DIAMETER_HOPBYHOP_GEN()->Get();
       diameter_unsigned32_t endId = DIAMETER_ENDTOEND_GEN()->Get();

       m_Data.m_PeerCapabilities.m_MsgId.m_LastTxHopId.Add(hopId, hopId);
       m_Data.m_PeerCapabilities.m_MsgId.m_LastTxEndId.Add(endId, endId);

       msg.hdr.hh = hopId;
       msg.hdr.ee = endId;
   }
   else {
       if (! m_Data.m_PeerCapabilities.m_MsgId.m_LastRxHopId.IsEmpty()) {
           msg.hdr.hh = m_Data.m_PeerCapabilities.m_MsgId.m_LastRxHopId.Dequeue();
       }
       else {
           AAA_LOG((LM_ERROR, "(%P|%t) Sending a peer message but Hop Id queue is empty\n"));
           msg.hdr.hh = 0;
       }
       if (! m_Data.m_PeerCapabilities.m_MsgId.m_LastRxEndId.IsEmpty()) {
           msg.hdr.ee = m_Data.m_PeerCapabilities.m_MsgId.m_LastRxEndId.Dequeue();
       }
       else {
           AAA_LOG((LM_ERROR, "(%P|%t) Sending a peer message but End Id queue is empty\n"));
           msg.hdr.ee = 0;
       }
   }
}

bool DiameterPeerStateMachine::MsgIdRxMessage(DiameterMsg &msg)
{
   if (msg.hdr.flags.r) {
       m_Data.m_PeerCapabilities.m_MsgId.m_LastRxHopId.Enqueue(msg.hdr.hh);
       m_Data.m_PeerCapabilities.m_MsgId.m_LastRxEndId.Enqueue(msg.hdr.ee);
       return (true);
   }
   else {
       AAA_IterActionDelete<diameter_unsigned32_t> delAct;
       if (m_Data.m_PeerCapabilities.m_MsgId.m_LastTxHopId.Remove(msg.hdr.hh, delAct) &&
           m_Data.m_PeerCapabilities.m_MsgId.m_LastTxEndId.Remove(msg.hdr.ee, delAct)) {
           return (true);
       }
   }
   return (false);
}

void DiameterPeerStateMachine::DoReConnect()
{
   if ((unsigned int)m_ReconnectAttempt < DIAMETER_CFG_TRANSPORT()->reconnect_max) {
       if (DIAMETER_CFG_TRANSPORT()->reconnect_interval > 0) {
           m_ReconnectAttempt ++;
           ScheduleTimer(DIAMETER_PEER_EV_CONN_RETRY,
                         DIAMETER_CFG_TRANSPORT()->reconnect_interval,
                         0,
                         DIAMETER_PEER_EV_CONN_RETRY);
       }
   }
   else {
       m_ReconnectAttempt = 0;
   }
}

void DiameterPeerStateMachine::StopReConnect()
{
   CancelTimer(DIAMETER_PEER_EV_CONN_RETRY);
}

int DiameterPeerStateMachine::TransportProtocolInUse()
{
   return (m_Data.m_IOInitiator.get() != NULL) ?
           m_Data.m_IOInitiator->TransportProtocolInUse() :
           m_Data.m_IOResponder->TransportProtocolInUse();
}

