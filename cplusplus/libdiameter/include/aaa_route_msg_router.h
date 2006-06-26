
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

#ifndef __ROUTE_MSG_ROUTER_H__
#define __ROUTE_MSG_ROUTER_H__

#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "aaa_route_framework.h"

typedef enum {
   AAA_ROUTER_RETX_DIVISOR = 2,
   AAA_ROUTER_MIN_RETX_INTERVAL = 8,
   AAA_ROUTER_MAX_RETX_COUNT    = 10
};

class AAA_MsgRouterHandler
{
   public:
       virtual int Request(std::auto_ptr<AAAMessage> &msg,
                           AAA_PeerEntry *source,
                           AAA_PeerEntry *dest) = 0;
       virtual int Answer(std::auto_ptr<AAAMessage> &msg,
                          AAA_PeerEntry *source,
                          AAA_PeerEntry *dest) = 0;

   protected:
      virtual ~AAA_MsgRouterHandler() { }
};

class AAA_MsgRouterHandlerTable
{
   /*
      Notes on the use of AAA_MsgRouterHandler. 
      In all cases, once the router has invoked
      the handler, the msg is consumed by the handler.
      The only exception is when you use H_PROXY.
      The router will not give up ownernship of
      msg to the handler so care should be given
      to its use
    */
   public:
       typedef enum {
           H_LOCAL,
           H_PROXY,
           H_ERROR,

           H_MAX
       } H_TYPE;
    
       void RegisterHandler(H_TYPE type,
                            AAA_MsgRouterHandler *h) {
           m_Handlers[type] = h;
       }
       void RemoveHandler(H_TYPE type) {
           m_Handlers[type] = NULL;
       }
       AAA_MsgRouterHandler *operator[](H_TYPE type) {
           return m_Handlers[type];
       }

   protected:
       AAA_MsgRouterHandlerTable() {
           for (int i=0; i<H_MAX; i++) {
               m_Handlers[i] = NULL;
           }
       }
       AAA_MsgRouterHandler *m_Handlers[H_MAX];
};

