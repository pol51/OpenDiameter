
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

#include "aaa_data_defs.h"
#include "aaa_peer_fsm.h"
#include "aaa_route_msg_router.h"

#define AAA_PEER_CONNECT_ATTEMPT_TOUT 30

AAA_PeerStateTable AAA_PeerStateMachine::m_StateTable;

class AAA_ApplicationIdLookup
{
   public:
      static inline bool Find(diameter_unsigned32_t id,
                              AAA_ApplicationIdLst &lst) {
          AAA_ApplicationIdLst::iterator i = lst.begin();
          for (; i!= lst.end(); i++) {
              if ((*i == AAA_RELAY_APPLICATION_ID) ||
                  (*i == id)) {
                  return true;
              }
          }
          return false;
      }
      static inline bool Find(diameter_unsigned32_t id,
                              AAA_ApplicationIdLst *lst[],
                              int count) {
          for (int i=0; i<count; i++) {
              if (Find(id, *(lst[i]))) {
                  return true;
              }              
          }
          return false;
      }
      static inline bool Find(diameter_unsigned32_t id,
                              AAA_VendorSpecificIdLst &lst) {
          AAA_VendorSpecificIdLst::iterator i = lst.begin();
          for (; i != lst.end(); i++) {
              if (((*i).authAppId == id) ||
                  ((*i).acctAppId == id) ||
                  ((*i).authAppId == AAA_RELAY_APPLICATION_ID) ||
                  ((*i).acctAppId == AAA_RELAY_APPLICATION_ID)) {
                  return true;
              }
          }
          return false;
      }
};

void AAA_PeerR_ISendConnReq::operator()(AAA_PeerStateMachine &fsm)
{
    fsm.ScheduleTimer(AAA_PEER_EV_TIMEOUT,
                      AAA_PEER_CONNECT_ATTEMPT_TOUT,
                      0,
                      AAA_PEER_EV_TIMEOUT);
}
    
void AAA_PeerR_AcceptSendCEA::operator()(AAA_PeerStateMachine &fsm)
{
    fsm.PeerData().m_IOResponder = fsm.m_CurrentPeerEventParam->m_IO;    
    std::auto_ptr<DiameterMsg> cer = fsm.m_CurrentPeerEventParam->m_Msg;

    fsm.DisassembleCE(*cer);

    std::string message;
    diameter_unsigned32_t rcode;
    bool valid = fsm.ValidatePeer(rcode, message);
    fsm.SendCEA(rcode, message);

    if (! valid) {
        AAA_LOG(LM_INFO, "(%P|%t) %s in connection attempt, discarding\n",
                   message.data());
        fsm.Cleanup();
    }
    else {
        fsm.CancelTimer(AAA_PEER_EV_TIMEOUT);
        fsm.ScheduleTimer(AAA_PEER_EV_WATCHDOG,
                          AAA_CFG_TRANSPORT()->watchdog_timeout,
                          0,
                          AAA_PEER_EV_WATCHDOG);                          
        fsm.PeerFsmConnected();
    }
}

void AAA_PeerI_SendCER::operator()(AAA_PeerStateMachine &fsm)
{
    AAA_LOG(LM_DEBUG, "(%P|%t) Connection attempt accepted\n");
    fsm.PeerData().m_IOInitiator = fsm.m_CurrentPeerEventParam->m_IO;
    fsm.SendCER();
}

void AAA_Peer_ConnNack::operator()(AAA_PeerStateMachine &fsm)
{
    fsm.Cleanup();
    if (AAA_CFG_TRANSPORT()->retry_interval > 0) {
        fsm.ScheduleTimer(AAA_PEER_EV_CONN_RETRY,
                          AAA_CFG_TRANSPORT()->retry_interval,
                          0,
                          AAA_PEER_EV_CONN_RETRY);
    }
    fsm.PeerFsmError(AAA_UNABLE_TO_COMPLY);
}

void AAA_Peer_Cleanup::operator()(AAA_PeerStateMachine &fsm)
{
    fsm.Cleanup();
}

void AAA_Peer_Retry::operator()(AAA_PeerStateMachine &fsm)
{
    AAA_LOG(LM_INFO,
            "(%P|%t) Retrying peer connection\n");

    fsm.CancelTimer(AAA_PEER_EV_CONN_RETRY);
    reinterpret_cast<AAA_PeerEntry*>(&fsm)->Start();
}

void AAA_PeerR_Accept::operator()(AAA_PeerStateMachine &fsm)
{
    fsm.PeerData().m_IOResponder = fsm.m_CurrentPeerEventParam->m_IO;
    std::auto_ptr<DiameterMsg> cer = fsm.m_CurrentPeerEventParam->m_Msg;
   
    fsm.DisassembleCE(*cer);
    
    std::string message;
    diameter_unsigned32_t rcode;
    if (! fsm.ValidatePeer(rcode, message)) {
        AAA_LOG(LM_INFO, "(%P|%t) %s during election, discarding\n",
                   message.data());
        fsm.Cleanup();
    }
    else {
        AAA_LOG(LM_DEBUG, "(%P|%t) *** Peer capabilities accepted ***\n");
    }
}

void AAA_Peer_Error::operator()(AAA_PeerStateMachine &fsm)
{
    AAA_LOG(LM_INFO,
               "(%P|%t) Timeout occurred or non-CEA message received\n");
    fsm.Cleanup();

    if (AAA_CFG_TRANSPORT()->retry_interval > 0) {
        fsm.ScheduleTimer(AAA_PEER_EV_CONN_RETRY,
                          AAA_CFG_TRANSPORT()->retry_interval,
                          0,
                          AAA_PEER_EV_CONN_RETRY);
    }
    fsm.PeerFsmError(AAA_LIMITED_SUCCESS);
}

