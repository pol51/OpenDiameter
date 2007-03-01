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
#include "pana_exceptions.h"
#include "pana_session.h"
#include "pana_memory_manager.h"

bool PANA_SessionTimerInterface::ScheduleTxRetry()
{
   /*
     9.  Retransmission Timers

        The PANA protocol provides retransmissions for the PANA-Client-Initiliaze
        message and all request messages, with the exception that the PANA-
        Start-Answer message is retransmitted instead of the PANA-Start-
        Request message in stateless PAA handshake.

        PANA retransmission timers are based on the model used in DHCPv6
        [RFC3315].  Variables used here are also borrowed from this
        specification.  PANA is a request response like protocol.  The
        message exchange terminates when either the request sender
        successfully receives the appropriate answer, or when the message
        exchange is considered to have failed according to the retransmission
        mechanism described below.

        The retransmission behavior is controlled and described by the
        following variables:

         RT     Retransmission timeout

         IRT    Initial retransmission time

         MRC    Maximum retransmission count

         MRT    Maximum retransmission time

         MRD    Maximum retransmission duration

         RAND   Randomization factor

        With each message transmission or retransmission, the sender sets RT
        according to the rules given below.  If RT expires before the message
        exchange terminates, the sender recomputes RT and retransmits the
        message.

        Each of the computations of a new RT include a randomization factor
        (RAND), which is a random number chosen with a uniform distribution
        between -0.1 and +0.1.  The randomization factor is included to
        minimize synchronization of messages.

        The algorithm for choosing a random number does not need to be
        cryptographically sound.  The algorithm SHOULD produce a different
        sequence of random numbers from each invocation.

        RT for the first message transmission is based on IRT:

            RT = IRT + RAND*IRT
   */
   unsigned int newRt = PANA_CFG_GENERAL().m_RT.m_IRT +
              int(RAND()*float(PANA_CFG_GENERAL().m_RT.m_IRT));
   if (newRt > PANA_CFG_GENERAL().m_RT.m_MRT) {
       newRt = PANA_CFG_GENERAL().m_RT.m_MRT +
       int(RAND()*float(PANA_CFG_GENERAL().m_RT.m_MRT));
   }

   m_Timeout = newRt;
   m_Duration = newRt;
   m_Count = 1;

   // schedule the timer
   Schedule(PANA_TID_RETRY, m_Timeout);
   return (true);
}

bool PANA_SessionTimerInterface::ReScheduleTxRetry()
{
   /*
      RT for each subsequent message transmission is based on the previous
      value of RT:

         RT = 2*RTprev + RAND*RTprev

      MRT specifies an upper bound on the value of RT (disregarding the
      randomization added by the use of RAND).  If MRT has a value of 0,
      there is no upper limit on the value of RT.  Otherwise:

         if (RT > MRT)
            RT = MRT + RAND*MRT

      MRC specifies an upper bound on the number of times a sender may
      retransmit a message.  Unless MRC is zero, the message exchange fails
      once the sender has transmitted the message MRC times.

      MRD specifies an upper bound on the length of time a sender may
      retransmit a message.  Unless MRD is zero, the message exchange fails
      once MRD seconds have elapsed since the client first transmitted the
      message.

      If both MRC and MRD are non-zero, the message exchange fails whenever
      either of the conditions specified in the previous two paragraphs are
      met.

      If both MRC and MRD are zero, the client continues to transmit the
      message until it receives a response.
    */

   // cancel any pending timers
   CancelTxRetry();
   unsigned int newRt = 2*m_Timeout + 
               int(RAND()*float(m_Timeout));

   if (newRt > PANA_CFG_GENERAL().m_RT.m_MRT) {
       newRt = PANA_CFG_GENERAL().m_RT.m_MRT +
       int(RAND()*float(PANA_CFG_GENERAL().m_RT.m_MRT));
   }

   if ((PANA_CFG_GENERAL().m_RT.m_MRC > 0) &&
       (m_Count >= PANA_CFG_GENERAL().m_RT.m_MRC)) {
       return (false);
   }

   if ((PANA_CFG_GENERAL().m_RT.m_MRD > 0) &&
       ((m_Duration + newRt) > PANA_CFG_GENERAL().m_RT.m_MRD)) {
       return (false);
   }

   m_Timeout = newRt;
   m_Duration += newRt;
   m_Count ++;

   // re-schedule the timer
   Schedule(PANA_TID_RETRY, m_Timeout);
   return (true);
}

