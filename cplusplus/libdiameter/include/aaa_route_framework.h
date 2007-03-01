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

#ifndef __ROUTE_FRAMEWORK_H__
#define __ROUTE_FRAMEWORK_H__

#include "diameter_parser.h"
#include "aaa_peer_table.h"
#include "aaa_route_id_generator.h"

typedef enum {
   AAA_ROUTE_RESULT_SUCCESS,
   AAA_ROUTE_RESULT_FAILED,
   AAA_ROUTE_RESULT_NEXT_CHAIN
} AAA_ROUTE_RESULT;

// abstract class for a routing node
class DiameterRoutingNode
{
   public:
       AAA_ROUTE_RESULT Route(std::auto_ptr<DiameterMsg> msg,
                              DiameterPeerEntry *source) {
           DiameterPeerEntry *dest = NULL;
           std::auto_ptr<DiameterMsg> pending;
           AAA_ROUTE_RESULT r = Lookup(msg, pending, dest);
           Dump(r);
           switch (r) {
               case AAA_ROUTE_RESULT_SUCCESS:
                   return Process(msg, pending, source, dest);
               case AAA_ROUTE_RESULT_NEXT_CHAIN:
                   if (m_Next) {
                       return m_Next->Route(msg, source);
                   }
                   // fall through
               default:
                   AAA_LOG((LM_INFO, "(%P|%t) **** Message failed to route ****\n"));
                   DiameterMsgHeaderDump::Dump(*msg);
                   break;
           }
           return (r);
       }
       DiameterRoutingNode *Next() {
           return m_Next;
       }
       void Next(DiameterRoutingNode *next) {
           m_Next = next;
       }
       std::string &Name() {
           return m_Name;
       }
       virtual AAA_ROUTE_RESULT Lookup(std::auto_ptr<DiameterMsg> &m,
                                       std::auto_ptr<DiameterMsg> &p,
                                       DiameterPeerEntry *&dest) = 0;
       virtual AAA_ROUTE_RESULT Process(std::auto_ptr<DiameterMsg> m,
                                        std::auto_ptr<DiameterMsg> &p,
                                        DiameterPeerEntry *source,
                                        DiameterPeerEntry *dest) = 0;

       virtual ~DiameterRoutingNode() {
       }
   protected:
       DiameterRoutingNode(DiameterRoutingNode *next = NULL,
                       char *name = "") :
           m_Next(next), m_Name(name) {
       }
       void Dump(AAA_ROUTE_RESULT r) {
#if AAA_ROUTE_DEBUG
           switch (r) {
               case AAA_ROUTE_RESULT_NEXT_CHAIN:
                   AAA_LOG((LM_INFO, "(%P|%t) Route lookup %s defer to next chain\n",
                             m_Name.c_str()));
                   break;
               case AAA_ROUTE_RESULT_SUCCESS:
                   AAA_LOG((LM_INFO, "(%P|%t) Route lookup %s successful, processing msg\n",
                              m_Name.c_str()));
                   break;
               default:
                   AAA_LOG((LM_INFO, "(%P|%t) Route lookup %s failed, discarding msg\n",
                              m_Name.c_str()));
                   break;
           }
#endif
       }

   private:
       DiameterRoutingNode *m_Next;
       std::string m_Name;
};

typedef struct {
   int m_OrigHH;
   DiameterPeerEntry *m_Source;
   DiameterPeerEntry *m_Dest;
   ACE_Time_Value m_ReTxExpireTime;
   unsigned int m_ReTxCount;
   std::auto_ptr<DiameterMsg> m_ReqMessage;
} DiameterRouterPendingReq;

typedef DiameterRouterPendingReq* DiameterRouterPendingReqPtr;

template<class ARG>
class DiameterDeliveryRoutingNode : public DiameterRoutingNode
{
   private:

       class PendingReqCleanup : public AAA_IterAction<DiameterRouterPendingReqPtr> {
          public:
             virtual bool operator()(DiameterRouterPendingReqPtr &r) {
                delete r;
                return true;
             }
       };

       template<class THRESHOLD>
       class PendingReqReTxCheck : public AAA_IterAction<DiameterRouterPendingReqPtr> {
          public:
             PendingReqReTxCheck(THRESHOLD thold,
                AAA_Action<DiameterRouterPendingReqPtr> &act) :
                m_Threshold(thold),
                m_Action(act) {
             }
             virtual bool operator()(DiameterRouterPendingReqPtr &r) {
                if (match(r)) {
                    if (r->m_ReTxCount < DIAMETER_CFG_TRANSPORT()->retx_max_count) {
                        r->m_ReTxCount++;
                        m_Action(r);
                    }
                    else {
                        AAA_LOG((LM_INFO, 
                            "(%P|%t) **** Re-Transmission count exceeded, Request discarded ****\n"));
                        delete r;
                        return (true);
                    }
                }
                return (false);
             }
             virtual bool match(DiameterRouterPendingReqPtr &r) = 0;
          protected:
             THRESHOLD m_Threshold;
             AAA_Action<DiameterRouterPendingReqPtr> &m_Action;
       };