void AAA_Peer_ProcessCEA::operator()(AAA_PeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> cea = fsm.m_CurrentPeerEventParam->m_Msg;
    
    DiameterMsgResultCode rcode(*cea);
    if (rcode.InterpretedResultCode() ==
        DiameterMsgResultCode::RCODE_SUCCESS) {
        fsm.DisassembleCE(*cea);
        fsm.CancelTimer(AAA_PEER_EV_TIMEOUT);
        fsm.ScheduleTimer(AAA_PEER_EV_WATCHDOG,
                          AAA_CFG_TRANSPORT()->watchdog_timeout,
                          0,
                          AAA_PEER_EV_WATCHDOG);
        AAA_LOG(LM_DEBUG, "(%P|%t) *** Local capabilities accepted by peer ***\n");
        fsm.PeerFsmConnected();
    }
    else {
       DiameterUtf8AvpContainerWidget errorMsg(cea->acl);
       diameter_utf8string_t *strMsg = errorMsg.GetAvp
           (AAA_AVPNAME_ERRORMESSAGE);
       if (strMsg) {
           AAA_LOG(LM_INFO,
                 "(%P|%t) Peer returned an error on CEA: %s\n",
                      strMsg->data());
       }
       fsm.Cleanup();       
    }
}

void AAA_PeerR_AcceptElect::operator()(AAA_PeerStateMachine &fsm)
{
    fsm.PeerData().m_IOResponder = fsm.m_CurrentPeerEventParam->m_IO;
    std::auto_ptr<DiameterMsg> cer = fsm.m_CurrentPeerEventParam->m_Msg;
   
    fsm.DisassembleCE(*cer);
    
    std::string message;
    diameter_unsigned32_t rcode;
    if (! fsm.ValidatePeer(rcode, message)) {
        AAA_LOG(LM_INFO, "(%P|%t) %s during election, discarding\n",
                   message.data());
        fsm.Cleanup(AAA_PeerStateMachine::CLEANUP_ALL &
                    ~AAA_PeerStateMachine::CLEANUP_IO_I);
    }
    else {
        fsm.Elect();
    }
}

void AAA_PeerI_SendCERElect::operator()(AAA_PeerStateMachine &fsm)
{
    fsm.PeerData().m_IOInitiator = fsm.m_CurrentPeerEventParam->m_IO;    
    fsm.SendCER();
    fsm.Elect();
}

void AAA_PeerR_SendCEA::operator()(AAA_PeerStateMachine &fsm)
{
    fsm.PeerData().m_IOResponder = fsm.m_CurrentPeerEventParam->m_IO;    
    std::string message = "Capabilities negotiation completed successfully (win-election)";
    fsm.SendCEA(AAA_SUCCESS, message);
    fsm.CancelTimer(AAA_PEER_EV_TIMEOUT);
    fsm.ScheduleTimer(AAA_PEER_EV_WATCHDOG,
                      AAA_CFG_TRANSPORT()->watchdog_timeout,
                      0,
                      AAA_PEER_EV_WATCHDOG);
    AAA_LOG(LM_DEBUG, "(%P|%t) %s\n", message.data());
    fsm.PeerFsmConnected();
}

void AAA_PeerR_SendCEAOpen::operator()(AAA_PeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> cer = fsm.m_CurrentPeerEventParam->m_Msg;
   
    fsm.DisassembleCE(*cer);
    
    std::string message;
    diameter_unsigned32_t rcode;
    if (! fsm.ValidatePeer(rcode, message)) {
        AAA_LOG(LM_INFO, "(%P|%t) %s during cap re-negotiation\n",
                   message.data());
    }
    else {
        AAA_LOG(LM_DEBUG, "(%P|%t) *** Peer capabilities accepted ***\n");
    }
    fsm.SendCEA(rcode, message);
}

void AAA_PeerR_DisconnectResp::operator()(AAA_PeerStateMachine &fsm)
{
    AAA_LOG(LM_DEBUG, "(%P|%t) Disconnecting responder\n");
    fsm.PeerData().m_IOResponder.reset();
}

void AAA_PeerR_DisconnectIOpen::operator()(AAA_PeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> cea = fsm.m_CurrentPeerEventParam->m_Msg;
    
    DiameterMsgResultCode rcode(*cea);
    if (rcode.InterpretedResultCode() ==
        DiameterMsgResultCode::RCODE_SUCCESS) {
        fsm.DisassembleCE(*cea);
        fsm.CancelTimer(AAA_PEER_EV_TIMEOUT);
        fsm.ScheduleTimer(AAA_PEER_EV_WATCHDOG,
                          AAA_CFG_TRANSPORT()->watchdog_timeout,
                          0,
                          AAA_PEER_EV_WATCHDOG);
        fsm.PeerData().m_IOResponder.reset();
        AAA_LOG(LM_DEBUG, "(%P|%t) *** Initiator capabilities accepted ***\n");
        fsm.PeerFsmConnected();
    }
    else {
       DiameterUtf8AvpContainerWidget errorMsg(cea->acl);
       diameter_utf8string_t *strMsg = errorMsg.GetAvp
           (AAA_AVPNAME_ERRORMESSAGE);
       if (strMsg) {
           AAA_LOG(LM_INFO,
                 "(%P|%t) Peer returned an error on CEA: %s\n",
                      strMsg->data());
       }
       fsm.Cleanup();       
    }
}

void AAA_PeerR_Reject::operator()(AAA_PeerStateMachine &fsm)
{
    AAA_LOG(LM_DEBUG, "(%P|%t) Responder connection attempt rejected\n");
    std::auto_ptr<AAA_IO_Base> io = fsm.m_CurrentPeerEventParam->m_IO;
    std::auto_ptr<DiameterMsg> msg = fsm.m_CurrentPeerEventParam->m_Msg;
    io.reset();
}

void AAA_PeerI_DisconnectSendCEA::operator()(AAA_PeerStateMachine &fsm)
{
    std::string message = "Capabilities negotiation completed successfully (win-election)";
    fsm.PeerData().m_IOInitiator.reset();
    fsm.SendCEA(AAA_SUCCESS, message);
    fsm.CancelTimer(AAA_PEER_EV_TIMEOUT);
    fsm.ScheduleTimer(AAA_PEER_EV_WATCHDOG,
                      AAA_CFG_TRANSPORT()->watchdog_timeout,
                      0,
                      AAA_PEER_EV_WATCHDOG);
    AAA_LOG(LM_DEBUG, "(%P|%t) %s\n", message.data());
    fsm.PeerFsmConnected();
}

