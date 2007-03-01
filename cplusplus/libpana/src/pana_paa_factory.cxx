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

#include "pana_session.h"
#include "pana_paa_factory.h"
#include "pana_config_manager.h"

void PANA_PaaSessionFactory::Receive(PANA_Message &msg)
{
   try {
      PANA_PaaSession *session = PANA_SESSIONDB_SEARCH(msg.sessionId());
      session->Receive(msg);
   }
   catch (PANA_SessionDb::DB_ERROR cause) {
      if (cause == PANA_SessionDb::ENTRY_NOT_FOUND) {
          switch (msg.type()) {
             case PANA_MTYPE_PCI:
                 RxPCI(msg);
                 break;
             case PANA_MTYPE_PSA:
                 StatelessRxPSA(msg);
                 break;
             default:
                 AAA_LOG((LM_ERROR, "(%P|%t) Unknown msg during handshake, discarding: seq=%d\n", msg.seq()));
                 break;
          }
      }
      return;
   }
   catch (...) {
      AAA_LOG((LM_ERROR, "(%P|%t) Unknown error receipt of msg, discarding: seq=%d\n", msg.seq()));
   }
}

void PANA_PaaSessionFactory::PacFound(ACE_INET_Addr &addr)
{
   if (PANA_CFG_PAA().m_OptimizedHandshake || PANA_CFG_PAA().m_RetryPSR) {

       ///////////////////////////////////////////////////////////////
       // - - - - - - - - - - (Optimized Handshake) - - - - - - - - - - -
       // (Rx:PCI ||               EAP_Restart();             WAIT_EAP_MSG_
       //  PAC_FOUND) &&                                      IN_INIT
       // OPTIMIZED_HANDSHAKE==Set
       //
       // - - - - - - - - - - (Non-Optimized Handshake) - - - - - - - - -
       // (Rx:PCI ||               if (AUTH_ALGORITHM_IN_PSR  WAIT_PAC_
       //  PAC_FOUND) &&               ==Set)                 IN_INIT
       // OPTIMIZED_HANDSHAKE==      PSR.insert_avp
       //  Unset &&                  ("Algorithm");
       // RTX_PSR=Set              Tx:PSR();
       //                          RtxTimerStart();
       //
       PANA_PaaSession *session = Create();
       if (session == NULL) {
          throw (PANA_Exception(PANA_Exception::NO_MEMORY,
                               "Failed to auth agent session"));
       }

       AAA_LOG((LM_INFO, "(%P|%t) Found a new PaC, created a session for it\n"));

       // add new session into database
       PANA_SESSIONDB_ADD(session->SessionId(), *session);

       // save PaC address
       session->PacAddress() = addr;

       // notify session
       PANA_PaaEventVariable ev;
       ev.Event_App(PANA_EV_APP_PAC_FOUND);
       if (PANA_CFG_PAA().m_OptimizedHandshake) {
           ev.EnableCfg_OptimizedHandshake();
           PANA_CFG_PAA().m_RetryPSR = true;
           AAA_LOG((LM_INFO, "(%P|%t) PAA is using EAP optimization\n"));
       }
       else {
           AAA_LOG((LM_INFO, "(%P|%t) PAA is acting stateful\n"));
       }
       session->Notify(ev.Get());
   }
   else {
       ///////////////////////////////////////////////////////////////
       // - - - - - - - - - - (Non-Optimized Handshake) - - - - - - - - -
       // (Rx:PCI ||               if (AUTH_ALGORITHM_IN_PSR  OFFLINE
       //  PAC_FOUND) &&               ==Set)
       // OPTIMIZED_HANDSHAKE==      PSR.insert_avp
       //  Unset &&                  ("Algorithm");
       // RTX_PSR=Unset            Tx:PSR();
       //
       AAA_LOG((LM_INFO, "(%P|%t) PAA is acting stateless\n"));
       StatelessTxPSR(addr);
   }
}

void PANA_PaaSessionFactory::RxPCI(PANA_Message &msg)
{
   /*
        7.1.  PANA-Client-Initiation (PCI)

        The PANA-Client-Initiation (PCI) message is used for PaC-initiated
        handshake.  The sequence number in this message is always set to zero
        (0).

        PANA-Client-Initiation ::= < PANA-Header: 1 >
                            *  [ AVP ]

   */

   AAA_LOG((LM_INFO, "(%P|%t) RxPCI: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

   // validate sequence number
   if (msg.seq() != 0) {
      throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
             "Received PCI with non-zero seq num"));
   }

   // validate session id
   if (msg.sessionId() != 0) {
      throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
             "Received PCI with non-zero session id"));
   }

   PacFound(msg.srcAddress());
   delete &msg;
}

void PANA_PaaSessionFactory::StatelessTxPSR(ACE_INET_Addr &addr)
{
   /*
     7.2.  PANA-Start-Request (PSR)

        The PANA-Start-Request (PSR) message is sent by the PAA to the PaC to
        start PANA authentication.  The PAA sets the Sequence Number field to
        an initial random value and sets the Session Identifier field to a
        newly assigned value.

        PANA-Start-Request ::= < PANA-Header: 2, REQ >
                            [ EAP-Payload ]
                            [ Algorithm ]
                         *  [ AVP ]
    */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PSR;
    msg->seq() = PANA_SerialNumber::GenerateISN();
    msg->flags().request = true;

    // generate a new session identifier
    ACE_Time_Value tv = ACE_OS::gettimeofday();
    msg->sessionId() = tv.sec() + tv.usec();

    // proper addresses
    msg->destAddress() = addr;

    AAA_LOG((LM_INFO, "(%P|%t) TxPSR: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    m_Channel.Send(msg);
}

void PANA_PaaSessionFactory::StatelessRxPSA(PANA_Message &msg)
{
    /*
      7.3.  PANA-Start-Answer (PSA)

         The PANA-Start-Answer (PSA) message is sent by the PaC to the PAA in
         response to a PANA-Start-Request message.  This message completes the
         handshake to start PANA authentication.

         PANA-Start-Answer ::= < PANA-Header: 2 >
                             [ EAP-Payload ]
                          *  [ AVP ]
    */

   AAA_LOG((LM_INFO, "(%P|%t) RxPSA: Stateless, id=%d seq=%d\n", msg.sessionId(), msg.seq()));

   if (msg.sessionId() == 0) {
       throw (PANA_Exception(PANA_Exception::NO_MEMORY,
                             "PSA has a zero session identifier"));
   }

   PANA_PaaSession *session = Create();
   if (session == NULL) {
       throw (PANA_Exception(PANA_Exception::NO_MEMORY,
                             "Failed to auth agent session"));
   }

   AAA_LOG((LM_INFO, "(%P|%t) New session created [stateless handshake]\n"));

   // save address of PaC
   session->m_PAA.PacAddress() = msg.srcAddress();

   // save initial seq number
   session->m_PAA.LastTxSeqNum() = msg.seq();

   // save session identifier
   session->SessionId() = msg.sessionId();

   // add to session database permanently
   PANA_SESSIONDB_ADD(session->SessionId(), *session);
   session->Receive(msg);
}