       class PendingReqAgeCheck : public PendingReqReTxCheck<ACE_Time_Value> {
          public:
             PendingReqAgeCheck(ACE_Time_Value thold,
                AAA_Action<DiameterRouterPendingReqPtr> &act) :
                PendingReqReTxCheck<ACE_Time_Value>(thold, act) {
             }
             virtual bool match(DiameterRouterPendingReqPtr &r) {
                return (r->m_ReTxExpireTime < 
                        PendingReqReTxCheck<ACE_Time_Value>::m_Threshold);
             }
       };

       class PendingReqPeerCheck : public PendingReqReTxCheck<DiameterPeerEntry*> {
          public:
             PendingReqPeerCheck(DiameterPeerEntry *peer,
                AAA_Action<DiameterRouterPendingReqPtr> &act) :
                PendingReqReTxCheck<DiameterPeerEntry*>(peer, act) {
             }
             virtual bool match(DiameterRouterPendingReqPtr &r) {
                return (r->m_Dest ==
                        PendingReqReTxCheck<DiameterPeerEntry*>::m_Threshold);
             }
       };

   public:
       virtual int RequestMsg(std::auto_ptr<DiameterMsg> msg,
                              DiameterPeerEntry *source,
                              DiameterPeerEntry *dest) = 0;
       int AnswerMsg(std::auto_ptr<DiameterMsg> msg, DiameterPeerEntry *source) {
           return (Route(msg, source) == AAA_ROUTE_RESULT_SUCCESS) ?
               0 : (-1);
       }
       ARG &Arg() {
           return m_Arg;
       }
       void ReTransmitCheck(DiameterPeerEntry *peer, 
                            AAA_Action<DiameterRouterPendingReqPtr> &act) {
           PendingReqPeerCheck peerCheck(peer, act);
           m_ReqMap.Iterate(peerCheck);
       }
       void ReTransmitCheck(ACE_Time_Value &current,
                            AAA_Action<DiameterRouterPendingReqPtr> &act) {
           PendingReqAgeCheck ageCheck(current, act);
           m_ReqMap.Iterate(ageCheck);
       }

   protected:
       DiameterDeliveryRoutingNode(ARG &a,
                               DiameterRoutingNode *next = NULL,
                               char *name = "") :
           DiameterRoutingNode(next, name),
           m_Arg(a) {
       }
       virtual ~DiameterDeliveryRoutingNode() {
           ClearQueuedMessages();
       }

       int Add(int localh2h,
               std::auto_ptr<DiameterMsg> &msg,
               DiameterPeerEntry *source,
               DiameterPeerEntry *dest) {
           DiameterRouterPendingReqPtr r = new DiameterRouterPendingReq;
           if (r) {
               ACE_Time_Value expire(DIAMETER_CFG_TRANSPORT()->retx_interval, 0);
               r->m_ReTxExpireTime = ACE_OS::gettimeofday() + expire;
               r->m_OrigHH = msg->hdr.hh;
               r->m_Source = source;
               r->m_Dest = dest;
               r->m_ReTxCount = 0;
               m_ReqMap.Add(localh2h, r);
               return (localh2h);
           }
           return (-1);
       }
       DiameterRouterPendingReqPtr LookupQueuedMessage(int h2hId) {
           DiameterRouterPendingReqPtr r = NULL;
           m_ReqMap.Lookup(h2hId, r);
           return r;
       }
       int DeleteQueuedMessage(int h2hId) {
           PendingReqCleanup dealloc;
           return (m_ReqMap.Remove(h2hId, dealloc)) ? 0 : (-1);
       }
       void ClearQueuedMessages() {
           PendingReqCleanup dealloc;
           m_ReqMap.Iterate(dealloc);
       }
       int StoreRequestMessage(int h2hId,
                               std::auto_ptr<DiameterMsg> &msg) {
           DiameterRouterPendingReqPtr r = NULL;
           if (m_ReqMap.Lookup(h2hId, r)) {
               r->m_ReqMessage = msg;
               return (0);
           }
           return (-1);
       }
       virtual AAA_ROUTE_RESULT Lookup(std::auto_ptr<DiameterMsg> &m,
                                       std::auto_ptr<DiameterMsg> &p,
                                       DiameterPeerEntry *&dest) {
           /*
              6.2.1.  Processing received Answers

              A Diameter client or proxy MUST match the Hop-by-Hop
              Identifier in an answer received against the list of
              pending requests.  The corresponding message should
              be removed from the list of pending requests. It SHOULD
              ignore answers received that do not match a known
              Hop-by-Hop Identifier.
              .
              .
              If the answer is for a request which was proxied
              or relayed, the agent MUST restore the original
              value of the Diameter header's Hop-by-Hop Identifier
              field.
           */
           DiameterRouterPendingReqPtr r = NULL;
           if (m_ReqMap.Lookup(m->hdr.hh, r)) {
               int localhh = m->hdr.hh;
               dest = r->m_Source;
               p = r->m_ReqMessage;
               m->hdr.hh = r->m_OrigHH;
               DeleteQueuedMessage(localhh);
               return (AAA_ROUTE_RESULT_SUCCESS);
           }
           return (AAA_ROUTE_RESULT_NEXT_CHAIN);
       }

