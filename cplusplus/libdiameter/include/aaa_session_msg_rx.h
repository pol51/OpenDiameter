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

#ifndef __AAA_SESSION_MSG_RX_H__
#define __AAA_SESSION_MSG_RX_H__

#include "aaa_session_server_factory.h"
#include "aaa_route_msg_router.h"

class DIAMETERBASEPROTOCOL_EXPORT AAA_ProxyHandler
{
    public:
        AAA_ProxyHandler(diameter_unsigned32_t appId) :
            m_ApplicationId(appId) {
        }
        virtual ~AAA_ProxyHandler() {
        }

        /// This function is called when incomming request message is received
        virtual AAAReturnCode RequestMsg(AAAMessage &msg) {
            // Incomming request messages are received here.
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called when incomming answer message is received
        virtual AAAReturnCode AnswerMsg(AAAMessage &msg) {
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called when incomming error message is received
        virtual AAAReturnCode ErrorMsg(AAAMessage &msg) {
            return (AAA_ERR_SUCCESS);
        }

        diameter_unsigned32_t &ApplicationId() {
            return m_ApplicationId;
        }

    protected:
        diameter_unsigned32_t m_ApplicationId;
};

class DIAMETERBASEPROTOCOL_EXPORT AAA_SessionFactoryMap : 
    private std::map<diameter_unsigned32_t, AAA_ServerSessionFactory*>
{
    private:
        typedef std::map<diameter_unsigned32_t, AAA_ServerSessionFactory*>::iterator 
            MapIterator;
    public:
        bool Add(AAA_ServerSessionFactory &factory) {
           insert(std::pair<diameter_unsigned32_t, AAA_ServerSessionFactory*>
                (factory.GetApplicationId(), &factory));
           return (true);
        }
        AAA_ServerSessionFactory *Lookup(diameter_unsigned32_t appId) {
           MapIterator i = find(appId);
           return (i == end()) ? NULL : i->second;
        }
        bool Remove(diameter_unsigned32_t appId) {
           MapIterator i = find(appId);
           if (i != end()) {
               erase(i);
               return (true);
           }
           return (false);
        }
        bool Empty() {
            return std::map<diameter_unsigned32_t, 
                AAA_ServerSessionFactory*>::empty();
        }
};

class DIAMETERBASEPROTOCOL_EXPORT AAA_ProxyHandlerMap : 
    private std::map<diameter_unsigned32_t, AAA_ProxyHandler*> 
{
    private:
        typedef std::map<diameter_unsigned32_t, AAA_ProxyHandler*>::iterator 
            MapIterator;
    public:
        bool Add(AAA_ProxyHandler &handler) {
           insert(std::pair<diameter_unsigned32_t, AAA_ProxyHandler*>
                (handler.ApplicationId(), &handler));
           return (true);
        }
        AAA_ProxyHandler *Lookup(diameter_unsigned32_t appId) {
           MapIterator i = find(appId);
           return (i == end()) ? NULL : i->second;
        }
        bool Remove(diameter_unsigned32_t appId) {
           MapIterator i = find(appId);
           if (i != end()) {
               erase(i);
               return (true);
           }
           return (false);
        }
};

class AAA_SessionMsgRx
{
    public:
       AAA_SessionMsgRx() :
           m_LocalHandler(*this),
           m_ProxyHandler(*this),
           m_ErrorHandler(*this) {
           AAA_MSG_ROUTER()->RegisterHandler
               (AAA_MsgRouterHandlerTable::H_LOCAL, &m_LocalHandler);
           AAA_MSG_ROUTER()->RegisterHandler
               (AAA_MsgRouterHandlerTable::H_PROXY, &m_ProxyHandler);
           AAA_MSG_ROUTER()->RegisterHandler
               (AAA_MsgRouterHandlerTable::H_ERROR, &m_ErrorHandler);
       }
       virtual ~AAA_SessionMsgRx() {
           AAA_MSG_ROUTER()->RemoveHandler(AAA_MsgRouterHandlerTable::H_LOCAL);
           AAA_MSG_ROUTER()->RemoveHandler(AAA_MsgRouterHandlerTable::H_PROXY);
           AAA_MSG_ROUTER()->RemoveHandler(AAA_MsgRouterHandlerTable::H_ERROR);
       }
       AAA_SessionFactoryMap &SessionFactoryMap() {
           return m_SessionFactoryMap;
       }
       AAA_ProxyHandlerMap &ProxyHandlerMap() {
           return m_ProxyHandlerMap;
       }

    protected:
       virtual AAAReturnCode RxUnknownSession(std::auto_ptr<AAAMessage> msg);

    protected:
       class RxLocalMsgHandler : public AAA_MsgRouterHandler {
           public:
              RxLocalMsgHandler(AAA_SessionMsgRx &rx) :
                  m_SessionRx(rx) {
              }
              int Request(std::auto_ptr<AAAMessage> &msg,
                          AAA_PeerEntry *source,
                          AAA_PeerEntry *dest);
              int Answer(std::auto_ptr<AAAMessage> &msg,
                         AAA_PeerEntry *source,
                         AAA_PeerEntry *dest);
           private:
              AAA_SessionMsgRx &m_SessionRx;
       };
       class RxProxyMsgHandler : public AAA_MsgRouterHandler {
           public:
              RxProxyMsgHandler(AAA_SessionMsgRx &rx) :
                  m_SessionRx(rx) {
              }
              int Request(std::auto_ptr<AAAMessage> &msg,
                          AAA_PeerEntry *source,
                          AAA_PeerEntry *dest);
              int Answer(std::auto_ptr<AAAMessage> &msg,
                          AAA_PeerEntry *source,
                         AAA_PeerEntry *dest);
           private:
              AAA_SessionMsgRx &m_SessionRx;
       };
       class RxErrorMsgHandler : public AAA_MsgRouterHandler {
           public:
              RxErrorMsgHandler(AAA_SessionMsgRx &rx) :
                  m_SessionRx(rx) {
              }
              int Request(std::auto_ptr<AAAMessage> &msg,
                          AAA_PeerEntry *source,
                          AAA_PeerEntry *dest) {
                  AAA_LOG(LM_DEBUG, "(%P|%t) **** Request Message Error ****\n");
                  AAA_MsgDump::Dump(*msg);
                  return LocalErrorHandling(msg, source, dest);
              }
              int Answer(std::auto_ptr<AAAMessage> &msg,
                         AAA_PeerEntry *source,
                         AAA_PeerEntry *dest) {
                  AAA_LOG(LM_DEBUG, "(%P|%t) **** Answer Message Error ****\n");
                  AAA_MsgDump::Dump(*msg);
                  return LocalErrorHandling(msg, source, dest);
              }
              int LocalErrorHandling(std::auto_ptr<AAAMessage> &msg,
                                     AAA_PeerEntry *source,
                                     AAA_PeerEntry *dest);
           private:
              AAA_SessionMsgRx &m_SessionRx;
       };

    private:
       void TxASA(std::auto_ptr<AAAMessage> &msg);

    private:
       AAA_SessionFactoryMap m_SessionFactoryMap;
       AAA_ProxyHandlerMap m_ProxyHandlerMap;
       RxLocalMsgHandler m_LocalHandler;
       RxProxyMsgHandler m_ProxyHandler;
       RxErrorMsgHandler m_ErrorHandler;

       friend class RxLocalMsgHandler;
       friend class RxProxyMsgHandler;
       friend class RxErrorMsgHandler;
};

#endif // __AAA_SESSION_MSG_RX_H__
