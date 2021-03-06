
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

#ifndef __PANA_INGRESS_H__
#define __PANA_INGRESS_H__

#include "pana_message.h"
#include "framework.h"
#include "pana_memory_manager.h"
#include "od_utl_patterns.h"

class PANA_EXPORT PANA_IngressJob :
   public AAA_Job
{
   public:
      PANA_IngressJob(AAA_GroupedJob &g, const char *name = "") :
          m_Group(g),
          m_MsgHandler(0),
          m_Name(name) {
      }
      virtual int Schedule(AAA_Job* job, size_t backlogSize=1) {
          return m_Group.Schedule(job);
      }
      virtual int Schedule() {
          return m_Group.Schedule(this);
      }
      void RegisterHandler(OD_Utl_CbFunction1<PANA_Message&> &h) {
          m_MsgHandler = h.clone();
      }
      void RemoveHandler() {
          delete m_MsgHandler;
          m_MsgHandler = NULL;
      }
      std::string &Name() {
          return m_Name;
      }

   protected:
      AAA_GroupedJob &m_Group;
      OD_Utl_CbFunction1<PANA_Message&> *m_MsgHandler;
      std::string m_Name;
};

/*!
 * Ingress message parser
 */
class PANA_EXPORT PANA_IngressMsgParser :
   public PANA_IngressJob
{
   public:
      PANA_IngressMsgParser(AAA_GroupedJob &g,
                            PANA_MessageBuffer &msg,
                            ACE_INET_Addr &srcAddr,
                            ACE_INET_Addr &dstAddr,
                            const char *name = "") :
         PANA_IngressJob(g, name),
         m_SrcAddr(srcAddr),
         m_DestAddr(dstAddr),
         m_Message(msg) {
      }
      virtual ~PANA_IngressMsgParser() {
      }

      virtual int Serve();

   private:
      ACE_INET_Addr m_SrcAddr;
      ACE_INET_Addr m_DestAddr;
      PANA_MessageBuffer &m_Message;
};

/*!
 * Ingress io receiver
 */
class PANA_EXPORT PANA_IngressReceiver :
    public PANA_IngressJob, ACE_Task<ACE_MT_SYNCH>
{
    public:
      PANA_IngressReceiver(AAA_GroupedJob &g,
                           PANA_Socket &so,
                           const char *name = "") :
                           PANA_IngressJob(g, name),
                           m_Socket(so) {
            m_localAddr.set_port_number(0);
            m_isRunning = false;
      }
      bool Start();
      void Stop();
      ACE_INET_Addr &SetLocalAddr() {
         return m_localAddr;
      }

   protected:
      int Serve();

   protected:
      ACE_INET_Addr m_localAddr;
      PANA_Socket &m_Socket;
      bool m_isRunning;

   private:
     int svc() {
        return Serve();
     }
};

#endif // __PANA_INGRESS_H__
