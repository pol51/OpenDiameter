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

#ifndef __AAA_SESSION_CLIENT_INL__
#define __AAA_SESSION_CLIENT_INL__

template <class REC_COLLECTOR>
AAAReturnCode AAA_ClientAcctSubSession<REC_COLLECTOR>::Begin
(bool oneTime)
{
    // check if RADIUS/DIAMETER translation
    AAA_ScholarAttribute<diameter_octetstring_t> rsid;
    this->SetRadiusAcctSessionId(rsid);
    if (rsid.IsSet()) {
        Attributes().RadiusAcctSessionId() = rsid;
    }

    // check for multi-session id
    AAA_ScholarAttribute<diameter_utf8string_t> msid;
    this->SetMultiSessionId(msid);
    if (msid.IsSet()) {
        Attributes().MultiSessionId() = rsid;
    }

    // check application for realtime
    AAA_ScholarAttribute<diameter_enumerated_t> rt;
    this->SetRealTimeRequired(rt);
    if (rt.IsSet()) {
        Attributes().RealtimeRequired() = rt();
    }

    // check application for interval
    AAA_ScholarAttribute<diameter_unsigned32_t> interval;
    this->SetInterimInterval(interval);
    if (interval.IsSet()) {
        Attributes().InterimInterval() = interval();
    }

    // notification policy
    if (RecCollector().IsLastRecordInStorage()) {
        m_Fsm.Notify(AAA_SESSION_ACCT_EV_REC_IN_STORAGE);
    }
    else if (oneTime) {
        Attributes().RecordType() = AAA_ACCT_RECTYPE_EVENT;
        m_Fsm.Notify(AAA_SESSION_ACCT_EV_REQUEST_ONETIME_ACCESS);
    }
    else {
        Attributes().RecordType() = AAA_ACCT_RECTYPE_START;
        m_Fsm.Notify(AAA_SESSION_ACCT_EV_REQUEST_ACCESS);
    }    
    return (AAA_ERR_SUCCESS);
}

template <class REC_COLLECTOR>
AAAReturnCode AAA_ClientAcctSubSession<REC_COLLECTOR>::Send
(std::auto_ptr<AAAMessage> msg) 
{
   ////        !!!! WARNING !!!!
   //// un-used for current accounting application
   //// For backward compatiblity only
   ////
   if (TxDelivery(msg) != AAA_ERR_SUCCESS) {
       if (RecCollector().IsLastRecordInStorage()) {
           m_Fsm.Notify(AAA_SESSION_ACCT_EV_FTS);
       }
       else if ((Attributes().RecordType() == AAA_ACCT_RECTYPE_EVENT) ||
                (Attributes().RecordType() == AAA_ACCT_RECTYPE_STOP)) {
           if (RecCollector().IsStorageSpaceAvailable()) {
               m_Fsm.Notify(AAA_SESSION_ACCT_EV_FTS_BUF);
           }
           else {
               m_Fsm.Notify(AAA_SESSION_ACCT_EV_FTS_NO_BUF);
           }
       }
       else if (RecCollector().IsStorageSpaceAvailable() &&
                (Attributes().RealtimeRequired()() != 
                   AAA_ACCT_REALTIME_DELIVER_AND_GRANT)) {
           m_Fsm.Notify(AAA_SESSION_ACCT_EV_FTS_NOT_DAG);
       }
       else if (Attributes().RealtimeRequired()() == 
                AAA_ACCT_REALTIME_GRANT_AND_LOSE) {
           m_Fsm.Notify(AAA_SESSION_ACCT_EV_FTS_AND_GAL);
       }
       else {
           m_Fsm.Notify(AAA_SESSION_ACCT_EV_FTS_NOT_GAL);
       }
       this->Failed(Attributes().RecordNumber()());
   }
   return (AAA_ERR_SUCCESS);
}

template <class REC_COLLECTOR>
void AAA_ClientAcctSubSession<REC_COLLECTOR>::RxRequest
(std::auto_ptr<AAAMessage> msg) 
{
    AAA_LOG(LM_INFO, "(%P|%t) Service specific request msg received in client, no handlers so discarding\n");
    AAA_MsgDump::Dump(*msg);
}

template <class REC_COLLECTOR>
void AAA_ClientAcctSubSession<REC_COLLECTOR>::RxAnswer
(std::auto_ptr<AAAMessage> msg) 
{
    // validate messge
    if (msg->hdr.code != AAA_MSGCODE_ACCOUNTING) {
        AAA_LOG(LM_INFO, "(%P|%t) Non-accounting answer message received, discarding\n");
        AAA_MsgDump::Dump(*msg);
        return;
    }

    AAA_LOG(LM_INFO, "(%P|%t) accounting answer received\n");
    m_Fsm.RxACA(*msg);
     
    // filter record-type
    AAA_EnumAvpContainerWidget recTypeAvp(msg->acl);
    diameter_enumerated_t *recType = recTypeAvp.GetAvp
                 (AAA_AVPNAME_ACCTREC_TYPE);
    if (recType) {
        Attributes().RecordType() = *recType;
    }

    // filter realtime-required
    AAA_EnumAvpContainerWidget realTimeAvp(msg->acl);
    diameter_enumerated_t *realTime = realTimeAvp.GetAvp
                 (AAA_AVPNAME_ACCTREALTIME);
    if (realTime) {
        Attributes().RealtimeRequired() = *realTime;
    }

    // filter interval
    AAA_UInt32AvpContainerWidget intervalAvp(msg->acl);
    diameter_unsigned32_t *interval = intervalAvp.GetAvp
                 (AAA_AVPNAME_ACCTINTERVAL);
    if (interval) {
        Attributes().InterimInterval() = *interval;
    }

    // notification rules 
    AAA_MsgResultCode rcode(*msg);
    if (rcode.InterpretedResultCode() == 
        AAA_MsgResultCode::RCODE_SUCCESS) {
        m_Fsm.Notify(AAA_SESSION_ACCT_EV_RX_ACA_OK);
    }
    else if (RecCollector().IsLastRecordInStorage() ||
             (Attributes().RecordType() == AAA_ACCT_RECTYPE_STOP) ||
             (Attributes().RecordType() == AAA_ACCT_RECTYPE_EVENT)) {
        m_Fsm.Notify(AAA_SESSION_ACCT_EV_RX_ACA_FAIL);
        this->Failed(Attributes().RecordNumber()());
    }
    else {   
        if (Attributes().RealtimeRequired()() == 
            AAA_ACCT_REALTIME_DELIVER_AND_GRANT) {
            m_Fsm.Notify(AAA_SESSION_ACCT_EV_RX_ACA_FAIL_AND_GAL);
        }
        else {
            m_Fsm.Notify(AAA_SESSION_ACCT_EV_RX_ACA_FAIL_NOT_GAL);
        }
        this->Failed(Attributes().RecordNumber()());
    }
}

template <class REC_COLLECTOR>
void AAA_ClientAcctSubSession<REC_COLLECTOR>::RxError
(std::auto_ptr<AAAMessage> msg) 
{
    ErrorMsg(*msg);
}

template <class REC_COLLECTOR>
AAAReturnCode AAA_ClientAcctSubSession<REC_COLLECTOR>::End()
{
    Attributes().RecordType() = AAA_ACCT_RECTYPE_STOP;
    m_Fsm.CancelAllTimer();
    m_Fsm.Notify(AAA_SESSION_ACCT_EV_STOP);
    return (AAA_ERR_SUCCESS);
}

#endif













