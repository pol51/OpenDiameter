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

#include "aaa_peer_table.h"

void AAA_PeerEntry::Start() throw (AAA_Error)
{
   Notify(AAA_PEER_EV_START);
   m_PeerInitiator.Connect(PeerData().m_Identity,
                           PeerData().m_Port,
                           PeerData().m_TLS);
}

void AAA_PeerEntry::Stop(AAA_DISCONNECT_CAUSE cause)
{
   PeerData().m_DisconnectCause = cause;
   Notify(AAA_PEER_EV_STOP);
   m_PeerInitiator.Stop();
   AAA_PeerStateMachine::WaitOnCleanup();
}

void AAA_PeerEntry::IncommingConnectionRequest(std::auto_ptr<AAA_IO_Base> io,
                                               std::auto_ptr<AAAMessage> cer)
{
   AAA_MsgCollector *h = reinterpret_cast<AAA_MsgCollector*>(io->Handler());
   h->RegisterHandler(*this);
   MsgIdRxMessage(*cer);
   Notify(AAA_PEER_EV_R_CONN_CER, cer, io);
}

void AAA_PeerEntry::ConnectionRequestAccepted(std::auto_ptr<AAA_IO_Base> io)
{
   AAA_MsgCollector *h = reinterpret_cast<AAA_MsgCollector*>(io->Handler());
   h->RegisterHandler(*this);
   Notify(AAA_PEER_EV_I_RCV_CONN_ACK, io);
}

void AAA_PeerEntry::ConnectionRequestFailed()
{
   Notify(AAA_PEER_EV_I_RCV_CONN_NACK);
}

void AAA_PeerEntry::Message(std::auto_ptr<AAAMessage> msg)
{

   ////////////////////////////////////////////
   // inspect msg and send proper notification
   ////////////////////////////////////////////

   AAA_MsgQuery query(*msg);
   if (state == AAA_PEER_ST_WAIT_I_CEA) {
       if (! query.IsCapabilities() || query.IsRequest()) {
           Notify(AAA_PEER_EV_I_RCV_NON_CEA);
           return;
       }

   }

   static diameter_unsigned32_t cmdCode[] = { 
                              AAA_MSGCODE_CAPABILITIES_EXCHG,
                              AAA_MSGCODE_WATCHDOG,
                              AAA_MSGCODE_DISCONNECT_PEER 
                            };
   static AAA_Event requestIEv[] = { 
                              AAA_PEER_EV_I_RCV_CER,
                              AAA_PEER_EV_I_RCV_DWR,
                              AAA_PEER_EV_I_RCV_DPR 
                            };
   static AAA_Event requestREv[] = { 
                              AAA_PEER_EV_R_RCV_CER,
                              AAA_PEER_EV_R_RCV_DWR,
                              AAA_PEER_EV_R_RCV_DPR 
                            };
   static AAA_Event answerIEv[]  = { 
                              AAA_PEER_EV_I_RCV_CEA,
                              AAA_PEER_EV_I_RCV_DWA,
                              AAA_PEER_EV_I_RCV_DPA 
                            };
   static AAA_Event answerREv[]  = { 
                              AAA_PEER_EV_R_RCV_CEA,
                              AAA_PEER_EV_R_RCV_DWA,
                              AAA_PEER_EV_R_RCV_DPA 
                            };

   for (int i=0;
        i<sizeof(cmdCode)/sizeof(diameter_unsigned32_t);
        i++) {
       if (cmdCode[i] == query.Code()) {           
           if (! MsgIdRxMessage(*msg) && ! query.IsRequest()) {
               AAA_LOG(LM_INFO,
                   "(%P|%t) Msg[%d] Invalid hop-by-hop or end-to-end id\
                    in answer message\n", query.Code());
               return;
           }
           switch (state) {
               case AAA_PEER_ST_I_OPEN:
                   Notify(query.IsRequest() ?
                          requestIEv[i] : answerIEv[i],
                          msg);
                   break;
               case AAA_PEER_ST_R_OPEN:
                   Notify(query.IsRequest() ?
                          requestREv[i] : answerREv[i],
                          msg);
                   break;
               case AAA_PEER_ST_CLOSING:                   
                   if (PeerData().m_IOInitiator.get()) {
                       Notify(query.IsRequest() ?
                              requestIEv[i] : answerIEv[i],
                       msg);
                   }
                   else {
                       Notify(query.IsRequest() ?
                              requestREv[i] : answerREv[i],
                       msg);
                   }
                   break;
               case AAA_PEER_ST_WAIT_RETURNS:
               case AAA_PEER_ST_WAIT_I_CEA:
                   if (query.IsCapabilities() &&
                       (! query.IsRequest())) {
                       Notify(AAA_PEER_EV_I_RCV_CEA, msg);
                       break;
                   }
                   // fall through
               default:
                   AAA_LOG(LM_DEBUG,
                             "(%P|%t) Message [%d,%d] in unlikely state [%d], discarding\n",
                              cmdCode[i], query.IsRequest(), state);
           }
           return;
       }
   }

   switch (state) {
       case AAA_PEER_ST_I_OPEN:
           Notify(AAA_PEER_EV_I_RCV_MESSAGE, msg);
           break;
       case AAA_PEER_ST_R_OPEN:
           Notify(AAA_PEER_EV_R_RCV_MESSAGE, msg);
           break;
       default:
           AAA_LOG(LM_INFO,
                      "(%P|%t) Received session non base protocol message\
                       in an un-opened state, discarding\n");
           break;
   }
}

void AAA_PeerEntry::Error(COLLECTOR_ERROR error, 
                          std::string &io_name)
{   
   static char *errMsg[] = { "Parsing error",
                             "Allocation failure",
                             "Transport disconnection",
                             "Invalid message" };

   AAA_LOG(LM_DEBUG, "(%P|%t) IO [%s] reported: %s\n",
              io_name.data(), errMsg[error-PARSING_ERROR]);
   
   switch (error) {
       case TRANSPORT_ERROR:
           if (io_name == std::string(AAA_IO_ACCEPTOR_NAME)) {
               Notify(AAA_PEER_EV_R_PEER_DISC);
           }
           else if (io_name == std::string(AAA_IO_CONNECTOR_NAME)) {
               Notify(AAA_PEER_EV_I_PEER_DISC);
           }
           else {
               Cleanup();
           }
           break;
       case PARSING_ERROR:
           // not fatal so disregard
           break;
       default:
           Cleanup();
           break;
   }
}