void AAA_PeerR_SendMessage::operator()(AAA_PeerStateMachine &fsm)
{
#if ASYNC_SEND
    boost::shared_ptr<DiameterMsg> msg = fsm.DequeueSendMsg();
    if (fsm.Send(*msg, fsm.PeerData().m_IOResponder.get()) < 0) {
        AAA_LOG(LM_INFO, "(%P|%t) Error sending message: %d\n",
                   msg->hdr.code);
    }
#endif
}

void AAA_Peer_Process::operator()(AAA_PeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> msg = fsm.m_CurrentPeerEventParam->m_Msg;
    AAA_MsgQuery query(*msg);
    if (query.IsRequest()) {
        AAA_MSG_ROUTER()->RequestMsg(msg,
               reinterpret_cast<AAA_PeerEntry*>(&fsm));
    }
    else {
        AAA_MSG_ROUTER()->AnswerMsg(msg,
               reinterpret_cast<AAA_PeerEntry*>(&fsm));
    }
}

void AAA_PeerProcessDWRSendDWA::operator()(AAA_PeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> dwr = fsm.m_CurrentPeerEventParam->m_Msg;
    fsm.DisassembleDW(*dwr);

    // TBD: do failover here
    
    std::string message = "Successful device watchdog";
    fsm.SendDWA(AAA_SUCCESS, message);
}

void AAA_Peer_ProcessDWA::operator()(AAA_PeerStateMachine &fsm)
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
            (AAA_AVPNAME_ERRORMESSAGE);
        if (strMsg) {
            AAA_LOG(LM_INFO,
                  "(%P|%t) Peer returned an error on Watchdog: %s, closing peer\n",
                       strMsg->data());
        }
        fsm.Cleanup();       
    }
}

void AAA_PeerI_SendMessage::operator()(AAA_PeerStateMachine &fsm)
{
#if ASYNC_SEND
    boost::shared_ptr<DiameterMsg> msg = fsm.DequeueSendMsg();
    if (fsm.Send(*msg, fsm.PeerData().m_IOInitiator.get()) < 0) {
        AAA_LOG(LM_INFO, "(%P|%t) Error sending message: %d\n",
                   msg->hdr.code);
    }    
#endif
}

void AAA_PeerI_SendCEA::operator()(AAA_PeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> cer = fsm.m_CurrentPeerEventParam->m_Msg;
   
    fsm.DisassembleCE(*cer);
    
    std::string message;
    diameter_unsigned32_t rcode;
    if (! fsm.ValidatePeer(rcode, message)) {
        AAA_LOG(LM_INFO, "(%P|%t) %s during cap re-negotiation\n",
                   message.data());
    }
    fsm.SendCEA(rcode, message);
}

void AAA_PeerI_SendDPR::operator()(AAA_PeerStateMachine &fsm)
{
    fsm.CancelTimer(AAA_PEER_EV_WATCHDOG);
    fsm.ScheduleTimer(AAA_PEER_EV_TIMEOUT,
                      AAA_PEER_CONNECT_ATTEMPT_TOUT,
                      0,
                      AAA_PEER_EV_TIMEOUT);
    fsm.SendDPR(true);
}

void AAA_PeerR_SendDPR::operator()(AAA_PeerStateMachine &fsm)
{
    fsm.CancelTimer(AAA_PEER_EV_WATCHDOG);
    fsm.ScheduleTimer(AAA_PEER_EV_TIMEOUT,
                      AAA_PEER_CONNECT_ATTEMPT_TOUT,
                      0,
                      AAA_PEER_EV_TIMEOUT);
    fsm.SendDPR(false);
}

void AAA_PeerI_SendDPADisconnect::operator()(AAA_PeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> dpr = fsm.m_CurrentPeerEventParam->m_Msg;
    
    AAA_LOG(LM_INFO, "(%P|%t) Peer initiator requested termination, disconnecting\n");
    std::string message = "Disconnected";
    fsm.SendDPA(true, AAA_SUCCESS, message);
    
    DiameterUInt32AvpContainerWidget cause(dpr->acl);
    diameter_unsigned32_t *uint32 = cause.GetAvp(AAA_AVPNAME_DISCONNECT_CAUSE);   
    
    fsm.Cleanup();    
    fsm.PeerFsmDisconnected
         ((uint32) ? *uint32 : AAA_DISCONNECT_UNKNOWN);   
}

void AAA_PeerR_SendDPADisconnect::operator()(AAA_PeerStateMachine &fsm)
{
    std::auto_ptr<DiameterMsg> dpr = fsm.m_CurrentPeerEventParam->m_Msg;
    
    AAA_LOG(LM_INFO, "(%P|%t) Peer responder requested termination, disconnecting\n");
    std::string message = "Disconnected";
    fsm.SendDPA(false, AAA_SUCCESS, message);
    
    DiameterUInt32AvpContainerWidget cause(dpr->acl);
    diameter_unsigned32_t *uint32 = cause.GetAvp(AAA_AVPNAME_DISCONNECT_CAUSE);   
    
    fsm.Cleanup();    
    fsm.PeerFsmDisconnected
         ((uint32) ? *uint32 : AAA_DISCONNECT_UNKNOWN);   
}

void AAA_Peer_Disconnect::operator()(AAA_PeerStateMachine &fsm)
{
    AAA_LOG(LM_INFO, "(%P|%t) General disconnection\n");
    fsm.Cleanup();
    if (AAA_CFG_TRANSPORT()->retry_interval > 0) {
        fsm.ScheduleTimer(AAA_PEER_EV_CONN_RETRY,
                          AAA_CFG_TRANSPORT()->retry_interval,
                          0,
                          AAA_PEER_EV_CONN_RETRY);
    }
    
    fsm.PeerFsmDisconnected
         (AAA_DISCONNECT_TRANSPORT);
}

