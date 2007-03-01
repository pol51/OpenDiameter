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

#include "aaa_peer_table.h"

void DiameterPeerEntry::Start() throw (AAA_Error)
{
   Notify(DIAMETER_PEER_EV_START);
   m_PeerInitiator.Connect(PeerData().m_Identity,
                           PeerData().m_Port,
                           PeerData().m_UseSctp);
}

void DiameterPeerEntry::Stop(DIAMETER_DISCONNECT_CAUSE cause)
{
   PeerData().m_DisconnectCause = cause;
   Notify(DIAMETER_PEER_EV_STOP);
   m_PeerInitiator.Stop();
}

void DiameterPeerEntry::IncommingConnectionRequest(std::auto_ptr<Diameter_IO_Base> io,
                                                   std::auto_ptr<DiameterMsg> cer)
{
   DiameterRxMsgCollector *h = reinterpret_cast<DiameterRxMsgCollector*>(io->Handler());
   h->RegisterHandler(*this);
   MsgIdRxMessage(*cer);
   Notify(DIAMETER_PEER_EV_R_CONN_CER, cer, io);
}

void DiameterPeerEntry::ConnectionRequestAccepted(std::auto_ptr<Diameter_IO_Base> io)
{
   DiameterRxMsgCollector *h = reinterpret_cast<DiameterRxMsgCollector*>(io->Handler());
   h->RegisterHandler(*this);
   Notify(DIAMETER_PEER_EV_I_RCV_CONN_ACK, io);
}

void DiameterPeerEntry::ConnectionRequestFailed()
{
   Notify(DIAMETER_PEER_EV_I_RCV_CONN_NACK);
}

void DiameterPeerEntry::Message(std::auto_ptr<DiameterMsg> msg)
{

   ////////////////////////////////////////////
   // inspect msg and send proper notification
   ////////////////////////////////////////////

   DiameterMsgQuery query(*msg);
   if (state == DIAMETER_PEER_ST_WAIT_I_CEA) {
       if (! query.IsCapabilities() || query.IsRequest()) {
           Notify(DIAMETER_PEER_EV_I_RCV_NON_CEA);
           return;
       }

   }

   static diameter_unsigned32_t cmdCode[] = { 
                              DIAMETER_MSGCODE_CAPABILITIES_EXCHG,
                              DIAMETER_MSGCODE_WATCHDOG,
                              DIAMETER_MSGCODE_DISCONNECT_PEER 
                            };
   static AAA_Event requestIEv[] = { 
                              DIAMETER_PEER_EV_I_RCV_CER,
                              DIAMETER_PEER_EV_I_RCV_DWR,
                              DIAMETER_PEER_EV_I_RCV_DPR 
                            };
   static AAA_Event requestREv[] = { 
                              DIAMETER_PEER_EV_R_RCV_CER,
                              DIAMETER_PEER_EV_R_RCV_DWR,
                              DIAMETER_PEER_EV_R_RCV_DPR 
                            };
   static AAA_Event answerIEv[]  = { 
                              DIAMETER_PEER_EV_I_RCV_CEA,
                              DIAMETER_PEER_EV_I_RCV_DWA,
                              DIAMETER_PEER_EV_I_RCV_DPA 
                            };
   static AAA_Event answerREv[]  = { 
                              DIAMETER_PEER_EV_R_RCV_CEA,
                              DIAMETER_PEER_EV_R_RCV_DWA,
                              DIAMETER_PEER_EV_R_RCV_DPA 
                            };

   for (unsigned int i=0;
        i<sizeof(cmdCode)/sizeof(diameter_unsigned32_t);
        i++) {
       if (cmdCode[i] == query.Code()) {
           if (! MsgIdRxMessage(*msg) && ! query.IsRequest()) {
               AAA_LOG((LM_INFO,
                   "(%P|%t) Msg[%d] Invalid hop-by-hop or end-to-end id\
                    in answer message\n", query.Code()));
               return;
           }
           switch (state) {
               case DIAMETER_PEER_ST_I_OPEN:
                   Notify(query.IsRequest() ?
                          requestIEv[i] : answerIEv[i],
                          msg);
                   break;
               case DIAMETER_PEER_ST_R_OPEN:
                   Notify(query.IsRequest() ?
                          requestREv[i] : answerREv[i],
                          msg);
                   break;
               case DIAMETER_PEER_ST_CLOSING:
                   if (PeerData().m_IOInitiator.get()) {
                       Notify(query.IsRequest() ?
                              requestIEv[i] : answerIEv[i], msg);
                   }
                   else {
                       Notify(query.IsRequest() ?
                              requestREv[i] : answerREv[i],
                       msg);
                   }
                   break;
               case DIAMETER_PEER_ST_WAIT_RETURNS:
               case DIAMETER_PEER_ST_WAIT_I_CEA:
                   if (query.IsCapabilities() &&
                       (! query.IsRequest())) {
                       Notify(DIAMETER_PEER_EV_I_RCV_CEA, msg);
                       break;
                   }
                   // fall through
               default:
                   AAA_LOG((LM_DEBUG,
                             "(%P|%t) Message [%d,%d] in unlikely state [%d], discarding\n",
                              cmdCode[i], query.IsRequest(), state));
           }
           return;
       }
   }

   switch (state) {
       case DIAMETER_PEER_ST_I_OPEN:
           Notify(DIAMETER_PEER_EV_I_RCV_MESSAGE, msg);
           break;
       case DIAMETER_PEER_ST_R_OPEN:
           Notify(DIAMETER_PEER_EV_R_RCV_MESSAGE, msg);
           break;
       default:
           AAA_LOG((LM_INFO,
                      "(%P|%t) Received session non base protocol message\
                       in an un-opened state, discarding\n"));
           break;
   }
}

void DiameterPeerEntry::Error(COLLECTOR_ERROR error,
                              std::string &io_name)
{
   AAA_LOG((LM_DEBUG, "(%P|%t) Message Collector reported [%s]\n",
              io_name.c_str()));

   switch (error) {
       case CORRUPTED_BYTE_STREAM:
       case TRANSPORT_ERROR:
           if (io_name == std::string(AAA_IO_ACCEPTOR_NAME)) {
               Notify(DIAMETER_PEER_EV_R_PEER_DISC);
           }
           else if (io_name == std::string(AAA_IO_CONNECTOR_NAME)) {
               Notify(DIAMETER_PEER_EV_I_PEER_DISC);
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

int DiameterPeerEntry::SendErrorAnswer(std::auto_ptr<DiameterMsg> &msg)
{
   return DiameterPeerStateMachine::Send(msg);
}

