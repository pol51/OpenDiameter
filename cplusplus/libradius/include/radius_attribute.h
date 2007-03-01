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

#ifndef __RADIUS_ATTRIBUTE_H__
#define __RADIUS_ATTRIBUTE_H__

#include <list>
#include "radius_defs.h"
#include "ace/INET_Addr.h"

class RADIUS_AttrBase
{
   public:
      RADIUS_OCTET &Type() {
         return m_Type;
      }
      RADIUS_OCTET &Length() {
         return m_Length;
      }
      RADIUS_AttrBase(RADIUS_OCTET type = 0) :
      	 m_Type(type),
      	 m_Length(0) {
      }

   protected:
      RADIUS_OCTET m_Type;
      RADIUS_OCTET m_Length;
};

template<class T>
class RADIUS_Attribute : public RADIUS_AttrBase
{
   public:
      RADIUS_Attribute(RADIUS_OCTET type = 0) : 
         RADIUS_AttrBase(type) {
      }
      T &Value() {
         return m_Value;
      }
      RADIUS_Attribute &operator=(RADIUS_Attribute &a) {
         m_Value = a.Value;
         return (*this);
      }

   protected:
      T m_Value;
};

typedef std::list<RADIUS_AttrBase*> RADIUS_AttrList;
typedef std::list<RADIUS_AttrBase*>::iterator RADIUS_AttrListIter;

class RADIUS_AttributeList : public RADIUS_AttrList
{
   public:
      void Add(RADIUS_AttrBase *attr) {
         push_back(attr);
      }
      RADIUS_AttrBase *Lookup(RADIUS_OCTET type) {
         RADIUS_AttrListIter i = begin();
         for (; i!=end(); i++) {
            if ((*i)->Type() == type) {
               return (*i);
            }
         }
         return (NULL);
      }
      void Remove(RADIUS_OCTET type) {
         RADIUS_AttrListIter i = begin();
         for (; i!=end(); i++) {
            if ((*i)->Type() == type) {
                delete (*i);
                erase(i);                
                break;
            }
         }
      }
      void Clear() {
         while (! empty()) {
            RADIUS_AttrBase *h = front();
            pop_front();
            delete h;
         }
      }
      virtual ~RADIUS_AttributeList() {
      }
};

// Pre-defined types
typedef RADIUS_Attribute<std::string>    RADIUS_OctetString;
typedef RADIUS_OctetString               RADIUS_UTF8String;
typedef RADIUS_Attribute<ACE_UINT32>     RADIUS_Integer;
typedef RADIUS_Integer                   RADIUS_Time;
typedef RADIUS_Integer                   RADIUS_InterfaceId;
typedef RADIUS_Attribute<ACE_INET_Addr>  RADIUS_IpAddress;
typedef RADIUS_IpAddress                 RADIUS_IPv4Address;
typedef RADIUS_IpAddress                 RADIUS_IPv6Address;
typedef RADIUS_IpAddress                 RADIUS_IPv6Prefix;

#endif /* __RADIUS_ATTRIBUTE_H__ */