void AAA_Peer_DisconnectDPA::operator()(AAA_PeerStateMachine &fsm)
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
            (AAA_AVPNAME_ERRORMESSAGE);
        if (strMsg) {
            AAA_LOG(LM_INFO,
               "(%P|%t) Peer returned an error on Watchdog: %s, closing peer\n",
                    strMsg->data());
        }
    }

    fsm.PeerFsmDisconnected
        (fsm.PeerData().m_DisconnectCause);    
    fsm.Cleanup();
}

void AAA_Peer_Watchdog::operator()(AAA_PeerStateMachine &fsm)
{
    fsm.SendDWR();
}

void AAA_PeerStateMachine::AssembleCE(DiameterMsg &msg,
                                      bool request)
{
   DiameterMsgHeader &h = msg.hdr;
   ACE_OS::memset(&msg.hdr, 0, sizeof(msg.hdr));
   h.ver = DIAMETER_PROTOCOL_VERSION;
   h.length = 0;
   h.flags.r = request ? DIAMETER_FLAG_SET : DIAMETER_FLAG_CLR;
   h.flags.p = DIAMETER_FLAG_CLR;
   h.flags.e = DIAMETER_FLAG_CLR;
   h.code = AAA_MSGCODE_CAPABILITIES_EXCHG;
   h.appId = DIAMETER_BASE_APPLICATION_ID;
   MsgIdTxMessage(msg);

   DiameterIdentityAvpWidget originHost(AAA_AVPNAME_ORIGINHOST);
   DiameterIdentityAvpWidget originRealm(AAA_AVPNAME_ORIGINREALM);
   DiameterAddressAvpWidget  hostIp(AAA_AVPNAME_HOSTIP);
   DiameterUInt32AvpWidget   vendorId(AAA_AVPNAME_VENDORID);
   DiameterUtf8AvpWidget     product(AAA_AVPNAME_PRODUCTNAME);
   DiameterUInt32AvpWidget   originStateId(AAA_AVPNAME_ORIGINSTATEID);
   DiameterUInt32AvpWidget   supportedVendorId(AAA_AVPNAME_SUPPORTEDVENDORID);
   DiameterUInt32AvpWidget   authId(AAA_AVPNAME_AUTHAPPID);
   DiameterUInt32AvpWidget   acctId(AAA_AVPNAME_ACCTAPPID);
   DiameterGroupedAvpWidget  vendorSpecificId(AAA_AVPNAME_VENDORAPPID);
   DiameterUInt32AvpWidget   firmware(AAA_AVPNAME_FIRMWAREREV);
   DiameterUInt32AvpWidget   inbandSecId(AAA_AVPNAME_INBANDSECID);

   originHost.Get() = AAA_CFG_TRANSPORT()->identity;
   originRealm.Get() = AAA_CFG_TRANSPORT()->realm;
   firmware.Get() = AAA_CFG_GENERAL()->version;
   inbandSecId.Get() = m_Data.m_TLS;
  
   // Host-IP-Address
   AAA_IpAddress tool;
   if (AAA_CFG_TRANSPORT()->advertised_host_ip.size() > 0) {
       std::list<std::string>::iterator i = 
           AAA_CFG_TRANSPORT()->advertised_host_ip.begin();
       for (; i != AAA_CFG_TRANSPORT()->advertised_host_ip.end(); 
            i++) {
           ACE_INET_Addr hostAddr((*i).data());           
           diameter_address_t &ipAvp = hostIp.Get();
           ipAvp.type = AAA_ADDRESS_IP;
           ipAvp.value.assign((char*)tool.GetAddressPtr(hostAddr),
                              tool.GetAddressSize(hostAddr));
       }
   }
   else {
       size_t count;
       ACE_INET_Addr *addrs;
       if (tool.GetLocalAddresses(count, addrs) == 0) {
           for(size_t i = 0; i < count; i++) {
               diameter_address_t &ipAvp = hostIp.Get();
               ipAvp.type = AAA_ADDRESS_IP;
               ipAvp.value.assign((char*)tool.GetAddressPtr(addrs[i]),
                                   tool.GetAddressSize(addrs[i]));
           }
	   if (count > 0) {
               delete[] addrs;
	   }
       }
   }
   
   vendorId.Get() = AAA_CFG_GENERAL()->vendor;
   product.Get() = AAA_CFG_GENERAL()->product;
   originStateId.Get() = AAA_CFG_RUNTIME()->originStateId;

   AAA_ApplicationIdLst *idList[] = {
       &AAA_CFG_GENERAL()->supportedVendorIdLst,
       &AAA_CFG_GENERAL()->authAppIdLst,
       &AAA_CFG_GENERAL()->acctAppIdLst
   };
   DiameterUInt32AvpWidget *widgets[] = {
       &supportedVendorId,
       &authId,
       &acctId
   };
   for (unsigned int i=0; i<sizeof(widgets)/sizeof(DiameterUInt32AvpWidget*); i++) {
       AAA_ApplicationIdLst::iterator x = idList[i]->begin();
       for (; x != idList[i]->end(); x++) {
           widgets[i]->Get() = *x;
       }
       if (! widgets[i]->empty()) {
           msg.acl.add((*widgets[i])());
       }
   }
   
   AAA_VendorSpecificIdLst::iterator y =
       AAA_CFG_GENERAL()->vendorSpecificId.begin();
   for (; y != AAA_CFG_GENERAL()->vendorSpecificId.end();
        y++) {
       AAA_DataVendorSpecificApplicationId vid = *y;
       diameter_grouped_t &grp = vendorSpecificId.Get();

       if (vid.authAppId > 0) {
           DiameterUInt32AvpWidget gAuthId(AAA_AVPNAME_AUTHAPPID, vid.authAppId);
           grp.add(gAuthId());
       }
       if (vid.acctAppId > 0) {
           DiameterUInt32AvpWidget gAcctId(AAA_AVPNAME_ACCTAPPID, vid.acctAppId);
           grp.add(gAcctId());
       }       

       DiameterUInt32AvpWidget gVendorId(AAA_AVPNAME_VENDORID);       
       AAA_ApplicationIdLst::iterator z = vid.vendorIdLst.begin();
       for (; z != vid.vendorIdLst.end(); z++) {
           gVendorId.Get() = *z;
       }
       grp.add(gVendorId());
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

void AAA_PeerStateMachine::DisassembleCE(DiameterMsg &msg)
{
   int ndx; 
   AAA_PeerCapabilities &cap = m_Data.m_PeerCapabilities;
   
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

   diameter_identity_t *identity = originHost.GetAvp(AAA_AVPNAME_ORIGINHOST);
   cap.m_Host.assign((identity) ? identity->data() : "",
                     (identity) ? identity->length() : 0);
   
   identity = originRealm.GetAvp(AAA_AVPNAME_ORIGINREALM);
   cap.m_Realm.assign((identity) ? identity->data() : "",
                      (identity) ? identity->length() : 0);
              

   while (! cap.m_HostIpLst.empty()) {
       diameter_address_t *ip = cap.m_HostIpLst.front();
       cap.m_HostIpLst.pop_front();
       delete ip;
   }
   
   diameter_address_t *address = hostIp.GetAvp(AAA_AVPNAME_HOSTIP);
   for (ndx=1; address; ndx++) {
       diameter_address_t *newAddr = new diameter_address_t;
       *newAddr = *address;
       cap.m_HostIpLst.push_back(newAddr);
       address = hostIp.GetAvp(AAA_AVPNAME_HOSTIP, ndx);
   }

   diameter_unsigned32_t *uint32 = vendorId.GetAvp(AAA_AVPNAME_VENDORID);
   cap.m_VendorId = (uint32) ? *uint32 : 0;

   diameter_utf8string_t *utf8str = product.GetAvp(AAA_AVPNAME_PRODUCTNAME);
   cap.m_ProductName.assign((utf8str) ? utf8str->data() : "",
                            (utf8str) ? utf8str->length() : 0);
   
   uint32 = originState.GetAvp(AAA_AVPNAME_ORIGINSTATEID);
   cap.m_OriginStateId = (uint32) ? *uint32 : 0;
   
   uint32 = firmware.GetAvp(AAA_AVPNAME_FIRMWAREREV);
   cap.m_FirmwareRevision = (uint32) ? *uint32 : 0;
   
   uint32 = inbandSecId.GetAvp(AAA_AVPNAME_INBANDSECID);
   cap.m_InbandSecurityId = (uint32) ? *uint32 : 0;
   
   AAA_ApplicationIdLst *idList[] = {
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
       AAA_AVPNAME_SUPPORTEDVENDORID,
       AAA_AVPNAME_AUTHAPPID,
       AAA_AVPNAME_ACCTAPPID
   };
   for (unsigned int i=0;
        i<sizeof(idList)/sizeof(AAA_ApplicationIdLst*);
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
       AAA_VendorSpecificIdLst::iterator x =
           cap.m_VendorSpecificId.begin();
       AAA_ApplicationIdLst &id = (*x).vendorIdLst;
       while (! id.empty()) {
           id.pop_front();
       }
       cap.m_VendorSpecificId.pop_front();
   }
   diameter_grouped_t *grouped = vendorSpecificId.GetAvp(AAA_AVPNAME_VENDORAPPID);
   for (ndx=1; grouped; ndx++) {
       AAA_DataVendorSpecificApplicationId vsid;
       DiameterUInt32AvpContainerWidget gAuthId(*grouped);
       DiameterUInt32AvpContainerWidget gAcctId(*grouped);
       DiameterUInt32AvpContainerWidget gVendorId(*grouped);

       uint32 = gVendorId.GetAvp(AAA_AVPNAME_VENDORID);
       for (int p=1; uint32; p++) {
           vsid.vendorIdLst.push_back(*uint32);
           uint32 = gVendorId.GetAvp(AAA_AVPNAME_VENDORID, p);
       }
       
       uint32 = gAuthId.GetAvp(AAA_AVPNAME_AUTHAPPID);
       vsid.authAppId = (uint32) ? *uint32 : 0;
       uint32 = gAcctId.GetAvp(AAA_AVPNAME_ACCTAPPID);
       vsid.acctAppId = (uint32) ? *uint32 : 0;

       cap.m_VendorSpecificId.push_back(vsid);
       grouped = vendorSpecificId.GetAvp(AAA_AVPNAME_VENDORAPPID, ndx);
   }

   DumpPeerCapabilities();
}

void AAA_PeerStateMachine::AssembleDW(DiameterMsg &msg, bool request)
{
   DiameterMsgHeader &h = msg.hdr;
   ACE_OS::memset(&msg.hdr, 0, sizeof(msg.hdr));
   h.ver = DIAMETER_PROTOCOL_VERSION;
   h.length = 0;
   h.flags.r = request ? DIAMETER_FLAG_SET : DIAMETER_FLAG_CLR;
   h.flags.p = DIAMETER_FLAG_CLR;
   h.flags.e = DIAMETER_FLAG_CLR;
   h.code = AAA_MSGCODE_WATCHDOG;
   h.appId = DIAMETER_BASE_APPLICATION_ID;
   MsgIdTxMessage(msg);

   DiameterIdentityAvpWidget originHost(AAA_AVPNAME_ORIGINHOST);
   DiameterIdentityAvpWidget originRealm(AAA_AVPNAME_ORIGINREALM);
   DiameterUInt32AvpWidget   originStateId(AAA_AVPNAME_ORIGINSTATEID);

   originHost.Get() = AAA_CFG_TRANSPORT()->identity;
   originRealm.Get() = AAA_CFG_TRANSPORT()->realm;
   originStateId.Get() = AAA_CFG_RUNTIME()->originStateId;

   msg.acl.add(originHost());
   msg.acl.add(originRealm());
   msg.acl.add(originStateId());
}

void AAA_PeerStateMachine::DisassembleDW(DiameterMsg &msg)
{
   diameter_identity_t Host;
   diameter_identity_t Realm;
   
   DiameterIdentityAvpContainerWidget originHost(msg.acl);
   DiameterIdentityAvpContainerWidget originRealm(msg.acl);
   DiameterUInt32AvpContainerWidget originState(msg.acl);

   diameter_identity_t *identity = originHost.GetAvp(AAA_AVPNAME_ORIGINHOST);
   Host.assign((identity) ? identity->data() : "",
               (identity) ? identity->length() : 0);
   
   identity = originRealm.GetAvp(AAA_AVPNAME_ORIGINREALM);
   Realm.assign((identity) ? identity->data() : "",
                (identity) ? identity->length() : 0);
              
   diameter_unsigned32_t *uint32 = originState.GetAvp(AAA_AVPNAME_ORIGINSTATEID);
   diameter_unsigned32_t OriginStateId = (uint32) ? *uint32 : 0;
   
   AAA_LOG(LM_INFO, "(%P|%t) Watchdog msg from [%s.%s], state=%d, time=%d\n",
             Host.data(), Realm.data(), OriginStateId, time(0));
}

void AAA_PeerStateMachine::AssembleDP(DiameterMsg &msg,
                                      bool request)
{
   DiameterMsgHeader &h = msg.hdr;
   h.ver = DIAMETER_PROTOCOL_VERSION;
   h.length = 0;
   h.flags.r = request ? DIAMETER_FLAG_SET : DIAMETER_FLAG_CLR;
   h.flags.p = DIAMETER_FLAG_CLR;
   h.flags.e = DIAMETER_FLAG_CLR;
   h.code = AAA_MSGCODE_DISCONNECT_PEER;
   h.appId = DIAMETER_BASE_APPLICATION_ID;
   MsgIdTxMessage(msg);

   DiameterIdentityAvpWidget originHost(AAA_AVPNAME_ORIGINHOST);
   DiameterIdentityAvpWidget originRealm(AAA_AVPNAME_ORIGINREALM);

   originHost.Get() = AAA_CFG_TRANSPORT()->identity;
   originRealm.Get() = AAA_CFG_TRANSPORT()->realm;

   msg.acl.add(originHost());
   msg.acl.add(originRealm());

}

void AAA_PeerStateMachine::DisassembleDP(DiameterMsg &msg)
{
   diameter_identity_t Host;
   diameter_identity_t Realm;
   
   DiameterIdentityAvpContainerWidget originHost(msg.acl);
   DiameterIdentityAvpContainerWidget originRealm(msg.acl);

   diameter_identity_t *identity = originHost.GetAvp(AAA_AVPNAME_ORIGINHOST);
   Host.assign((identity) ? identity->data() : "",
               (identity) ? identity->length() : 0);
   
   identity = originRealm.GetAvp(AAA_AVPNAME_ORIGINREALM);
   Realm.assign((identity) ? identity->data() : "",
                (identity) ? identity->length() : 0);
              
   AAA_LOG(LM_INFO, "(%P|%t) Disconnect msg from [%s.%s]\n",
              Host.data(), Realm.data());
}

void AAA_PeerStateMachine::SendCER()
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
   if (RawSend(msg, m_Data.m_IOInitiator.get()) == 0) {
       AAA_LOG(LM_INFO, "(%P|%t) Sent CER\n");
   }
}

void AAA_PeerStateMachine::SendCEA(diameter_unsigned32_t rcode,
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

   DiameterUInt32AvpWidget resultCode(AAA_AVPNAME_RESULTCODE, rcode);
   msg->acl.add(resultCode());
   
   DiameterMsgResultCode::RCODE interpretedRcode = 
       DiameterMsgResultCode::InterpretedResultCode(rcode);
   if ((message.length() > 0) && 
       (interpretedRcode != DiameterMsgResultCode::RCODE_NOT_PRESENT) &&
       (interpretedRcode != DiameterMsgResultCode::RCODE_SUCCESS)) {
       DiameterUtf8AvpWidget errorMsg(AAA_AVPNAME_ERRORMESSAGE);
       errorMsg.Get() = message.data();
       msg->acl.add(errorMsg());
   }

   // check resulting state to determine
   // which IO to use
   AAA_IO_Base *io = (state == AAA_PEER_ST_I_OPEN) ?
                      m_Data.m_IOInitiator.get() :
                      m_Data.m_IOResponder.get();
   if (RawSend(msg, io) == 0) {
       AAA_LOG(LM_INFO, "(%P|%t) Sent CEA: rcode=%d\n",
                 rcode);
   }
}

void AAA_PeerStateMachine::SendDWR()
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
   AAA_IO_Base *io = (state == AAA_PEER_ST_I_OPEN) ?
                      m_Data.m_IOInitiator.get() :
                      m_Data.m_IOResponder.get();
   if (RawSend(msg, io) < 0) {
       AAA_LOG(LM_INFO, "(%P|%t) Failed sending DWR\n");
   }
}

