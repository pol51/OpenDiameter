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

#include "pana_channel.h"
#include "pana_session.h"
#include "pana_memory_manager.h"

void PANA_Channel::Open(ACE_INET_Addr &addr)
{
    if (m_Socket.open(addr) < 0) {
        AAA_LOG((LM_ERROR, "(%P|%t) Failed to open socket\n"));
        throw (PANA_Exception(PANA_Exception::TRANSPORT_FAILED,
                              "Failed to open device"));
    }
    SetLocalAddr() = addr;
    PANA_IngressReceiver::Start();
}

void PANA_Channel::Close()
{
    m_Socket.close();
    PANA_IngressReceiver::Stop();
}

void PANA_Channel::Send(boost::shared_ptr<PANA_Message> m)
{
    PANA_MessageBuffer *rawBuf = PANA_MESSAGE_POOL()->malloc();

    PANA_HeaderParser hp;
    hp.setRawData(rawBuf);
    hp.setAppData(static_cast<PANA_MsgHeader*>(m.get()));

    hp.parseAppToRaw();

    // Parse the payload
    PANA_PayloadParser pp;
    pp.setRawData(rawBuf);
    pp.setAppData(&(m->avpList()));
    pp.setDictData(hp.getDictData());

    pp.parseAppToRaw();

    // Re-do the header again to set the length
    m->length() = rawBuf->wr_ptr() - rawBuf->base();
    hp.parseAppToRaw();

    /* --- send the message --- */
    AAA_MutexScopeLock lock(m_SocketMtx);
    if (m_Socket.send(rawBuf->base(), m->length(),
                      m->destAddress()) < 0) {
        AAA_LOG((LM_ERROR, "(%P|%t) Transmit error [%s]\n",
                       strerror(errno)));
    }

    PANA_MESSAGE_POOL()->free(rawBuf);
}