void PANA_Session::NotifyScheduleLifetime()
{
    m_Timer.ScheduleSession(SessionLifetime());
}

bool PANA_Session::IsFatalError()
{
    // TBD: Needs work ... everything is fatal for the moment
    return true;
}

void PANA_Session::TxPUR()
{
    /*
      7.16.  PANA-Update-Request (PUR)

         The PANA-Update-Request (PUR) message is sent either by the PaC or
         the PAA to deliver attribute updates.  In the scope of this
         specification only the IP address the PaC can be updated via this
         message.

         PANA-Update-Request ::= < PANA-Header: 9, REQ >
                          *  [ AVP ]
                         0*1 < AUTH >
     */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PUR;
    msg->flags().request = true;
    msg->sessionId() = this->SessionId();

    // adjust serial num
    ++ LastTxSeqNum();
    msg->seq() = LastTxSeqNum().Value();

    // auth avp if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPUR: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Session::TxPUA()
{
    /*
      7.17.  PANA-Update-Answer (PUA)

         The PANA-Update-Answer (PUA) message is sent by the PAA (PaC) to the
         PaC (PAA) in response to a PANA-Update-Request from the PaC (PAA).

         PANA-Update-Answer ::= < PANA-Header: 9 >
                          *  [ AVP ]
                         0*1 < AUTH >
     */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PUA;
    msg->seq() = LastRxSeqNum().Value();
    msg->sessionId() = this->SessionId();

    // auth avp if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPUA: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Session::TxPPR()
{
    /*
       7.10.  PANA-Ping-Request (PPR)

          The PANA-Ping-Request (PPR) message is either sent by the PaC or the
          PAA for performing liveness test.

          PANA-Ping-Request ::= < PANA-Header: 6, REQ >
                           *  [ AVP ]
                          0*1 < AUTH >
     */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PPR;
    msg->flags().request = true;
    msg->sessionId() = this->SessionId();

    // adjust serial num
    ++ LastTxSeqNum();
    msg->seq() = LastTxSeqNum().Value();

    // auth avp if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPPR: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Session::RxPUA()
{
    /*
      7.17.  PANA-Update-Answer (PUA)

         The PANA-Update-Answer (PUA) message is sent by the PAA (PaC) to the
         PaC (PAA) in response to a PANA-Update-Request from the PaC (PAA).

         PANA-Update-Answer ::= < PANA-Header: 9 >
                          *  [ AVP ]
                         0*1 < AUTH >
     */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPUA: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    // RtxTimerStop()
    m_Timer.CancelTxRetry();
}

void PANA_Session::RxPPR()
{
    /*
       7.10.  PANA-Ping-Request (PPR)

          The PANA-Ping-Request (PPR) message is either sent by the PaC or the
          PAA for performing liveness test.

          PANA-Ping-Request ::= < PANA-Header: 6, REQ >
                           *  [ AVP ]
                          0*1 < AUTH >
     */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPPR: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    TxPPA();
}

void PANA_Session::RxPUR()
{
    /*
      7.16.  PANA-Update-Request (PUR)

         The PANA-Update-Request (PUR) message is sent either by the PaC or
         the PAA to deliver attribute updates.  In the scope of this
         specification only the IP address the PaC can be updated via this
         message.

         PANA-Update-Request ::= < PANA-Header: 9, REQ >
                          *  [ AVP ]
                         0*1 < AUTH >
     */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPUR: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    TxPUA();
}

void PANA_Session::TxPPA()
{
    /*
      7.11.  PANA-Ping-Answer (PPA)

         The PANA-Ping-Answer (PPA) message is sent in response to a
         PANA-Ping-Request.

         PANA-Ping-Answer ::= < PANA-Header: 6 >
                          *  [ AVP ]
                         0*1 < AUTH >
     */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PPA;
    msg->seq() = LastRxSeqNum().Value();
    msg->sessionId() = this->SessionId();

    // auth avp if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPPA: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Session::RxPPA()
{
    /*
      7.11.  PANA-Ping-Answer (PPA)

         The PANA-Ping-Answer (PPA) message is sent in response to a
         PANA-Ping-Request.

         PANA-Ping-Answer ::= < PANA-Header: 6 >
                          *  [ AVP ]
                         0*1 < AUTH >
     */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPPA: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    m_Timer.CancelTxRetry();
}

void PANA_Session::TxPTR(ACE_UINT32 cause)
{
    /*
      7.12.  PANA-Termination-Request (PTR)

         The PANA-Termination-Request (PTR) message is sent either by the PaC
         or the PAA to terminate a PANA session.

         PANA-Termination-Request ::= < PANA-Header: 7, REQ >
                             < Termination-Cause >
                          *  [ AVP ]
                         0*1 < AUTH >
     */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PTR;
    msg->flags().request = true;
    msg->sessionId() = this->SessionId();

    // adjust serial num
    ++ LastTxSeqNum();
    msg->seq() = LastTxSeqNum().Value();

    // termination cause
    PANA_UInt32AvpWidget causeAvp(PANA_AVPNAME_TERMCAUSE);
    causeAvp.Get() = ACE_HTONL(cause);
    msg->avpList().add(causeAvp());

    // auth avp if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPTR: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    // session timer
    m_Timer.CancelSession();

    SendReqMsg(msg);
}

void PANA_Session::RxPTR()
{
    /*
      7.12.  PANA-Termination-Request (PTR)

         The PANA-Termination-Request (PTR) message is sent either by the PaC
         or the PAA to terminate a PANA session.

         PANA-Termination-Request ::= < PANA-Header: 7, REQ >
                             < Termination-Cause >
                          *  [ AVP ]
                         0*1 < AUTH >
     */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPTR: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    TxPTA();

    // termination cause
    PANA_UInt32AvpContainerWidget causeAvp(msg.avpList());
    pana_unsigned32_t *cause = causeAvp.GetAvp(PANA_AVPNAME_TERMCAUSE);
    if (cause) {
        Disconnect(ACE_NTOHL(*cause));
    }
    else {
        Disconnect(PANA_TERMCAUSE_ADMINISTRATIVE);
    }
}

void PANA_Session::TxPTA()
{
    /*
      7.13.  PANA-Termination-Answer (PTA)

         The PANA-Termination-Answer (PTA) message is sent either by the PaC
         or the PAA in response to PANA-Termination-Request.

         PANA-Termination-Answer ::= < PANA-Header: 7 >
                          *  [ AVP ]
                         0*1 < AUTH >
     */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PTA;
    msg->seq() = LastRxSeqNum().Value();
    msg->sessionId() = this->SessionId();

    // auth avp if any
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    AAA_LOG((LM_INFO, "(%P|%t) TxPTA: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Session::RxPTA()
{
    /*
      7.13.  PANA-Termination-Answer (PTA)

         The PANA-Termination-Answer (PTA) message is sent either by the PaC
         or the PAA in response to PANA-Termination-Request.

         PANA-Termination-Answer ::= < PANA-Header: 7 >
                          *  [ AVP ]
                         0*1 < AUTH >
     */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPTA: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    Disconnect(PANA_TERMCAUSE_LOGOUT);
}

void PANA_Session::TxPER(pana_unsigned32_t rcode)
{
    /*
      7.14.  PANA-Error-Request (PER)

         The PANA-Error-Request (PER) message is sent either by the PaC or the
         PAA to report an error with the last received PANA message.  This
         message MUST contain one Failed-Message-Header AVP which carries the
         content of the PANA message header of the erroneous message.

         PANA-Error-Request ::= < PANA-Header: 8, REQ >
                              < Result-Code >
                              { Failed-Message-Header }
                           *  [ Failed-AVP ]
                           *  [ AVP ]
                          0*1 < AUTH >
    */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PER;
    msg->flags().request = true;
    msg->sessionId() = this->SessionId();

    // adjust serial num
    ++ LastTxSeqNum();
    msg->seq() = LastTxSeqNum().Value();

    // add result-code
    PANA_UInt32AvpWidget rcodeAvp(PANA_AVPNAME_RESULTCODE);
    rcodeAvp.Get() = ACE_HTONL(rcode);
    msg->avpList().add(rcodeAvp());

    // add Failed-Message-Header
    PANA_StringAvpWidget failedHeaderAvp(PANA_AVPNAME_FAILEDMSGHDR);
    failedHeaderAvp.Get().assign((char*)&LastRxHeader(),
                                 PANA_MsgHeader::HeaderLength);
    msg->avpList().add(failedHeaderAvp());

    // auth avp
    if (SecurityAssociation().Auth().IsSet()) {
        SecurityAssociation().AddAuthAvp(*msg);
    }

    // RtxTimerStop()
    m_Timer.CancelEapResponse();

    AAA_LOG((LM_INFO, "(%P|%t) TxPER: [RCODE=%d] id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendReqMsg(msg);
}

void PANA_Session::RxPER()
{
    /*
      7.14.  PANA-Error-Request (PER)

         The PANA-Error-Request (PER) message is sent either by the PaC or the
         PAA to report an error with the last received PANA message.  This
         message MUST contain one Failed-Message-Header AVP which carries the
         content of the PANA message header of the erroneous message.

         PANA-Error-Request ::= < PANA-Header: 8, REQ >
                              < Result-Code >
                              { Failed-Message-Header }
                           *  [ Failed-AVP ]
                           *  [ AVP ]
                          0*1 < AUTH >
    */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPER: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    // result-code
    PANA_UInt32AvpContainerWidget rcodeAvp(msg.avpList());
    pana_unsigned32_t *rcode = rcodeAvp.GetAvp(PANA_AVPNAME_RESULTCODE);
    if (rcode == NULL) {
        throw (PANA_Exception(PANA_Exception::FAILED, 
               "No Result-Code AVP in PER message"));
    }

    Error(ACE_NTOHL(*rcode));

    // TBD: Process Failed-AVP and Failed-Message-Header
}

void PANA_Session::TxPEA()
{
    /*
       7.15.  PANA-Error-Answer (PEA)

          The PANA-Error-Answer (PEA) message is sent in response to a
          PANA-Error-Request.

          PANA-Error-Answer ::= < PANA-Header: 8 >
                            *  [ AVP ]
                           0*1 < AUTH >
    */
    boost::shared_ptr<PANA_Message> msg(new PANA_Message);

    // Populate header
    msg->type() = PANA_MTYPE_PEA;
    msg->seq() = LastRxSeqNum().Value();
    msg->sessionId() = this->SessionId();

    AAA_LOG((LM_INFO, "(%P|%t) TxPEA: id=%d seq=%d\n", msg->sessionId(), msg->seq()));

    SendAnsMsg(msg);
}

void PANA_Session::RxPEA()
{
    /*
       7.15.  PANA-Error-Answer (PEA)

          The PANA-Error-Answer (PEA) message is sent in response to a
          PANA-Error-Request.

          PANA-Error-Answer ::= < PANA-Header: 8 >
                            *  [ AVP ]
                           0*1 < AUTH >
    */
    std::auto_ptr<PANA_Message> cleanup(AuxVariables().
        RxMsgQueue().Dequeue());
    PANA_Message &msg = *cleanup;

    AAA_LOG((LM_INFO, "(%P|%t) RxPEA: id=%d seq=%d\n", msg.sessionId(), msg.seq()));

    m_Timer.CancelTxRetry();

    Disconnect();
}

void PANA_Session::RxValidateMsg(PANA_Message &msg,
                                 bool skipAuth)
{
   bool doUpdate = true;

   // validate seq number
   if (msg.flags().request) {
       if (LastRxSeqNum() == 0) {
           // update the seq number
           LastRxSeqNum() = msg.seq();
           doUpdate = false;
       }
       else if (LastRxSeqNum() == msg.seq()) {
           if (msg.type() == CachedAnsMsg()->type()) {
               // re-transmitt cached answer message
               TxLastAnsMsg();
               throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                      "Re-transmitting answer message"));
           }
           else {
               throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                      "unexpected request msg with invalid seq number"));
           }
       }
       else if (msg.seq() != (LastRxSeqNum().Value() + 1)) {
           throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                  "request msg with invalid seq number"));
       }
   }
   else if (msg.seq() != LastTxSeqNum().Value()) {
       throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
              "answer msg with invalid seq number"));
   }

   // validate auth-avp
   if (! skipAuth) {
       PANA_StringAvpContainerWidget authAvp(msg.avpList());
       if (authAvp.GetAvp(PANA_AVPNAME_AUTH) && SecurityAssociation().Auth().IsSet()) {
           if (SecurityAssociation().ValidateAuthAvp(msg) == false) {
                throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE, 
                       "PANA session received msg with invalid AUTH value"));
           }
           AAA_LOG((LM_INFO, "(%P|%t) Auth validated\n"));
       }
   }

   // verify the session id
   if ((msg.type() != PANA_MTYPE_PCI) && (msg.sessionId() != SessionId())) {
       throw (PANA_Exception(PANA_Exception::INVALID_MESSAGE,
              "Received invalid session id"));
   }

   // wait till all validation happens before
   // we update seq number
   if (msg.flags().request && doUpdate) {
       ++ LastRxSeqNum();
   }
}

