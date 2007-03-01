
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

#ifndef __ROUTE_MSG_ROUTER_H__
#define __ROUTE_MSG_ROUTER_H__

#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "aaa_route_framework.h"

typedef enum {
   DIAMETER_ROUTER_RETX_DIVISOR = 2,
   DIAMETER_ROUTER_MIN_RETX_INTERVAL = 8,
   DIAMETER_ROUTER_MAX_RETX_COUNT    = 10
};

class DiameterMsgRouterHandler
{
   public:
       virtual int Request(std::auto_ptr<DiameterMsg> &msg,
                           DiameterPeerEntry *source,
                           DiameterPeerEntry *dest) = 0;
       virtual int Answer(std::auto_ptr<DiameterMsg> &msg,
                          DiameterPeerEntry *source,
                          DiameterPeerEntry *dest) = 0;

   protected:
      virtual ~DiameterMsgRouterHandler() { }
};

class DiameterMsgRouterHandlerTable
{
   /*
      Notes on the use of DiameterMsgRouterHandler. 
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
                            DiameterMsgRouterHandler *h) {
           m_Handlers[type] = h;
       }
       void RemoveHandler(H_TYPE type) {
           m_Handlers[type] = NULL;
       }
       DiameterMsgRouterHandler *operator[](H_TYPE type) {
           return m_Handlers[type];
       }

   protected:
       DiameterMsgRouterHandlerTable() {
           for (int i=0; i<H_MAX; i++) {
               m_Handlers[i] = NULL;
           }
       }
       DiameterMsgRouterHandler *m_Handlers[H_MAX];
};

class DiameterMsgRouter : public DiameterRouterFramework,
                          public DiameterMsgRouterHandlerTable 
{
   public:
       class DcLocal : public DiameterDeliveryRoutingNode<DiameterMsgRouter> {
           public:
               DcLocal(DiameterMsgRouter &r) :
                   DiameterDeliveryRoutingNode<DiameterMsgRouter>(r, 0, "dcLocal") {
               }
               AAA_ROUTE_RESULT Process(std::auto_ptr<DiameterMsg> msg,
                                        std::auto_ptr<DiameterMsg> &p,
                                        DiameterPeerEntry *source,
                                        DiameterPeerEntry *dest);
               int RequestMsg(std::auto_ptr<DiameterMsg> msg,
                              DiameterPeerEntry *source,
                              DiameterPeerEntry *dest);
               AAA_ROUTE_RESULT ErrorHandling(std::auto_ptr<DiameterMsg> msg,
                                              DiameterPeerEntry *source,
                                              DiameterPeerEntry *dest);
       };

       class DcForward : public DiameterDeliveryRoutingNode<DiameterMsgRouter> {
           public:
               DcForward(DiameterMsgRouter &r) :
                   DiameterDeliveryRoutingNode<DiameterMsgRouter>(r, 0, "dcForward") {
               }
               AAA_ROUTE_RESULT Process(std::auto_ptr<DiameterMsg> msg,
                                        std::auto_ptr<DiameterMsg> &p,
                                        DiameterPeerEntry *source,
                                        DiameterPeerEntry *dest);
               int RequestMsg(std::auto_ptr<DiameterMsg> msg,
                              DiameterPeerEntry *source,
                              DiameterPeerEntry *dest);
               int LoopDetection(std::auto_ptr<DiameterMsg> &msg);
       };

       class DcRouted : public DiameterDeliveryRoutingNode<DiameterMsgRouter> {
           public:
               DcRouted(DiameterMsgRouter &r) :
                   DiameterDeliveryRoutingNode<DiameterMsgRouter>(r, 0, "dcRouted") {
               }
               AAA_ROUTE_RESULT Process(std::auto_ptr<DiameterMsg> msg,
                                        std::auto_ptr<DiameterMsg> &p,
                                        DiameterPeerEntry *source,
                                        DiameterPeerEntry *dest);
               int RequestMsg(std::auto_ptr<DiameterMsg> msg,
                              DiameterPeerEntry *source,
                              DiameterPeerEntry *dest);
       };

       class DcReject : public DiameterDeliveryRoutingNode<DiameterMsgRouter> {
           public:
               DcReject(DiameterMsgRouter &r) :
                   DiameterDeliveryRoutingNode<DiameterMsgRouter>(r, 0, "dcReject") {
               }
               AAA_ROUTE_RESULT Process(std::auto_ptr<DiameterMsg> msg,
                                        std::auto_ptr<DiameterMsg> &p,
                                        DiameterPeerEntry *source,
                                        DiameterPeerEntry *dest);
               int RequestMsg(std::auto_ptr<DiameterMsg> msg,
                              DiameterPeerEntry *source,
                              DiameterPeerEntry *dest);
               AAA_ROUTE_RESULT Lookup(std::auto_ptr<DiameterMsg> &msg,
                                       std::auto_ptr<DiameterMsg> &p,
                                       DiameterPeerEntry *&dest);
       };

       class RcLocal : public DiameterRequestRoutingNode<DcLocal,
                       DiameterMsgRouter> {
           public:
               RcLocal(DiameterMsgRouter &r) :
                  DiameterRequestRoutingNode<DcLocal,
                                         DiameterMsgRouter>(r, 0, "rcLocal") { }
               AAA_ROUTE_RESULT Lookup(std::auto_ptr<DiameterMsg> &msg,
                                       std::auto_ptr<DiameterMsg> &p,
                                       DiameterPeerEntry *&dest);
       };

       class RcForwarded : public DiameterRequestRoutingNode<DcForward,
                           DiameterMsgRouter> {
           public:
               RcForwarded(DiameterMsgRouter &r) :
                   DiameterRequestRoutingNode<DcForward,
                                          DiameterMsgRouter>(r, 0, "rcForward") {
               }
               AAA_ROUTE_RESULT Lookup(std::auto_ptr<DiameterMsg> &msg,
                                       std::auto_ptr<DiameterMsg> &p,
                                       DiameterPeerEntry *&dest);
           };

       class RcRouted : public DiameterRequestRoutingNode<DcRouted,
                        DiameterMsgRouter> {
           public:
               RcRouted(DiameterMsgRouter &r) :
                   DiameterRequestRoutingNode<DcRouted,
                                          DiameterMsgRouter>(r, 0, "rcRouted") {
               }
               AAA_ROUTE_RESULT Lookup(std::auto_ptr<DiameterMsg> &msg,
                                       std::auto_ptr<DiameterMsg> &p,
                                       DiameterPeerEntry *&dest);
       };

       class RcRejected : public DiameterRequestRoutingNode<DcReject,
                          DiameterMsgRouter> {
           public:
               RcRejected(DiameterMsgRouter &r) :
                   DiameterRequestRoutingNode<DcReject,
                                          DiameterMsgRouter>(r, 0, "rcReject") {
               }
               AAA_ROUTE_RESULT Lookup(std::auto_ptr<DiameterMsg> &msg,
                                       std::auto_ptr<DiameterMsg> &p,
                                       DiameterPeerEntry *&dest);
       };

   protected:
       class DcRequestReTransmission : public AAA_Action<DiameterRouterPendingReqPtr> {
       	  public:
       	      DcRequestReTransmission(DiameterMsgRouter &r) : 
       	           m_Router(r) {
       	      }
              virtual void operator()(DiameterRouterPendingReqPtr &pReq) {
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
                  pReq->m_ReqMessage->hdr.flags.t = DIAMETER_FLAG_SET;

                  AAA_LOG((LM_INFO, "(%P|%t) **** Request message re-transmission ****\n"));
                  DiameterMsgHeaderDump::Dump(*(pReq->m_ReqMessage));

                  ACE_Time_Value expire(DIAMETER_CFG_TRANSPORT()->retx_interval, 0);
                  pReq->m_ReTxExpireTime = ACE_OS::gettimeofday() + expire;

              	  m_Router.RequestMsg(pReq->m_ReqMessage, pReq->m_Source);
              }
          private:
              DiameterMsgRouter &m_Router;
       };

   public:

       DiameterMsgRouter() : m_rcLocal(*this),
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
       void ReTransmitEvent(DiameterPeerEntry *peer = 0) {
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

       class RedirectAgent {
           public:
               int Request(std::auto_ptr<DiameterMsg> &msg,
                           DiameterPeerEntry *source,
                           DiameterPeerEntry *dest);
               int Answer(std::auto_ptr<DiameterMsg> &msg,
                          std::auto_ptr<DiameterMsg> p,
                          DiameterPeerEntry *source,
                          DiameterPeerEntry *dest);
               bool IsRedirected(std::auto_ptr<DiameterMsg> &msg);
       };

   private:
       RcLocal               m_rcLocal;
       RcForwarded           m_rcForward;
       RcRouted              m_rcRouted;
       RcRejected            m_rcRejected;
       RedirectAgent         m_RedirectAgent;
};

typedef ACE_Singleton<DiameterMsgRouter, ACE_Recursive_Thread_Mutex> DiameterMsgRouter_S;
#define DIAMETER_MSG_ROUTER() DiameterMsgRouter_S::instance()

#endif /* __ROUTE_MSG_ROUTER_H__ */


