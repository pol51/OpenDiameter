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

#ifndef __AAA_SESSION_SERVER_INL__
#define __AAA_SESSION_SERVER_INL__

#include "aaa_session_db.h"

template <class REC_STORAGE>
DiameterServerAcctSession<REC_STORAGE>::DiameterServerAcctSession
(AAA_Task &task, diameter_unsigned32_t id, bool stateful) :
    DiameterAcctSession(id),
    m_Stateful(stateful),
    m_Fsm(task, *this, m_RecStorage) 
{
    if (m_Stateful) {
       Attributes().RealtimeRequired().Set
             (DIAMETER_ACCT_REALTIME_DELIVER_AND_GRANT);
       m_Fsm.Start();
    }
}

template <class REC_STORAGE>
AAAReturnCode DiameterServerAcctSession<REC_STORAGE>::Send
(std::auto_ptr<DiameterMsg> msg) 
{
    ////        !!!! WARNING !!!!
    //// un-used for current accounting application
    //// For backward compatiblity only
    ////

    return TxDelivery(msg);
}

template <class REC_STORAGE>
void DiameterServerAcctSession<REC_STORAGE>::RxRequest
(std::auto_ptr<DiameterMsg> msg) 
{
    // validate messge
    if (msg->hdr.code != DIAMETER_MSGCODE_ACCOUNTING) {
        AAA_LOG((LM_INFO, "(%P|%t) Non-accounting request message received, discarding\n"));
        DiameterMsgHeaderDump::Dump(*msg);
        return;
    }

    // filter session id
    if (Attributes().SessionId().IsEmpty()) {
        DiameterSessionId sid;
        if (sid.Get(*msg)) {
            AAA_LOG((LM_DEBUG,"(%P|%t) ERROR: Fatal, failed session id\n"));
            return;
        }
        Attributes().SessionId() = sid;
        AAA_LOG((LM_DEBUG,"(%P|%t) New acct session\n"));
        sid.Dump();
    }

    // filter sub-session id
    DiameterUInt64AvpContainerWidget subSessionIdAvp(msg->acl);
    diameter_unsigned64_t *subSid = subSessionIdAvp.GetAvp
                 (DIAMETER_AVPNAME_ACCTSUBSID);
    if (subSid) {
        Attributes().SubSessionId() = *subSid;
    }

    // filter record number
    DiameterUInt32AvpContainerWidget recNumAvp(msg->acl);
    diameter_unsigned32_t *recNum = recNumAvp.GetAvp
                 (DIAMETER_AVPNAME_ACCTREC_NUM);
    if (recNum) {
        Attributes().RecordNumber() = *recNum;
    }

    // filter record-type
    DiameterEnumAvpContainerWidget recTypeAvp(msg->acl);
    diameter_enumerated_t *recType = recTypeAvp.GetAvp
                 (DIAMETER_AVPNAME_ACCTREC_TYPE);
    if (recType) {
        Attributes().RecordType() = *recType;
    }

    // filter RADIUS accounting id    
    DiameterStringAvpContainerWidget radiusIdAvp(msg->acl);
    diameter_octetstring_t *radius = radiusIdAvp.GetAvp
             (DIAMETER_AVPNAME_ACCTSID);
    if (radius) {
        Attributes().RadiusAcctSessionId() = *radius;
    }

    // filter multi-session id
    DiameterUtf8AvpContainerWidget multiSidAvp(msg->acl);
    diameter_utf8string_t *multi = multiSidAvp.GetAvp
             (DIAMETER_AVPNAME_ACCTMULTISID);
    if (multi) {
        Attributes().MultiSessionId() = *multi;
    }

    // set the realtime required
    if (! Attributes().RealtimeRequired().IsNegotiated()) {
        DiameterScholarAttribute<diameter_enumerated_t> rt;
        this->SetRealTimeRequired(rt);
        if (rt.IsSet()) {
            Attributes().RealtimeRequired().Set(rt());
        }
        else {
            Attributes().RealtimeRequired().IsNegotiated() = true;
        }
    }

    // set the interim interval
    if (! Attributes().InterimInterval().IsNegotiated()) {
        DiameterScholarAttribute<diameter_unsigned32_t> ival;
        this->SetInterimInterval(ival);
        if (ival.IsSet()) {
            Attributes().InterimInterval().Set(ival());
        }
        else {
            Attributes().InterimInterval().IsNegotiated() = true;
        }
    }

    m_Fsm.RxACR(*msg);

    // formulate proper event
    if (m_Stateful) {
        if (! RecStorage().IsSpaceAvailableOnDevice()) {
            m_Fsm.Notify(DIAMETER_SESSION_ACCT_EV_RX_ACR_NO_BUF);
            return;
        }
	switch (Attributes().RecordType()()) {
           case DIAMETER_ACCT_RECTYPE_EVENT:
              m_Fsm.Notify(DIAMETER_SESSION_ACCT_EV_RX_ACR_EV_OK, msg);
              break;
           case DIAMETER_ACCT_RECTYPE_START:
              m_Fsm.Notify(DIAMETER_SESSION_ACCT_EV_RX_ACR_START_OK, msg);
              break;
           case DIAMETER_ACCT_RECTYPE_INTERIM:
              m_Fsm.Notify(DIAMETER_SESSION_ACCT_EV_RX_ACR_INT_OK, msg);
              break;
           case DIAMETER_ACCT_RECTYPE_STOP:
              m_Fsm.Notify(DIAMETER_SESSION_ACCT_EV_RX_ACR_STOP_OK, msg);
              break;
	}
    }
    else {
        RecStorage().StoreRecord(msg->acl, 
                                 Attributes().RecordType()(),
                                 Attributes().RecordNumber()());
        if (RecStorage().IsSpaceAvailableOnDevice()) {
            m_Fsm.TxACA(AAA_SUCCESS);
            Success();
        }
        else {
           m_Fsm.TxACA(AAA_OUT_OF_SPACE);
        }
    }
}

template <class REC_STORAGE>
void DiameterServerAcctSession<REC_STORAGE>::RxAnswer
(std::auto_ptr<DiameterMsg> msg) 
{
    AAA_LOG((LM_INFO, "(%P|%t) Service specific answer msg received in server, discarding\n"));
    DiameterMsgHeaderDump::Dump(*msg);
}

template <class REC_STORAGE>
void DiameterServerAcctSession<REC_STORAGE>::RxError
(std::auto_ptr<DiameterMsg> msg) 
{
    ErrorMsg(*msg);
}

template <class REC_STORAGE>
AAAReturnCode DiameterServerAcctSession<REC_STORAGE>::Reset()
{
    DiameterAcctSession::Reset();
    DIAMETER_SESSION_DB().Remove(Attributes().SessionId());
    m_Fsm.Stop();

    // WARNING!!!: schedule this object for destruction
    DIAMETER_ACCT_SESSION_GC().ScheduleForDeletion(*this);
    return (AAA_ERR_SUCCESS);
}

#endif
