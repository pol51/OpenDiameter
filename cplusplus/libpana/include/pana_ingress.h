
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

#ifndef __PANA_INGRESS_H__
#define __PANA_INGRESS_H__

#include "pana_message.h"
#include "pana_io.h"
#include "framework.h"
#include "pana_memory_manager.h"
#include "od_utl_patterns.h"

class PANA_EXPORT PANA_IngressJob : public AAA_Job
{
   public:
      PANA_IngressJob(AAA_GroupedJob &g, const char *name = "") : 
          m_Group(g), 
          m_MsgHandler(0), 
          m_Name(name) { 
      }
      virtual int Schedule(AAA_Job* job, unsigned int backlogSize=1) { 
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
                            ACE_UINT32 port,
                            PANA_DeviceIdContainer &dev,
                            const char *name = "") :
         PANA_IngressJob(g, name),
         m_SrcPort(port),
         m_Message(msg),
         m_SrcDevices(dev) { 
      }
      virtual ~PANA_IngressMsgParser() { 
      }

      virtual int Serve();

   private:
      ACE_UINT32 m_SrcPort;
      PANA_MessageBuffer &m_Message;
      PANA_DeviceIdContainer &m_SrcDevices;
};

/*!
 * Ingress io receiver
 */
class PANA_EXPORT PANA_IngressReceiver : public PANA_IngressJob
{
    public:
      PANA_IngressReceiver(AAA_GroupedJob &g, 
                           PANA_ResilientIO &io,
                           const char *name = "") :
             PANA_IngressJob(g, name),
			 m_IO(io),
			 m_Running(false),
             m_Abort(false) { 
      }

      bool &Abort() {
         return m_Abort;
      }
      virtual int Serve();
      virtual void Wait() {		  
         while (m_Running) {
            ACE_Time_Value tm(1, 0);
            ACE_OS::sleep(tm);
         }
      }

   protected:
      PANA_ResilientIO &m_IO;
      bool m_Running;
      bool m_Abort;
};

#endif // __PANA_INGRESS_H__