   protected:
       ARG &m_Arg;

   private:
       AAA_ProtectedMap<int, DiameterRouterPendingReqPtr> m_ReqMap;
};

template<class DELIVERY_NODE, class ARG>
class DiameterRequestRoutingNode : public DiameterRoutingNode
{
   public:
       DELIVERY_NODE &Delivery() {
           return m_DeliveryNode;
       }
   protected:
       DiameterRequestRoutingNode(ARG &a,
                              DiameterRoutingNode *next = NULL,
                              char *name = "") :
           DiameterRoutingNode(next, name),
           m_DeliveryNode(a) { }
       virtual ~DiameterRequestRoutingNode() {
       }
       AAA_ROUTE_RESULT Process(std::auto_ptr<DiameterMsg> msg,
                                std::auto_ptr<DiameterMsg> &p,
                                DiameterPeerEntry *source,
                                DiameterPeerEntry *dest) {
           return (m_DeliveryNode.RequestMsg(msg, source, dest) == 0) ?
               AAA_ROUTE_RESULT_SUCCESS : AAA_ROUTE_RESULT_FAILED;
       }

       DELIVERY_NODE m_DeliveryNode;
};

class DiameterRoutingChain
{
   public:
       DiameterRoutingChain() : m_Head(NULL) {
       }
       virtual ~DiameterRoutingChain() {
       }
       AAA_ROUTE_RESULT Route(std::auto_ptr<DiameterMsg> msg,
                              DiameterPeerEntry *source) {
           return (m_Head) ? m_Head->Route(msg, source) :
               AAA_ROUTE_RESULT_FAILED;
       }
       int Add(DiameterRoutingNode *n) {
           if (m_Head == NULL) {
               m_Head = n;
               return (0);
           }
           DiameterRoutingNode *next = m_Head;
           DiameterRoutingNode *prev = NULL;
           while (next) {
               prev = next;
               next = next->Next();
           }
           prev->Next(n);
           return (0);
       }
       bool Lookup(DiameterRoutingNode *n) {
           DiameterRoutingNode *next = m_Head;
           while (next) {
               if (next == n) {
                   return (true);
               }
               next = next->Next();
           }
           return (false);
       }
       int Remove(DiameterRoutingNode *n) {
           DiameterRoutingNode *next = m_Head;
           DiameterRoutingNode *prev = NULL;
           while (next) {
               if (next == n) {
                   if (prev) {
                       prev->Next(next->Next());
                   }
                   else if (m_Head->Next()) {
                       m_Head = m_Head->Next();
                   }
                   else {
                       m_Head = NULL;
                   }
                   delete n;
                   return (0);
               }
               prev = next;
               next = next->Next();
           }
           return (-1);
       }

    private:
       DiameterRoutingNode *m_Head;
};

class DiameterRouterFramework
{
    public:
       AAA_ROUTE_RESULT RequestMsg(std::auto_ptr<DiameterMsg> msg,
                                   DiameterPeerEntry *source) {
           return m_RequestChain.Route(msg, source);
       }
       AAA_ROUTE_RESULT AnswerMsg(std::auto_ptr<DiameterMsg> msg,
                                  DiameterPeerEntry *source) {
           return m_DeliveryChain.Route(msg, source);
       }
       DiameterRoutingChain &Delvery() {
           return m_DeliveryChain;
       }
       DiameterRoutingChain &Request() {
           return m_RequestChain;
       }

    protected:
       DiameterRoutingChain m_DeliveryChain;
       DiameterRoutingChain m_RequestChain;
};

#endif /* __ROUTE_FRAMEWORK_H__ */