void AAA_PeerStateMachine::SendDWA(diameter_unsigned32_t rcode,
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

   DiameterUInt32AvpWidget resultCode(AAA_AVPNAME_RESULTCODE, rcode);
   msg->acl.add(resultCode());
   
#if INTEROP
   if (message.length() > 0) {
       DiameterUtf8AvpWidget errorMsg(AAA_AVPNAME_ERRORMESSAGE);
       errorMsg.Get() = message.data();
       msg->acl.add(errorMsg());
   }
#endif

   // check resulting state to determine
   // which IO to use
   AAA_IO_Base *io = (state == AAA_PEER_ST_I_OPEN) ?
                      m_Data.m_IOInitiator.get() :
                      m_Data.m_IOResponder.get();
   if (RawSend(msg, io) < 0) {
       AAA_LOG(LM_INFO, "(%P|%t) Failed sending DWA: rcode=%d\n",
                 rcode);
   }
}

void AAA_PeerStateMachine::SendDPR(bool initiator)
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
   
   DiameterUInt32AvpWidget disCause(AAA_AVPNAME_DISCONNECT_CAUSE);
   disCause.Get() = diameter_unsigned32_t(m_Data.m_DisconnectCause);
   msg->acl.add(disCause());   
   
   AAA_IO_Base *io = (initiator) ?
                      m_Data.m_IOInitiator.get() :
                      m_Data.m_IOResponder.get();
   if (RawSend(msg, io) < 0) {
       AAA_LOG(LM_INFO, "(%P|%t) Failed sending Disconnect\n");
   }
}

