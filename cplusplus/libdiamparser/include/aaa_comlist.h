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
/* $Id: comlist.h,v 1.15 2004/06/17 21:07:49 yohba Exp $ */
#ifndef __COMLIST_H__
#define __COMLIST_H__

#include "diameter_parser.h"
#include "aaa_q_avplist.h"

class DiameterDictionary :
    public AAADictionaryHandle
{
    public:
        AAACommandCode code;
        DiameterApplicationId appId;
        AAAVendorId vendorId;
        DiameterQualifiedAvpList* avp_f;  /* fixed */
        DiameterQualifiedAvpList* avp_r;  /* required */
        DiameterQualifiedAvpList* avp_o;  /* optional */
};

struct comflags
{
    AAAUInt8 r:1;
    AAAUInt8 p:1;
    AAAUInt8 e:1;
    AAAUInt8 reseverd:5;
};

class DiameterCommand :
    public DiameterDictionary
{
    public:
        std::string name;
        struct comflags flags;
};

class DiameterCommandList_S :
    public AAACommandList<DiameterCommand>
{
        friend class ACE_Singleton<DiameterCommandList_S, ACE_Recursive_Thread_Mutex>;

    public:
        DiameterCommand* search(const char*name) {
            return AAACommandList<DiameterCommand>::search(name);
        }
        DiameterCommand* search(ACE_UINT32 code,
                                ACE_UINT32 appId,
                                int request) {
            mutex.acquire();
            for (iterator c=begin(); c!=end(); c++) {
                if ((*c)->code == code && 
                    (*c)->appId == appId &&
                    (*c)->flags.r == request) {
                    mutex.release();
                    return *c;
                }
            }
            mutex.release();
            return NULL;
        }

    private:
        virtual ~DiameterCommandList_S() {
            for (iterator i=begin(); i!=end(); i++) {
                //      delete (*i)->name;
                delete (*i)->avp_f;
                delete (*i)->avp_r;
                delete (*i)->avp_o;
                delete *i;
            }
        }
};

typedef ACE_Singleton<DiameterCommandList_S, ACE_Recursive_Thread_Mutex> DiameterCommandList;

#endif // __COMLIST_H__
