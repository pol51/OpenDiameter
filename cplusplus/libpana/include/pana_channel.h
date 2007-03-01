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

#ifndef __PANA_CHANNEL_H__
#define __PANA_CHANNEL_H__

#include "boost/shared_ptr.hpp"
#include "pana_exceptions.h"
#include "pana_egress.h"
#include "pana_ingress.h"

/*
 * PANA transport abstraction
 */
class PANA_EXPORT PANA_Channel :
    public PANA_IngressReceiver
{
    public:
        PANA_Channel(AAA_GroupedJob &g, const char *name = "") :
            PANA_IngressReceiver(g, m_Socket, name) {
        }
        PANA_Channel(AAA_GroupedJob &g, ACE_INET_Addr &addr) :
            PANA_IngressReceiver(g, m_Socket) {
            Open(addr);
        }
        virtual ~PANA_Channel() {
            Close();
        }
        virtual void Open(ACE_INET_Addr &addr) {
            if (m_Socket.open(addr) < 0) {
                AAA_LOG((LM_ERROR, "(%P|%t) Failed to open socket\n"));
                throw (PANA_Exception(PANA_Exception::TRANSPORT_FAILED,
                                      "Failed to open device"));
            }
            if (m_Group.Schedule(this) < 0) {
                AAA_LOG((LM_ERROR, "(%P|%t) Failed to schedule receiver job\n"));
                throw (PANA_Exception(PANA_Exception::TRANSPORT_FAILED,
                                      "Failed to schedule channel"));
            }
        }
        virtual void Close() {
            PANA_IngressReceiver::Stop();
            PANA_IngressReceiver::Wait();
            m_Socket.close();
        }
        virtual void Send(boost::shared_ptr<PANA_Message> m) {
            PANA_EgressSender *egress = new PANA_EgressSender
                (m_Group, m_Socket, m);
            egress->Schedule(egress);
        }

    protected:
        PANA_Socket m_Socket;
};

#endif // __PANA_CHANNEL_H__
