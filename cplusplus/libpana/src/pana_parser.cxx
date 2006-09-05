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

#include <ace/OS.h>
#include "pana_parser.h"

static AAADictionaryEntry PANA_CatchAllAvp(0, "AVP", AAA_AVP_DATA_TYPE, 0, 0);

PANA_AvpList_S::PANA_AvpList_S()
{
    this->add(&PANA_CatchAllAvp);
}

PANA_AvpList_S::~PANA_AvpList_S()
{
    pop_front(); // remove ANY AVP
}

void PANA_AvpHeaderList::create(AAAMessageBlock *aBuffer)
    throw (AAAErrorCode)
{
    PANA_AvpHeader h;
    AAAErrorCode st;
    char *start = aBuffer->rd_ptr();
    char *end = aBuffer->base()+aBuffer->size();

    for (char *cavp = start; cavp < end; cavp += adjust_word_boundary(h.m_Length)) {
        char *p = cavp;

        h.m_Code = ACE_NTOHS(*((ACE_UINT16*)p)); p += 2;
        h.m_Flags.vendor = (*((ACE_UINT16*)p) & PANA_AVP_FLAG_VENDOR_SPECIFIC) ? 1 : 0;
        h.m_Flags.mandatory = (*((ACE_UINT16*)p) & PANA_AVP_FLAG_MANDATORY) ? 1 : 0;
        p += 2;

        h.m_Length = ACE_NTOHL(*((ACE_UINT16*)p)); p += 4;
        if (h.m_Length == 0 || h.m_Length > (ACE_UINT32)(end-cavp)) {
            AAAErrorCode st;
            AAA_LOG(LM_ERROR, "invalid message length\n");
            st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_MESSAGE_LENGTH);
            throw st;
        }

        if (h.m_Flags.vendor == 1) {
            h.m_Flags.vendor = ACE_NTOHL(*((ACE_UINT32*)p)); p+=4;
        }
        h.m_pValue = cavp;      // Store the pointer to the header head
        push_back(h);
    }

    aBuffer->rd_ptr(end);
}

template<> void PANA_QualifiedAvpListParser::parseRawToApp()// throw (DiameterErrorCode)
{
}

template<> void PANA_QualifiedAvpListParser::parseAppToRaw()// throw (DiameterErrorCode)
{
}

