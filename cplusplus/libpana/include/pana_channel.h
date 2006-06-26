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

#ifndef __PANA_CHANNEL_H__
#define __PANA_CHANNEL_H__

#include "boost/shared_ptr.hpp"
#include "pana_exceptions.h"
#include "pana_egress.h"
#include "pana_ingress.h"
#include "pana_udp_transport.h"

/*
 * PANA transport abstraction
 */
template<class SOCKET>
class PANA_EXPORT PANA_Channel : public PANA_IngressReceiver,
                                 public PANA_ResilientIO
{
    public:
        PANA_Channel(AAA_GroupedJob &g, const char *name = "") :
            PANA_IngressReceiver(g, *this, name),
            PANA_ResilientIO(m_Socket) {
        }
        PANA_Channel(AAA_GroupedJob &g, ACE_INET_Addr &addr) :
            PANA_IngressReceiver(g, *this),
            PANA_ResilientIO(m_Socket) {
            Open(addr);
        }
        virtual ~PANA_Channel() {
            Close();
        }
        virtual void Open(ACE_INET_Addr &addr) {
            if (m_Socket.open(PANA_CFG_GENERAL().m_Interface, addr) < 0) {
                throw (PANA_Exception(PANA_Exception::TRANSPORT_FAILED,
                                      "Failed to open device"));
            }
            PANA_IngressReceiver::Abort() = false;
            if (m_Group.Schedule(this) < 0) {
                throw (PANA_Exception(PANA_Exception::TRANSPORT_FAILED,
                                      "Failed to schedule channel"));
            }
            m_Socket.get_local_addr(m_LocalDeviceId);
            m_ListenAddr = addr;
        }
        virtual void Close() {
            PANA_IngressReceiver::Abort() = true;
            PANA_IngressReceiver::Wait();
            m_Socket.close();
        }
        virtual bool ReOpen() {
            m_Socket.close();
            return (m_Socket.open(PANA_CFG_GENERAL().m_Interface, m_ListenAddr) < 0) ?
                false : true;
        }
        virtual void Send(boost::shared_ptr<PANA_Message> m) {
            PANA_EgressSender *egress = new PANA_EgressSender
                (m_Group, *this, m);
            egress->Schedule(egress);
        }
        virtual PANA_DeviceIdContainer &GetLocalAddress() {
            return m_LocalDeviceId; 
        }
    protected:
        SOCKET m_Socket;
        PANA_DeviceIdContainer m_LocalDeviceId;
        ACE_INET_Addr m_ListenAddr;
};

typedef PANA_Channel<PANA_UdpBoundedSender> PANA_SenderChannel;
typedef PANA_Channel<PANA_McastListener>    PANA_ListenerChannel;

#endif // __PANA_CHANNEL_H__
