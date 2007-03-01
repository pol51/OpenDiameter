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


#include "pana_ingress.h"
#include "pana_exceptions.h"
#include "pana_parser.h"

int PANA_IngressMsgParser::Serve()
{
    try {

       PANA_Message *parsedMsg;
       ACE_NEW_NORETURN(parsedMsg, PANA_Message);

       if (parsedMsg == NULL) {
          AAA_LOG((LM_ERROR, "(%P|%t) [INGRESS, ALLOC] App message\n"));
          throw (0);
       }

       PANA_HeaderParser hp;
       hp.setRawData(&m_Message);
       hp.setAppData(parsedMsg);

       hp.parseRawToApp(); // may throw exception
       m_Message.size(parsedMsg->length());

       PANA_PayloadParser pp;
       pp.setRawData(&m_Message);
       pp.setAppData(&(parsedMsg->avpList()));
       pp.setDictData(hp.getDictData());

       pp.parseRawToApp(); // may throw exception

       // migrate source address
       parsedMsg->srcAddress() = m_SrcAddr;

       (*m_MsgHandler)(*parsedMsg);
    }
    catch (AAAErrorCode &st) {
       AAA_LOG((LM_ERROR, "(%P|%t) [INGRESS, PARSING] parsing error\n"));
    }
    catch (PANA_Exception &e) {
       AAA_LOG((LM_ERROR, "(%P|%t) [INGRESS, RECEIVER] %s\n", 
                  e.description().data()));
    }
    catch (...) {
       AAA_LOG((LM_ERROR, "(%P|%t) [INGRESS, RECEIVER] Unknown error\n"));
    }

    // release the raw buffer and device id containers
    PANA_MESSAGE_POOL()->free(&m_Message);

    Release(2);
    return (0);
}

int PANA_IngressReceiver::Serve()
{
    m_Running = true;
    PANA_MessageBuffer *msg_buffer = NULL;
    try {
        msg_buffer = PANA_MESSAGE_POOL()->malloc();
        ACE_INET_Addr srcAddr;
        ACE_Time_Value tm(PANA_SOCKET_RECV_TIMEOUT, 0);
        ssize_t bytes = m_Socket.recv(msg_buffer->wr_ptr(),
                                      msg_buffer->size(),
                                      srcAddr, 0, &tm);
        if (bytes > 0) {
            if (m_MsgHandler == NULL) {
                AAA_LOG((LM_ERROR, "(%P|%t) [INGRESS, RECV] handler absent on %s\n",
                         m_Name.data()));
                throw (1);
            }

            PANA_IngressMsgParser *parser;
            ACE_NEW_NORETURN(parser,
                             PANA_IngressMsgParser(m_Group,
                                                   *msg_buffer,
                                                   srcAddr));
            if (parser) {
                msg_buffer->size(bytes);
                parser->RegisterHandler(*m_MsgHandler);
                if (Schedule(parser) < 0) {
                   delete parser;
                   AAA_LOG((LM_ERROR, "(%P|%t) [INGRESS, SCHEDULE] delivery job on %s\n",
                              m_Name.data()));
                   throw (0);
                }
            }
            else {
                AAA_LOG((LM_ERROR, "(%P|%t) [INGRESS, ALLOC] message parser job on %s\n",
                           m_Name.data()));
                throw (0);
            }
            if (m_Running) {
                Schedule();
            }
        }
        else if (bytes == 0) {
            throw (1);
        } else if ((errno != EAGAIN) &&
                   (errno != ETIME) &&
                   (errno != ECONNREFUSED)) {
            AAA_LOG((LM_ERROR, "(%P|%t) Receive channel error on %s : %s\n",
                       m_Name.data(), strerror(errno)));
            throw (1);
        }
        else {
            // timeout
            throw (1);
        }
    }
    catch (int rc) {
        PANA_MESSAGE_POOL()->free(msg_buffer);
        if (rc) {
            // manually re-schedule myself
            Schedule();
        }
        else {
            m_Running = false;
        }
    }
    return (0);
}

