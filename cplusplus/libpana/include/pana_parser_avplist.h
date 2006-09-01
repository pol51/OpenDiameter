/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open PANA_: Open-source software for the PANA_ and               */
/*                PANA_ related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2004 Open PANA_ Project                          */
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

#ifndef __PANA_AVPLIST_H__
#define __PANA_AVPLIST_H__

#include "pana_parser.h"
#include "pana_parser_comlist.h"

#define PANA_AVP_HEADER_LEN(avp) \
  (avp->m_AvpCode == 0 ? 0 : \
  (avp->m_Flags & PANA_AVP_FLAG_VENDOR_SPECIFIC ? 12 : 8))

class PANA_AvpList_S :
    public AAAAvpList
{
    friend class ACE_Singleton<PANA_AvpList_S, ACE_Recursive_Thread_Mutex>;

    private:
        PANA_AvpList_S();
        virtual ~PANA_AvpList_S();
};

typedef ACE_Singleton<PANA_AvpList_S, ACE_Recursive_Thread_Mutex> PANA_AvpList;

class PANA_GroupedAVP :
    public PANA_Dictionary {
};

class PANA_QualifiedAvpList:
    public AAAQualifiedAvpList
{
    public:
        PANA_QualifiedAvpList(AAAAvpParseType pt) :
            AAAQualifiedAvpList(pt) {
        }
        ACE_UINT32 getMinSize(void) {
            return (0);
        }
};

class PANA_GroupedAvpList_S :
    public AAAGroupedAvpList<PANA_GroupedAVP>
{
    friend class ACE_Singleton<PANA_GroupedAvpList_S, ACE_Recursive_Thread_Mutex>;

    public:
        PANA_GroupedAVP* search(ACE_UINT32 code,
                                ACE_UINT32 vendorId) {
            mutex.acquire();
            for (iterator c=begin(); c!=end(); c++) {
                if ((*c)->code == code && (*c)->vendorId == vendorId) {
                    mutex.release();
                    return *c;
                }
            }
            mutex.release();
            return NULL;
        }

    private:
        virtual ~PANA_GroupedAvpList_S() {
            for (iterator i=begin(); i!=end(); i++) {
                delete (*i)->avp_f;
                delete (*i)->avp_r;
                delete (*i)->avp_o;
                delete *i;
            }
        }
};

typedef ACE_Singleton<PANA_GroupedAvpList_S, ACE_Recursive_Thread_Mutex>
    PANA_GroupedAvpList;

#endif // __PANA_AVPLIST_H__
