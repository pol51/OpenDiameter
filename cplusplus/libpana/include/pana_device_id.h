
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

#ifndef __PANA_DEVICE_ID_H__
#define __PANA_DEVICE_ID_H__

#include <string>
#include <list>
#include "ace/OS.h"
#include "ace/INET_Addr.h"
#include "pana_defs.h"

class PANA_DeviceId : 
    public diameter_address_t
{
    public:
        PANA_DeviceId(ACE_UINT16 typ, std::string &id) {
            type = typ;
            value.assign(id.data(), id.size());
        }
        PANA_DeviceId(diameter_address_t &addr) {
            *this = addr;
        }
        PANA_DeviceId() {
        }
        bool operator==(PANA_DeviceId &cmp) {
            return (ACE_OS::memcmp(value.data(), 
                                   cmp.value.data(),
                                   cmp.value.size()) == 0);
        }
        PANA_DeviceId &operator=(PANA_DeviceId &src) {
            type = src.type;
            value.assign(src.value.data(), 
                         src.value.size());
            return (*this);
        }
        diameter_address_t &operator=(diameter_address_t &src) {
            type = src.type;
            value.assign(src.value.data(), 
                         src.value.size());
            return (*this);
        }
        diameter_address_t &operator()() {
            return *this;
        }
};

typedef std::list<PANA_DeviceId*>::iterator PANA_DeviceIdIterator;

class PANA_DeviceIdContainer : 
    public std::list<PANA_DeviceId*>
{
    public:
        virtual ~PANA_DeviceIdContainer() {
            clear();
        }
        PANA_DeviceId *search(ACE_UINT16 type) {
            PANA_DeviceId *id;
            PANA_DeviceIdIterator i;
            for (i = begin(); i != end(); i++) {
                id = *i;
                if (id->type == type) {
                    return id;
                }
            }
            return (NULL);
        }
        void replace(PANA_DeviceId &id) {
            PANA_DeviceId *old = search(id.type);
            if (old) {
                *old = id;
            }
        }
        void remove(ACE_UINT16 type) {
            PANA_DeviceId *id;
            PANA_DeviceIdIterator i;
            for (i = begin(); i != end(); i++) {
                id = *i;
                if (id->type == type) {
                    delete id;
                    erase(i);
                    break;
                }
            }
        }
        void move(PANA_DeviceIdContainer &c) {
            while (! c.empty()) {
                PANA_DeviceId *id = c.front();
                c.pop_front();
                push_back(id);
            }
        }
        void move(PANA_DeviceIdContainer &c,
                  PANA_DeviceId &id) {
            PANA_DeviceId *entry;
            PANA_DeviceIdIterator i;
            for (i = c.begin(); i != c.end(); i++) {
                entry = *i;
                if (entry == &id) {
                    c.erase(i);
                    push_back(entry);
                    return;
                }
            }
        }
        void clone(PANA_DeviceIdContainer &c) {
            PANA_DeviceId *id;
            PANA_DeviceIdIterator i;
            for (i = c.begin(); i != c.end(); i++) {
                id = *i;
                clone(*id);
            }
        }
        void clone(PANA_DeviceId &id) {
            PANA_DeviceId *newId = new PANA_DeviceId;
            if (newId) {
                *newId = id;
            }
            push_back(newId);
        }
        void clear() {
            while (! empty()) {
                PANA_DeviceId *id = front();
                pop_front();
                delete id;
            }
        }
        PANA_DeviceIdContainer &operator=(PANA_DeviceIdContainer &src) {
            clear();
            clone(src);
            return *this;
        }
};

class PANA_DeviceIdConverter {
    public:
        static inline bool PopulateFromAddr(ACE_INET_Addr &addr,
                                            PANA_DeviceId &id) {
            ACE_UINT16 type;
            std::string octetAddr;
            switch (addr.get_type()) {
                case AF_INET: {
                       type = AAA_ADDR_FAMILY_IPV4;
                       ACE_UINT32 netAddr = addr.get_ip_address();
                       octetAddr.assign((char*)&netAddr, sizeof(ACE_UINT32));
                    }
                    break;
#if defined (ACE_HAS_IPV6)
                case AF_INET6:
                    type = AAA_ADDR_FAMILY_IPV6;
                    octetAddr.assign((char*)((sockaddr_in6*)
                                     addr.get_addr())->sin6_addr.s6_addr,
                                     sizeof(in6_addr));
                    break;
#endif
                default: return (false);
            }
            id.type = type;
            id.value.assign(octetAddr.data(), octetAddr.size());
            return (true);
        }
        static inline int AsciiToHex(const char *str, 
            unsigned char *hexValue) {
            int len = (int)strlen(str)/2;
            for (int y = 0; y < len; ++y) {
                for (int i = y * 2; i <= ((y * 2) + 1); i ++) {
                    UCHAR value = 0;
                    if (isxdigit(str[i])) {
                        if (isdigit(str[i])) {
                            value = str[i] - '0';
                        }
                        else if (islower(str[i])) {
                            value = str[i] - 'a';
                            value += 10;
                        }
                        else {
                            value = str[i] - 'A';
                            value += 10;
                        }
                        if (i == (y * 2)) {
                            hexValue[y] = value << 4;
                        }
                        else {
                            hexValue[y] |= value;
                        }
                    }
                    else {
                        return (0);
                    }
                }
            }
            return (len);
        }
        static inline PANA_DeviceId *CreateFromAddr(ACE_INET_Addr &addr) {
            PANA_DeviceId *id = new PANA_DeviceId;
            if (id) {
                PopulateFromAddr(addr, *id);
            }
            return (id);
        }
        static inline PANA_DeviceId *CreateFromAddr(void *addr, int len) {
            ACE_INET_Addr baseAddr;
            baseAddr.set_addr(addr, len, 0);
            return CreateFromAddr(baseAddr);
        }
        static inline PANA_DeviceId *CreateFromLinkLayer(void *addr, int len) {
            std::string holder;
            holder.assign((char*)addr, len);
            // TBD: we may need to have a different
            // addr family assingment here.
            return (new PANA_DeviceId(AAA_ADDR_FAMILY_802, holder));
        }
        static inline void FormatToAddr(PANA_DeviceId &id,
                                        ACE_INET_Addr &addr) {
            switch (id.type) {
                case AAA_ADDR_FAMILY_IPV4:
                    addr.set_type(AF_INET);
                    addr.set_address(id.value.data(), 
                                     (int)id.value.size());
                    break;
#if defined (ACE_HAS_IPV6)
                case AAA_ADDR_FAMILY_IPV6:
                    addr.set_type(AF_INET6);
                    addr.set_address(id.value.data(), 
                                     (int)id.value.size(),
                                     0);
                    break;
#endif
                default: return;
            }
        }
        static inline PANA_DeviceId *GetIpOnlyAddress
                (PANA_DeviceIdContainer &cntr,
                 bool ipv6 = false) {
            return cntr.search(ipv6 ?
                               AAA_ADDR_FAMILY_IPV6 :
                               AAA_ADDR_FAMILY_IPV4);
        }
};

#endif // __PANA_DEVICE_ID_H__

