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

#ifndef __PANA_MCAST_TRANSPORT_H__
#define __PANA_MCAST_TRANSPORT_H__

#include "ace/SOCK_Dgram_Mcast.h"
#include "pana_io.h"

class PANA_EXPORT PANA_McastTransport : public PANA_IO
{
    public:
        PANA_McastTransport() { }
        virtual ~PANA_McastTransport() { }
    
        virtual ACE_INT32 open(std::string &iface,
                               ACE_INET_Addr &listenAddr) {
            m_Iface = iface;
            m_McastAddr.set(listenAddr);
            return m_Mcast.join(listenAddr, 1, iface.data());
        }
        virtual ACE_INT32 recv(void *buf,
                               size_t n,
                               ACE_UINT32 &srcPort,
                               PANA_DeviceIdContainer &srcDevices) {            
           ACE_INET_Addr srcAddr;
           ACE_Time_Value tm(1, 0);
           ACE_INT32 pkt_len = m_Mcast.recv((char*)buf, n,
                                            srcAddr, 0, &tm);           
           if (pkt_len > 0) {
               srcPort = srcAddr.get_port_number();
               PANA_DeviceId *ip = PANA_DeviceIdConverter::CreateFromAddr(srcAddr);
               srcDevices.push_back(ip);
           }
           return (pkt_len);
        }
        virtual ACE_INT32 send(void *buf,
                               size_t n,
                               ACE_UINT32 destPort,
                               PANA_DeviceIdContainer &destDevices) {
           return m_Mcast.send(buf, n);
        }
        virtual ACE_INT32 get_local_addr(PANA_DeviceIdContainer &localDevices) {
           return (-1); // un-supported by this platform
        }
        virtual void close() {
           m_Mcast.leave(m_McastAddr, m_Iface.data());
        }

    private:
        ACE_SOCK_Dgram_Mcast m_Mcast;
        ACE_INET_Addr m_McastAddr;
        std::string m_Iface;
};

#endif /* __PANA_MCAST_TRANSPORT_H__ */

