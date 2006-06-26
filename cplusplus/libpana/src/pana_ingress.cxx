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


#include "pana_ingress.h"
#include "pana_exceptions.h"

int PANA_IngressMsgParser::Serve()
{
    AAAMessageBlock *aBuffer = reinterpret_cast<AAAMessageBlock*>(&m_Message);
    try {

       PANA_Message *parsedMsg;
       ACE_NEW_NORETURN(parsedMsg, PANA_Message);

       if (parsedMsg == NULL) {
          ACE_DEBUG((LM_ERROR, "(%P|%t) [INGRESS, ALLOC] App message\n"));
          throw (0);
       }

       PANA_HeaderParser hp;
       AAADictionaryOption opt(PARSE_STRICT, PANA_DICT_PROTOCOL_ID);
       hp.setRawData(aBuffer);
       hp.setAppData(static_cast<PANA_MsgHeader*>(parsedMsg));
       hp.setDictData(&opt);

       hp.parseRawToApp(); // may throw exception

       PANA_PayloadParser pp;
       pp.setRawData(aBuffer);
       pp.setAppData(&(parsedMsg->avpList()));
       pp.setDictData(parsedMsg->getDictHandle());

       pp.parseRawToApp(); // may throw exception

       // migrate deivice id and port info
       parsedMsg->srcDevices().move(m_SrcDevices);
       parsedMsg->srcPort() = m_SrcPort;

       (*m_MsgHandler)(*parsedMsg);
    }
    catch (AAAErrorStatus st) {
       ACE_DEBUG((LM_ERROR, "(%P|%t) [INGRESS, PARSING] parsing error\n"));
    }
    catch (PANA_Exception &e) {
       ACE_DEBUG((LM_ERROR, "(%P|%t) [INGRESS, RECEIVER] %s\n", 
                  e.description().data())); 
    }
    catch (...) {
    }

    // release the raw buffer and device id containers
    PANA_MESSAGE_POOL()->free(&m_Message);
    delete &m_SrcDevices;
    
    Release(2);
    return (0);
}

int PANA_IngressReceiver::Serve()
{
    m_Running = true;
    ACE_UINT32 srcPort;
    PANA_MessageBuffer *msg_buffer = NULL;
    try {
        PANA_DeviceIdContainer *srcDevices;
        ACE_NEW_NORETURN(srcDevices, PANA_DeviceIdContainer);
        if (srcDevices == NULL) {
            ACE_DEBUG((LM_ERROR, "(%P|%t) [INGRESS, ALLOC] device id on %s\n",
                       m_Name.data()));
            throw (0);
        }

        msg_buffer = PANA_MESSAGE_POOL()->malloc();
        ssize_t bytes = m_IO().recv(msg_buffer->wr_ptr(), msg_buffer->size(), 
                                  srcPort, *srcDevices);
        if (bytes > 0) {
            if (m_MsgHandler == NULL) {
                ACE_DEBUG((LM_ERROR, "(%P|%t) [INGRESS, RECV] handler absent on %s\n",
                           m_Name.data()));
                throw (1);
            }

            PANA_IngressMsgParser *parser;
            ACE_NEW_NORETURN(parser, 
                             PANA_IngressMsgParser(m_Group,
                                                   *msg_buffer,
                                                   srcPort,
                                                   *srcDevices));
            if (parser) {
                msg_buffer->size(bytes);
                parser->RegisterHandler(*m_MsgHandler);
                if (Schedule(parser) < 0) {
                   delete parser;
                   ACE_DEBUG((LM_ERROR, "(%P|%t) [INGRESS, SCHEDULE] delivery job on %s\n",
                              m_Name.data()));
                   throw (0);
                }
            }
            else {
                ACE_DEBUG((LM_ERROR, "(%P|%t) [INGRESS, ALLOC] message parser job on %s\n",
                           m_Name.data()));
                throw (0);
            }
            if (m_Abort) {
                throw (0);
            }
            Schedule();
        } 
        else if (bytes == 0) {
            throw (1);
        } else if ((errno != EAGAIN) &&
                   (errno != ETIME) &&
                   (errno != ECONNREFUSED)) {
            ACE_DEBUG((LM_ERROR, "(%P|%t) Receive channel error on %s : %s, retrying\n",
                       m_Name.data(), strerror(errno)));
            if (! m_Abort) {
                m_IO.ReOpen();
            }
            throw (1);
        }
        else {
            // timeout
            throw (1);
        }
    }
    catch (int rc) {
        PANA_MESSAGE_POOL()->free(msg_buffer);
        if (rc && (! m_Abort)) {
            // manually re-schedule myself
            Schedule();
        }
        else {
            m_Running = false;
        }
    }
    return (0);
}

