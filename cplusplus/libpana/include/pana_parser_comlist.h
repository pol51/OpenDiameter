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

#ifndef __PANA_COMLIST_H__
#define __PANA_COMLIST_H__

#include "pana_parser.h"
#include "pana_q_avplist.h"

class PANA_Dictionary :
    public AAADictionaryHandle
{
    public:
        PANA_AvpCode            m_Code;
        PANA_QualifiedAvpList*  m_Fixed;     /* fixed */
        PANA_QualifiedAvpList*  m_Required;  /* required */
        PANA_QualifiedAvpList*  m_Optional;  /* optional */
};

typedef struct 
{
    AAAUInt8 request:1;   /* request */
    AAAUInt8 stateless:1; /* stateless handshake */
    AAAUInt8 reserved:6;
} PANA_CommandFlags;

class PANA_Command :
    public PANA_Dictionary
{
    public:
        std::string       m_Name;
        PANA_CommandFlags m_Flags;
};

class PANA_CommandList_S :
    public AAACommandList<PANA_Command>
{
        friend class ACE_Singleton<PANA_CommandList_S, ACE_Recursive_Thread_Mutex>;
    public:
        PANA_Command* search(const char*name) {
            return AAACommandList<PANA_Command>::search(name);
        }

    private:
        virtual ~PANA_CommandList_S() {
            for (iterator i=begin(); i!=end(); i++) {
                delete (*i)->m_Fixed;
                delete (*i)->m_Required;
                delete (*i)->m_Optional;
                delete *i;
            }
        }
};

typedef ACE_Singleton<PANA_CommandList_S, ACE_Recursive_Thread_Mutex> PANA_CommandList;

#endif // __PANA_COMLIST_H__