void AAA_PeerStateMachine::SendDPA(bool initiator,
                                   diameter_unsigned32_t rcode,
                                   std::string &message)
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

   DiameterUInt32AvpWidget resultCode(AAA_AVPNAME_RESULTCODE, rcode);
   msg->acl.add(resultCode());
   
   if (message.length() > 0) {
       DiameterUtf8AvpWidget errorMsg(AAA_AVPNAME_ERRORMESSAGE);
       errorMsg.Get() = message.data();
       msg->acl.add(errorMsg());
   }

   AAA_IO_Base *io = (initiator) ?
                      m_Data.m_IOInitiator.get() :
                      m_Data.m_IOResponder.get();
   if (RawSend(msg, io) < 0) {
       AAA_LOG(LM_INFO, "(%P|%t) Failed sending DWA: rcode=%d\n",
                 rcode);
   }
}

void AAA_PeerStateMachine::Elect()
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
   AAA_LOG(LM_DEBUG, "(%P|%t) Election occurring ...\n");
   std::string localHost = AAA_CFG_TRANSPORT()->identity;
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
           Notify(AAA_PEER_EV_WIN_ELECTION);
           AAA_LOG(LM_INFO, "(%P|%t) ***** Local peer wins election *****\n");
           return;
       }
       else {
           break;
       }
   }
   AAA_LOG(LM_INFO, "(%P|%t) ***** Peer (%s) wins election *****\n",
              peerHost.data());
}