bool PANA_Session::TxLastReqMsg()
{
    if (m_Timer.ReScheduleTxRetry()) {

        AAA_LOG((LM_INFO, "(%P|%t) Re-transmitting last request\n"));

        // reset the master list
        LastTxReqMsg()->avpList().reset();

        // re-send last req msg
        TxPrepareMessage(*LastTxReqMsg());
        m_TxChannel.Send(LastTxReqMsg());
        return (true);
    }
    else {
        AAA_LOG((LM_INFO, "(%P|%t) Re-transmission giving up\n"));
    }
    return (false);
}

bool PANA_Session::TxLastAnsMsg()
{
    AAA_LOG((LM_INFO, "(%P|%t) Re-transmitting last answer\n"));

    // reset the master list
    CachedAnsMsg()->avpList().reset();

    // re-transmitt cached answer message
    TxPrepareMessage(*CachedAnsMsg());
    m_TxChannel.Send(CachedAnsMsg());
    return (true);
}

void PANA_Session::TxPrepareMessage(PANA_Message &msg)
{
}

void PANA_Session::SendReqMsg(boost::shared_ptr<PANA_Message> msg,
                              bool allowRetry)
{
    TxPrepareMessage(*msg);
    m_TxChannel.Send(msg);
    if (allowRetry) {
       m_Timer.ScheduleTxRetry();
    }
    LastTxReqMsg()= msg;
}

