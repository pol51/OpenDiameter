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
        virtual AAAReturnCode RequestMsg(DiameterMsg &msg) {
            // Incomming request messages are received here.
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called when incomming answer message is received
        virtual AAAReturnCode AnswerMsg(DiameterMsg &msg) {
            return (AAA_ERR_SUCCESS);
        }

        /// This function is called when incomming error message is received
        virtual AAAReturnCode ErrorMsg(DiameterMsg &msg) {
            return (AAA_ERR_SUCCESS);
        }

        diameter_unsigned32_t &ApplicationId() {
            return m_ApplicationId;
        }

    protected:
        diameter_unsigned32_t m_ApplicationId;
};

class DIAMETERBASEPROTOCOL_EXPORT DiameterSessionFactoryMap : 
    private std::map<diameter_unsigned32_t, DiameterServerSessionFactory*>
{
    private:
        typedef std::map<diameter_unsigned32_t, DiameterServerSessionFactory*>::iterator 
            MapIterator;
    public:
        bool Add(DiameterServerSessionFactory &factory) {
           insert(std::pair<diameter_unsigned32_t, DiameterServerSessionFactory*>
                (factory.GetApplicationId(), &factory));
           return (true);
        }
        DiameterServerSessionFactory *Lookup(diameter_unsigned32_t appId) {
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
                DiameterServerSessionFactory*>::empty();
        }
};

class DIAMETERBASEPROTOCOL_EXPORT DiameterProxyHandlerMap : 
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

class DiameterSessionMsgRx
{
    public:
       DiameterSessionMsgRx() :
           m_LocalHandler(*this),
           m_ProxyHandler(*this),
           m_ErrorHandler(*this) {
           DIAMETER_MSG_ROUTER()->RegisterHandler
               (DiameterMsgRouterHandlerTable::H_LOCAL, &m_LocalHandler);
           DIAMETER_MSG_ROUTER()->RegisterHandler
               (DiameterMsgRouterHandlerTable::H_PROXY, &m_ProxyHandler);
           DIAMETER_MSG_ROUTER()->RegisterHandler
               (DiameterMsgRouterHandlerTable::H_ERROR, &m_ErrorHandler);
       }
       virtual ~DiameterSessionMsgRx() {
           DIAMETER_MSG_ROUTER()->RemoveHandler(DiameterMsgRouterHandlerTable::H_LOCAL);
           DIAMETER_MSG_ROUTER()->RemoveHandler(DiameterMsgRouterHandlerTable::H_PROXY);
           DIAMETER_MSG_ROUTER()->RemoveHandler(DiameterMsgRouterHandlerTable::H_ERROR);
       }
       DiameterSessionFactoryMap &SessionFactoryMap() {
           return m_SessionFactoryMap;
       }
       DiameterProxyHandlerMap &ProxyHandlerMap() {
           return m_ProxyHandlerMap;
       }

    protected:
       virtual AAAReturnCode RxUnknownSession(std::auto_ptr<DiameterMsg> msg);

    protected:
       class RxLocalMsgHandler : public DiameterMsgRouterHandler {
           public:
              RxLocalMsgHandler(DiameterSessionMsgRx &rx) :
                  m_SessionRx(rx) {
              }
              int Request(std::auto_ptr<DiameterMsg> &msg,
                          DiameterPeerEntry *source,
                          DiameterPeerEntry *dest);
              int Answer(std::auto_ptr<DiameterMsg> &msg,
                         DiameterPeerEntry *source,
                         DiameterPeerEntry *dest);
           private:
              DiameterSessionMsgRx &m_SessionRx;
       };
       class RxProxyMsgHandler : public DiameterMsgRouterHandler {
           public:
              RxProxyMsgHandler(DiameterSessionMsgRx &rx) :
                  m_SessionRx(rx) {
              }
              int Request(std::auto_ptr<DiameterMsg> &msg,
                          DiameterPeerEntry *source,
                          DiameterPeerEntry *dest);
              int Answer(std::auto_ptr<DiameterMsg> &msg,
                          DiameterPeerEntry *source,
                         DiameterPeerEntry *dest);
           private:
              DiameterSessionMsgRx &m_SessionRx;
       };
       class RxErrorMsgHandler : public DiameterMsgRouterHandler {
           public:
              RxErrorMsgHandler(DiameterSessionMsgRx &rx) :
                  m_SessionRx(rx) {
              }
              int Request(std::auto_ptr<DiameterMsg> &msg,
                          DiameterPeerEntry *source,
                          DiameterPeerEntry *dest) {
                  AAA_LOG((LM_DEBUG, "(%P|%t) **** Request Message Error ****\n"));
                  DiameterMsgHeaderDump::Dump(*msg);
                  return LocalErrorHandling(msg, source, dest);
              }
              int Answer(std::auto_ptr<DiameterMsg> &msg,
                         DiameterPeerEntry *source,
                         DiameterPeerEntry *dest) {
                  AAA_LOG((LM_DEBUG, "(%P|%t) **** Answer Message Error ****\n"));
                  DiameterMsgHeaderDump::Dump(*msg);
                  return LocalErrorHandling(msg, source, dest);
              }
              int LocalErrorHandling(std::auto_ptr<DiameterMsg> &msg,
                                     DiameterPeerEntry *source,
                                     DiameterPeerEntry *dest);
           private:
              DiameterSessionMsgRx &m_SessionRx;
       };

    private:
       void TxASA(std::auto_ptr<DiameterMsg> &msg);

    private:
       DiameterSessionFactoryMap m_SessionFactoryMap;
       DiameterProxyHandlerMap m_ProxyHandlerMap;
       RxLocalMsgHandler m_LocalHandler;
       RxProxyMsgHandler m_ProxyHandler;
       RxErrorMsgHandler m_ErrorHandler;

       friend class RxLocalMsgHandler;
       friend class RxProxyMsgHandler;
       friend class RxErrorMsgHandler;
};

#endif // __AAA_SESSION_MSG_RX_H__