class AAA_MsgRouter : public AAA_RouterFramework,
                      public AAA_MsgRouterHandlerTable 
{
   public:
       class DcLocal : public AAA_DeliveryRoutingNode<AAA_MsgRouter> {
           public:
               DcLocal(AAA_MsgRouter &r) :
                   AAA_DeliveryRoutingNode<AAA_MsgRouter>(r, 0, "dcLocal") {
               }
               AAA_ROUTE_RESULT Process(std::auto_ptr<AAAMessage> msg,
                                        AAA_PeerEntry *source,
                                        AAA_PeerEntry *dest);
               int RequestMsg(std::auto_ptr<AAAMessage> msg,
                              AAA_PeerEntry *source,
                              AAA_PeerEntry *dest);
               AAA_ROUTE_RESULT ErrorHandling(std::auto_ptr<AAAMessage> msg,
                                              AAA_PeerEntry *source,
                                              AAA_PeerEntry *dest);
       };
    
       class DcForward : public AAA_DeliveryRoutingNode<AAA_MsgRouter> {
           public:
               DcForward(AAA_MsgRouter &r) :
                   AAA_DeliveryRoutingNode<AAA_MsgRouter>(r, 0, "dcForward") {
               }
               AAA_ROUTE_RESULT Process(std::auto_ptr<AAAMessage> msg,
                                        AAA_PeerEntry *source,
                                        AAA_PeerEntry *dest);
               int RequestMsg(std::auto_ptr<AAAMessage> msg,
                              AAA_PeerEntry *source,
                              AAA_PeerEntry *dest);
               int LoopDetection(std::auto_ptr<AAAMessage> &msg);
       };

       class DcRouted : public AAA_DeliveryRoutingNode<AAA_MsgRouter> {
           public:
               DcRouted(AAA_MsgRouter &r) :
                   AAA_DeliveryRoutingNode<AAA_MsgRouter>(r, 0, "dcRouted") {
               }
               AAA_ROUTE_RESULT Process(std::auto_ptr<AAAMessage> msg,
                                        AAA_PeerEntry *source,
                                        AAA_PeerEntry *dest);
               int RequestMsg(std::auto_ptr<AAAMessage> msg,
                              AAA_PeerEntry *source,
                              AAA_PeerEntry *dest);
       };
    
       class DcReject : public AAA_DeliveryRoutingNode<AAA_MsgRouter> {
           public:
               DcReject(AAA_MsgRouter &r) :
                   AAA_DeliveryRoutingNode<AAA_MsgRouter>(r, 0, "dcReject") {
               }
               AAA_ROUTE_RESULT Process(std::auto_ptr<AAAMessage> msg,
                                        AAA_PeerEntry *source,
                                        AAA_PeerEntry *dest);
               int RequestMsg(std::auto_ptr<AAAMessage> msg,
                              AAA_PeerEntry *source,
                              AAA_PeerEntry *dest);
               AAA_ROUTE_RESULT Lookup(std::auto_ptr<AAAMessage> &msg,
                                       AAA_PeerEntry *&dest);
       };

       class RcLocal : public AAA_RequestRoutingNode<DcLocal,
                       AAA_MsgRouter> {
           public:
               RcLocal(AAA_MsgRouter &r) :
                  AAA_RequestRoutingNode<DcLocal,
                                         AAA_MsgRouter>(r, 0, "rcLocal") { }
               AAA_ROUTE_RESULT Lookup(std::auto_ptr<AAAMessage> &msg,
                                       AAA_PeerEntry *&dest);
       };

       class RcForwarded : public AAA_RequestRoutingNode<DcForward,
                           AAA_MsgRouter> {
           public:
               RcForwarded(AAA_MsgRouter &r) :
                   AAA_RequestRoutingNode<DcForward,
                                          AAA_MsgRouter>(r, 0, "rcForward") {
               }
               AAA_ROUTE_RESULT Lookup(std::auto_ptr<AAAMessage> &msg,
                                       AAA_PeerEntry *&dest);
           };

       class RcRouted : public AAA_RequestRoutingNode<DcRouted,
                        AAA_MsgRouter> {
           public:
               RcRouted(AAA_MsgRouter &r) :
                   AAA_RequestRoutingNode<DcRouted,
                                          AAA_MsgRouter>(r, 0, "rcRouted") {
               }
               AAA_ROUTE_RESULT Lookup(std::auto_ptr<AAAMessage> &msg,
                                       AAA_PeerEntry *&dest);
       };

       class RcRejected : public AAA_RequestRoutingNode<DcReject,
                          AAA_MsgRouter> {
           public:
               RcRejected(AAA_MsgRouter &r) :
                   AAA_RequestRoutingNode<DcReject,
                                          AAA_MsgRouter>(r, 0, "rcReject") {
               }
               AAA_ROUTE_RESULT Lookup(std::auto_ptr<AAAMessage> &msg,
                                       AAA_PeerEntry *&dest);
       };

   protected:
       class DcRequestReTransmission : public AAA_Action<AAA_RouterPendingReqPtr> {
       	  public:
       	      DcRequestReTransmission(AAA_MsgRouter &r) : 
       	           m_Router(r) {
       	      }
              virtual void operator()(AAA_RouterPendingReqPtr &pReq) {
              	   /* 
                     The T flag is used as an indication of an application layer
                     retransmission event, e.g., due to failover to an alternate server.
                     It is defined only for request messages sent by Diameter clients or
                     agents.  For instance, after a reboot, a client may not know whether
                     it has already tried to send the accounting records in its non-
                     volatile memory before the reboot occurred.  Diameter servers MAY use
                     the T flag as an aid when processing requests and detecting duplicate
                     messages.  However, servers that do this MUST ensure that duplicates
                     are found even when the first transmitted request arrives at the
                     server after the retransmitted request.  It can be used only in cases
                     where no answer has been received from the Server for a request and
                     the request is sent again, (e.g., due to a failover to an alternate
                     peer, due to a recovered primary peer or due to a client re-sending a
                     stored record from non-volatile memory such as after reboot of a
                     client or agent).
                   */
                  pReq->m_ReqMessage->hdr.flags.t = AAA_FLG_SET;
                                    
                  AAA_LOG(LM_INFO, "(%P|%t) **** Request message re-transmission ****\n");
                  AAA_MsgDump::Dump(*(pReq->m_ReqMessage));
                  
                  ACE_Time_Value expire(AAA_CFG_TRANSPORT()->retx_interval, 0);
                  pReq->m_ReTxExpireTime = ACE_OS::gettimeofday() + expire;

              	  m_Router.RequestMsg(pReq->m_ReqMessage, pReq->m_Source);
              }
          private:
              AAA_MsgRouter &m_Router;
       };
       
   public:
   
       AAA_MsgRouter() : m_rcLocal(*this),
                         m_rcForward(*this), 
                         m_rcRouted(*this),
                         m_rcRejected(*this) {
           
           m_RequestChain.Add(&m_rcLocal);
           m_RequestChain.Add(&m_rcForward);
           m_RequestChain.Add(&m_rcRouted);
           m_RequestChain.Add(&m_rcRejected);

           m_DeliveryChain.Add(&m_rcLocal.Delivery());
           m_DeliveryChain.Add(&m_rcForward.Delivery());
           m_DeliveryChain.Add(&m_rcRouted.Delivery());
           m_DeliveryChain.Add(&m_rcRejected.Delivery());
       }
       void ReTransmitEvent(AAA_PeerEntry *peer = 0) {
           DcRequestReTransmission reTx(*this);
           if (peer) {
               m_rcForward.Delivery().ReTransmitCheck(peer, reTx);
               m_rcRouted.Delivery().ReTransmitCheck(peer, reTx);
           }
           else {
               ACE_Time_Value current = ACE_OS::gettimeofday();
               m_rcForward.Delivery().ReTransmitCheck(current, reTx);
               m_rcRouted.Delivery().ReTransmitCheck(current, reTx);
           }
       }
       
   protected:
    
       friend class DcLocal;
       friend class DcForward;
       friend class DcRouted;

       class RedirectAgent : public AAA_MsgRouterHandler {
           public:
               int Request(std::auto_ptr<AAAMessage> &msg,
                           AAA_PeerEntry *source,
                           AAA_PeerEntry *dest);
               int Answer(std::auto_ptr<AAAMessage> &msg,
                          AAA_PeerEntry *source,
                          AAA_PeerEntry *dest);
               bool IsRedirected(std::auto_ptr<AAAMessage> &msg);
       };
    
   private:
       RcLocal               m_rcLocal;
       RcForwarded           m_rcForward;
       RcRouted              m_rcRouted;
       RcRejected            m_rcRejected;
       RedirectAgent         m_RedirectAgent;
};

typedef ACE_Singleton<AAA_MsgRouter, ACE_Recursive_Thread_Mutex> AAA_MsgRouter_S;
#define AAA_MSG_ROUTER() AAA_MsgRouter_S::instance()

#endif /* __ROUTE_MSG_ROUTER_H__ */