void AAA_PeerStateMachine::Cleanup(unsigned int flags)
{
   AAA_PeerCapabilities &cap = m_Data.m_PeerCapabilities;

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

   AAA_ApplicationIdLst *idList[] = {
       &cap.m_SupportedVendorIdLst,
       &cap.m_AuthAppIdLst,
       &cap.m_AcctAppIdLst
   };
   for (unsigned int i=0;
        i<sizeof(idList)/sizeof(AAA_ApplicationIdLst*);
        i++) {
       while (! idList[i]->empty()) {
           idList[i]->pop_front();
       }
   }

   while (! cap.m_VendorSpecificId.empty()) {
       AAA_VendorSpecificIdLst::iterator x =
           cap.m_VendorSpecificId.begin();
       AAA_ApplicationIdLst &id = (*x).vendorIdLst;
       while (! id.empty()) {
           id.pop_front();
       }
       cap.m_VendorSpecificId.pop_front();
   }

   cap.m_InbandSecurityId = 0;
   cap.m_FirmwareRevision = 0;

   CancelTimer(AAA_PEER_EV_TIMEOUT);
   CancelTimer(AAA_PEER_EV_WATCHDOG);
   CancelTimer(AAA_PEER_EV_CONN_RETRY);
   
   if (flags & CLEANUP_FSM) {
       AAA_StateMachineWithTimer<AAA_PeerStateMachine>::Stop();
   }

   if (flags & CLEANUP_IO_R) {
       m_Data.m_IOResponder.reset();
   }
   if (flags & CLEANUP_IO_I) {
       m_Data.m_IOInitiator.reset();
   }

   if (flags & CLEANUP_FSM) {
       m_CurrentPeerEventParam->m_Msg.reset();
       m_CurrentPeerEventParam->m_IO.reset();
       AAA_StateMachineWithTimer<AAA_PeerStateMachine>::Start();
   }
   
   m_CleanupEvent = true;
}

int AAA_PeerStateMachine::RawSend(std::auto_ptr<DiameterMsg> &msg, 
                                  AAA_IO_Base *io)
{
   AAAMessageBlock *aBuffer = NULL;

   for (int blockCnt = 1; 
        blockCnt <= AAA_MsgCollector::MAX_MSG_BLOCK; 
        blockCnt ++) {
       
       aBuffer = AAAMessageBlock::Acquire
             (AAA_MsgCollector::MAX_MSG_LENGTH * blockCnt);

       msg->acl.reset();

       DiameterMsgHeaderParser hp;
       hp.setRawData(aBuffer);
       hp.setAppData(&msg->hdr);
       hp.setDictData(DIAMETER_PARSE_STRICT);
   
       try {
          hp.parseAppToRaw();
       }
       catch (DiameterErrorCode &st) {
          ACE_UNUSED_ARG(st);
          aBuffer->Release();
          return (-1);
       }

       DiameterMsgPayloadParser pp;
       pp.setRawData(aBuffer);
       pp.setAppData(&msg->acl);
       pp.setDictData(msg->hdr.getDictHandle());

       try { 
          pp.parseAppToRaw();
       }
       catch (DiameterErrorCode &st) {
          aBuffer->Release();

          AAA_PARSE_ERROR_TYPE type;
          int code;
          st.get(type, code);
          if ((type == AAA_PARSE_ERROR_TYPE_NORMAL) && (code == AAA_OUT_OF_SPACE)) {
              if (blockCnt < AAA_MsgCollector::MAX_MSG_BLOCK) {
                   msg->acl.reset();
                   continue;
              }
              AAA_LOG(LM_ERROR, "(%P|%t) Not enough block space for transmission\n");
          }
          return (-1);
      }

      msg->hdr.length = aBuffer->wr_ptr() - aBuffer->base();
      try {
          hp.parseAppToRaw();
      }
      catch (DiameterErrorCode &st) {
          aBuffer->Release();
          return (-1);
      }
      break;
   }

   aBuffer->length(msg->hdr.length);
   return io->Send(aBuffer);
}

