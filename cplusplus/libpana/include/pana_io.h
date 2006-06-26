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

#ifndef __PANA_IO_H__
#define __PANA_IO_H__

#include "pana_exports.h"
#include "pana_exceptions.h"
#include "pana_device_id.h"

class PANA_EXPORT PANA_IO
{
    public:
        virtual ~PANA_IO() { 
        }
        virtual ACE_INT32 open(std::string &iface,
                               ACE_INET_Addr &listenAddr) = 0;
        virtual ACE_INT32 recv(void *buf,
                               size_t n,
                               ACE_UINT32 &srcPort,
                               PANA_DeviceIdContainer &srcDevices) = 0;
        virtual ACE_INT32 send(void *buf,
                               size_t n,
                               ACE_UINT32 destPort,
                               PANA_DeviceIdContainer &destDevices) = 0;
        virtual ACE_INT32 get_local_addr(PANA_DeviceIdContainer &localDevices) = 0;
        virtual void close() = 0;
};

class PANA_EXPORT PANA_ResilientIO
{
    public:
        PANA_ResilientIO(PANA_IO &io) : 
            m_IO(io) {
        }
        virtual ~PANA_ResilientIO() {
        }
        PANA_IO &operator()() {
            return m_IO;
        }
        virtual bool ReOpen() = 0;

    protected:
        PANA_IO &m_IO;
};

#endif /* __PANA_IO_H__ */