void PANA_Session::SendAnsMsg(boost::shared_ptr<PANA_Message> msg) 
{
    TxPrepareMessage(*msg);
    m_TxChannel.Send(msg);
    CachedAnsMsg() = msg;
}

void PANA_Session::Reset() 
{
    m_SA.Reset();
    m_AuxVariables.Reset();
    PANA_SessionAttribute::Reset();
}

void PANA_Session::Disconnect(ACE_UINT32 cause)
{
   AAA_LOG((LM_INFO, "(%P|%t) Disconnect: cause=%d\n",
       cause));

   m_Timer.CancelTxRetry();
   m_Timer.CancelSession();
   m_Timer.CancelEapResponse();
   m_Event.Disconnect(cause);
   Reset();
}

void PANA_Session::Error(ACE_UINT32 resultCode)
{
   AAA_LOG((LM_INFO, "(%P|%t) Error: result-code=%d\n",
       resultCode));

   m_Timer.CancelTxRetry();
   m_Timer.CancelSession();
   m_Timer.CancelEapResponse();
   m_Event.Error(resultCode);
   Reset();
}

void PANA_AuxillarySessionVariables::Reset()
{
   m_Authorized = false;
   m_AlgorithmIsSet = false;

   while (! RxMsgQueue().Empty()) {
       std::auto_ptr<PANA_Message> cleanup
                    (RxMsgQueue().Dequeue());
   }
   while (! TxEapMessageQueue().Empty()) {
      PANA_MsgBlockGuard eapPkt(TxEapMessageQueue().Dequeue());
   }
}

void PANA_SessionAttribute::Reset()
{
   m_SessionId = 0;
   m_LastTxSeqNum.Reset();
   m_LastRxSeqNum = 0;
   m_SessionLifetime = PANA_CFG_GENERAL().m_SessionLifetime;
}