void AAA_PeerStateMachine::DumpPeerCapabilities()
{
   AAA_PeerCapabilities &cap = m_Data.m_PeerCapabilities;
    
   AAA_LOG(LM_INFO, "(%P|%t) Peer Capabilities\n");
   AAA_LOG(LM_INFO, "(%P|%t)             Hostname : %s\n", cap.m_Host.data());
   AAA_LOG(LM_INFO, "(%P|%t)                Realm : %s\n", cap.m_Realm.data());

   AAA_HostIpLst::iterator x = cap.m_HostIpLst.begin();
   for (; x != cap.m_HostIpLst.end(); x++) {
       AAA_LOG(LM_INFO, "(%P|%t)              Host IP : type=%d, %s\n", (*x)->type,
                  inet_ntoa(*((struct in_addr*)(*x)->value.data())));
   }
   
   AAA_LOG(LM_INFO, "(%P|%t)             VendorId : %d\n", cap.m_VendorId);
   AAA_LOG(LM_INFO, "(%P|%t)         Product Name : %s\n", cap.m_ProductName.data());
   AAA_LOG(LM_INFO, "(%P|%t)           Orig State : %d\n", cap.m_OriginStateId);

   AAA_ApplicationIdLst *idList[] = {
       &cap.m_SupportedVendorIdLst,
       &cap.m_AuthAppIdLst,
       &cap.m_AcctAppIdLst
   };
   char *label[] = { "Supported Vendor Id",
                     "Auth Application Id",
                     "Acct Application Id" };
   for (unsigned int i=0;
        i < sizeof(idList)/sizeof(AAA_ApplicationIdLst*);
        i++) {
       AAA_ApplicationIdLst::iterator x = idList[i]->begin();
       for (; x != idList[i]->end(); x++) {
           AAA_LOG(LM_INFO, "(%P|%t)  %s : %d\n",
                   label[i], *x);
       }
   }
   
   AAA_VendorSpecificIdLst::iterator y = cap.m_VendorSpecificId.begin();
   for (; y != cap.m_VendorSpecificId.end(); y++) {
       AAA_LOG(LM_INFO, "(%P|%t)  Vendor Specific Id : ");
       if ((*y).authAppId > 0) {
           AAA_LOG(LM_INFO, " Auth=%d ", (*y).authAppId);
       }
       if ((*y).acctAppId > 0) {
           AAA_LOG(LM_INFO, " Acct=%d ", (*y).acctAppId);
       }
       AAA_LOG(LM_INFO, "%s\n", (((*y).authAppId == 0) && ((*y).acctAppId == 0)) ? "---" : "");
       AAA_ApplicationIdLst::iterator z = (*y).vendorIdLst.begin();
       for (; z != (*y).vendorIdLst.end(); z++) {
           AAA_LOG(LM_INFO, "(%P|%t)                        vendor id=%d\n",
                      *z);
       }
   }
   
   AAA_LOG(LM_INFO, "(%P|%t)           Inband Sec : %d\n", cap.m_InbandSecurityId);
   AAA_LOG(LM_INFO, "(%P|%t)         Firmware Ver : %d\n", cap.m_FirmwareRevision);
}

bool AAA_PeerStateMachine::ValidatePeer(diameter_unsigned32_t &rcode,
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
   AAA_PeerCapabilities &cap = m_Data.m_PeerCapabilities;

   AAA_ApplicationIdLst *localIdList[] = {
       &AAA_CFG_GENERAL()->supportedVendorIdLst,
       &AAA_CFG_GENERAL()->authAppIdLst,
       &AAA_CFG_GENERAL()->acctAppIdLst
   };
   AAA_ApplicationIdLst *idList[] = {
       &cap.m_SupportedVendorIdLst,
       &cap.m_AuthAppIdLst,
       &cap.m_AcctAppIdLst
   };
   for (unsigned int i=0;
        i < sizeof(idList)/sizeof(AAA_ApplicationIdLst*);
        i++) {
       AAA_ApplicationIdLst::iterator x = idList[i]->begin();
       for (; x != idList[i]->end(); x++) {
           if (AAA_ApplicationIdLookup::Find(*x, localIdList,
               sizeof(localIdList)/sizeof(AAA_ApplicationIdLst*))) {
               found = true;
               break;
           }
           if (AAA_ApplicationIdLookup::Find
               (*x, AAA_CFG_GENERAL()->vendorSpecificId)) {
               found = true;
               break;
           }
       }
       if (found) {
           break;
       }
   }
   if (! found) {
       AAA_VendorSpecificIdLst::iterator i = cap.m_VendorSpecificId.begin();
       for (; i != cap.m_VendorSpecificId.end(); i++) {
           if (AAA_ApplicationIdLookup::Find((*i).authAppId, localIdList,
                sizeof(localIdList)/sizeof(AAA_ApplicationIdLst*))) {
               found = true;
               break;
           }
           if (AAA_ApplicationIdLookup::Find((*i).acctAppId, localIdList,
                sizeof(localIdList)/sizeof(AAA_ApplicationIdLst*))) {
               found = true;
               break;
           }
           if (AAA_ApplicationIdLookup::Find
               ((*i).authAppId, AAA_CFG_GENERAL()->vendorSpecificId)) {
               found = true;
               break;
           }
           if (AAA_ApplicationIdLookup::Find
               ((*i).acctAppId, AAA_CFG_GENERAL()->vendorSpecificId)) {
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

void AAA_PeerStateMachine::MsgIdTxMessage(DiameterMsg &msg)
{
   if (msg.hdr.flags.r) {
       m_Data.m_PeerCapabilities.m_MsgId.m_LastTxHopId = AAA_HOPBYHOP_GEN()->Get();
       m_Data.m_PeerCapabilities.m_MsgId.m_LastTxEndId = AAA_ENDTOEND_GEN()->Get();
       msg.hdr.hh = m_Data.m_PeerCapabilities.m_MsgId.m_LastTxHopId;
       msg.hdr.ee = m_Data.m_PeerCapabilities.m_MsgId.m_LastTxEndId;
   }
   else {
       msg.hdr.hh = m_Data.m_PeerCapabilities.m_MsgId.m_LastRxHopId;
       msg.hdr.ee = m_Data.m_PeerCapabilities.m_MsgId.m_LastRxEndId;
   }
}

bool AAA_PeerStateMachine::MsgIdRxMessage(DiameterMsg &msg)
{
   if (msg.hdr.flags.r) {
       m_Data.m_PeerCapabilities.m_MsgId.m_LastRxHopId = msg.hdr.hh;
       m_Data.m_PeerCapabilities.m_MsgId.m_LastRxEndId = msg.hdr.ee;
       return (true);
   }
   return ((msg.hdr.hh == m_Data.m_PeerCapabilities.m_MsgId.m_LastTxHopId) &&
           (msg.hdr.ee == m_Data.m_PeerCapabilities.m_MsgId.m_LastTxEndId));
}




