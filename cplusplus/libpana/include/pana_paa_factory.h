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

#ifndef __PANA_PAA_FACTORY_H__
#define __PANA_PAA_FACTORY_H__

#include "pana_node.h"
#include "pana_session.h"
#include "pana_paa_fsm.h"
#include "pana_message.h"
#include "od_utl_patterns.h"
#include "od_utl_rbtree_dbase.h"

typedef OD_Utl_DbaseTree<std::string, PANA_PaaSession> PANA_PendingSessionDb;

class PANA_EXPORT PANA_PaaSessionFactory : public PANA_PaaSessionChannel
{
   public:
      PANA_PaaSessionFactory(PANA_Node &n) : PANA_PaaSessionChannel(n) { 
          
         OD_Utl_SCSIAdapter1<PANA_PaaSessionFactory, 
                           void(PANA_PaaSessionFactory::*)(PANA_Message&),
                           PANA_Message&> 
                     msgHandler(*this, &PANA_PaaSessionFactory::Receive);
         PANA_PaaSessionChannel::RegisterHandler(msgHandler);
         m_Flags.p = 0;
         m_Flags.i.CarryPcapInPSR = PANA_CARRY_PCAP_IN_PSR;
         m_Flags.i.CarryPcapInPBR = (PANA_CFG_GENERAL().m_ProtectionCap >= 0) ?
                                      true : PANA_CARRY_PCAP_IN_PBR;
         if (! m_Flags.i.CarryPcapInPBR) {
             m_Flags.i.CarryPcapInPSR = false;
         }
      }
      virtual PANA_PaaSessionFactory::~PANA_PaaSessionFactory() {
         PANA_PaaSessionChannel::RemoveHandler();
      }
      virtual PANA_PaaSession *Create() = 0;
      virtual void PacFound(ACE_INET_Addr &addr);

   protected:
      virtual void Receive(PANA_Message &msg);

   private:
      virtual void RxPDI(PANA_Message &msg);
      virtual void StatelessTxPSR(ACE_INET_Addr &addr);
      virtual void StatelessRxPSA(PANA_Message &msg);
      virtual bool ValidateCookie(PANA_Message &msg);

      PANA_PendingSessionDb m_PendingDb;
      PANA_PaaSupportFlags m_Flags;
};

template<class PAA_SESSION, class ARG>
class PANA_PaaSessionFactoryAdapter : public PANA_PaaSessionFactory
{
   public:
      PANA_PaaSessionFactoryAdapter(ARG &arg) : 
         m_arg(arg) { 
      }
      PANA_PaaSession *Create() {
         return (new PAA_SESSION(m_Node, m_arg));
      }
   protected:
      ARG &m_arg;
};

#endif /* __PANA_PAA_FACTORY_H__ */

