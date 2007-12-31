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

#include "ace/OS.h"
#include "ace/Date_Time.h"
#include "pana_session.h"
#include "pana_paa_factory.h"
#include "pana_config_manager.h"

PANA_ReplicatableNumbers::PANA_ReplicatableNumbers()
{
    m_GetCount = 0;
    m_Base = ACE_OS::gettimeofday();
}

ACE_UINT32 PANA_ReplicatableNumbers::Get(ACE_UINT32 seed)
{
    // Get current minutes
    ACE_Date_Time dateTime;
    dateTime.update(m_Base);

    // use RAND so that the 32-bit numbers can be replicated using the same seed
    m_GetCount ++;
    ACE_OS::srand(seed + dateTime.minute() + m_GetCount);
    return ACE_OS::rand();
}

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
             case PANA_MTYPE_PAN:
                 StatelessRxPANStart(msg);
                 break;
             default:
                 AAA_LOG((LM_ERROR, "(%P|%t) Unknown msg during handshake, discarding: seq=%u\n",\
                          msg.seq()));
                 break;
          }
      }
      return;
   }
   catch (...) {
      AAA_LOG((LM_ERROR, "(%P|%t) Unknown error receipt of msg, discarding: seq=%u\n",
               msg.seq()));
   }
}

void PANA_PaaSessionFactory::PacFound(ACE_INET_Addr &addr)
{
    ///////////////////////////////////////////////////////////////
    // - - - - - - - - - - (Optimized Handshake) - - - - - - - - - - -
    // (Rx:PCI[] ||             if (OPTIMIZED_INIT == Set) INITIAL
    //  PAC_FOUND)                EAP_Restart();
    //                          else
    //                            Tx:PAR[S]();
    //
    if (PANA_CFG_PAA().m_OptimizedHandshake) {

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
           AAA_LOG((LM_INFO, "(%P|%t) PAA is using EAP optimization\n"));
       }
       else {
           AAA_LOG((LM_INFO, "(%P|%t) PAA is acting stateful\n"));
       }
       session->Notify(ev.Get());
   }
   else {
       AAA_LOG((LM_INFO, "(%P|%t) PAA is acting stateless\n"));
       StatelessTxPARStart(addr);
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

   AAA_LOG((LM_INFO, "(%P|%t) RxPCI: id=%u seq=%u\n",
            msg.sessionId(), msg.seq()));

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

boost::shared_ptr<PANA_Message> PANA_PaaSessionFactory::GenerateStatelessPAR()
{
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PAR;
    msg->flags().request = true;
    msg->flags().start = true;

    // add integrity algorithm
    PANA_UInt32AvpWidget integrityAlgoAvp(PANA_AVPNAME_INTEGRITY_ALGO);
    integrityAlgoAvp.Get() = PANA_AUTH_HMAC_SHA1_160;
    msg->avpList().add(integrityAlgoAvp());

    // add prf algorithm
    PANA_UInt32AvpWidget prfAlgoAvp(PANA_AVPNAME_PRF_ALGO);
    prfAlgoAvp.Get() = PANA_PRF_HMAC_SHA1;
    msg->avpList().add(prfAlgoAvp());

    return msg;
}

void PANA_PaaSessionFactory::StatelessTxPARStart(ACE_INET_Addr &addr)
{
   /*
    7.2.  PANA-Auth-Request (PAR)

      The PANA-Auth-Request (PAR) message is either sent by the PAA or the
      PaC.

      The message MUST NOT have both 'S' and 'C' bits set.

      PANA-Auth-Request ::= < PANA-Header: 2, REQ[,STA][,COM] >
                          [ Nonce ]
                         *[ PRF-Algorithm ]
                         *[ Integrity-Algorithm ]
                          [ Result-Code ]
                          [ Session-Lifetime ]
                          [ Key-Id ]
                        * [ AVP ]
                      0*1 < AUTH >
    */
    boost::shared_ptr<PANA_Message> msg = GenerateStatelessPAR();

    // generate good numbers
    PANA_ReplicatableNumbers replica;
    msg->sessionId() = replica.Get(addr.get_ip_address());
    msg->seq() = replica.Get(addr.get_ip_address());

    // proper addresses
    char buf[32];
    sprintf(buf, "%s:%d", PANA_CFG_GENERAL().m_ListenAddress.data(),
            PANA_CFG_GENERAL().m_ListenPort);
    msg->srcAddress().set(buf);
    msg->destAddress() = addr;

    AAA_LOG((LM_INFO, "(%P|%t) TxPAR-Start: id=%u seq=%u\n",
             msg->sessionId(), msg->seq()));

    m_Channel.Send(msg);
}

void PANA_PaaSessionFactory::StatelessRxPANStart(PANA_Message &msg)
{
    /*
      7.3.  PANA-Auth-Answer (PAN)

        The PANA-Auth-Answer (PAN) message is sent by either the PaC or the
        PAA in response to a PANA-Auth-Request message.

        The message MUST NOT have both 'S' and 'C' bits set.

        PANA-Auth-Answer ::= < PANA-Header: 2 [,STA][,COM] >
                            [ Nonce ]
                            [ EAP-Payload ]
                            [ Key-Id ]
                          *  [ AVP ]
                        0*1 < AUTH >
    */

   AAA_LOG((LM_INFO, "(%P|%t) RxPAN-Start: Stateless, id=%u seq=%u\n",
           msg.sessionId(), msg.seq()));

   // sanity checks
   if (msg.sessionId() == 0) {
       throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                             "Invalid session ID in PAN, ingorning answer"));
   }
   if (msg.flags().start == 0) {
       throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                             "Expecting PAN-Start, ingorning answer"));
   }

   // Numbers validation
   PANA_ReplicatableNumbers replica;
   if (msg.sessionId() != replica.Get(msg.srcAddress().get_ip_address())) {
       throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                             "Session ID is not what we expect, ingorning answer"));
   }
   if (msg.seq() != replica.Get(msg.srcAddress().get_ip_address())) {
       throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
                             "Sequence number is not what we expect, ingorning answer"));
   }

   PANA_PaaSession *session = Create();
   if (session == NULL) {
       throw (PANA_Exception(PANA_Exception::NO_MEMORY,
                             "Failed to auth agent session"));
   }

   AAA_LOG((LM_INFO, "(%P|%t) New session created [stateless handshake]\n"));

   // save matching PAR (stateless)
   boost::shared_ptr<PANA_Message> matchingPAR = GenerateStatelessPAR();
   matchingPAR->sessionId() = msg.sessionId();
   matchingPAR->seq() = msg.seq();

   PANA_MsgByteStream byteConverter;
   PANA_MessageBuffer *buffer = byteConverter.Get(*matchingPAR);
   session->m_PAA.SecurityAssociation().PARStart().assign(buffer->base(), matchingPAR->length());

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
