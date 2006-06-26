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

#include "pana_egress.h"
#include "pana_memory_manager.h"

int PANA_EgressSender::Serve()
{
    PANA_MessageBuffer *rawBuf = PANA_MESSAGE_POOL()->malloc();

    PANA_HeaderParser hp;
    AAADictionaryOption opt(PARSE_STRICT, PANA_DICT_PROTOCOL_ID);
    hp.setRawData(reinterpret_cast<AAAMessageBlock*>(rawBuf));
    hp.setAppData(static_cast<PANA_MsgHeader*>(m_Msg.get()));
    hp.setDictData(&opt);

    hp.parseAppToRaw();

    // Parse the payload
    PANA_PayloadParser pp;
    pp.setRawData(reinterpret_cast<AAAMessageBlock*>(rawBuf));
    pp.setAppData(&(m_Msg->avpList()));
    pp.setDictData(m_Msg->getDictHandle());

    pp.parseAppToRaw();

    // Re-do the header again to set the length
    m_Msg->length() = rawBuf->wr_ptr() - rawBuf->base();
    hp.parseAppToRaw();

    /* --- send the message --- */
    int retry = 0;
    for (; retry < RETRY_COUNT; retry ++) {
        if (m_IO().send(rawBuf->base(), m_Msg->length(), 
                        m_Msg->srcPort(), m_Msg->srcDevices()) < 0) {
            ACE_DEBUG((LM_ERROR, "(%P|%t) Retransmitt error: %s, retrying\n",
                       strerror(errno)));
            try {
                for (int reopen = 0; reopen < RETRY_COUNT; reopen ++) {
                    if (m_IO.ReOpen()) {
                        throw (0);
                    }
                    ACE_Time_Value tout(1, 0);
                    ACE_OS::sleep(tout);
                }                
                ACE_DEBUG((LM_ERROR, "(%P|%t) Unable to re-open socket, giving-up\n"));
                break;
            } catch (...) {
            }
        }
        else {
            break;
        }
    }
    if (retry == RETRY_COUNT) {
        ACE_DEBUG((LM_ERROR, "(%P|%t) Unable to send message, giving-up\n"));
    }

    PANA_MESSAGE_POOL()->free(rawBuf);
    Release(2);
    return (0);
}

