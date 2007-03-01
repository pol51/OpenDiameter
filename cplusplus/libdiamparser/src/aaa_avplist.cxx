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

#include <ace/OS.h>
#include "aaa_avplist.h"
#include "aaa_g_avplist.h"
#include "diameter_parser.h"

static AAADictionaryEntry Any(0, "AVP", AAA_AVP_DATA_TYPE, 0, 0);

// calculate minimum AVP length
ACE_UINT32 getMinSize(AAADictionaryEntry *avp)
    throw (DiameterErrorCode)
{
    int sum=0;
    DiameterGroupedAVP *gavp;
    DiameterAvpType *avpt;
    DiameterErrorCode st;

    if (!avp) {
        AAA_LOG((LM_ERROR, "getMinSize(): AVP dictionary cannot be null."));
        st.set(AAA_PARSE_ERROR_TYPE_BUG,
            AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY);
        throw st;
    }

    avpt = (DiameterAvpType*)DiameterAvpTypeList::instance()->search(avp->avpType);
    if (!avpt) {
        AAA_LOG((LM_ERROR, "getMinSize(): Cannot find AVP type."));
        st.set(AAA_PARSE_ERROR_TYPE_BUG,
            AAA_PARSE_ERROR_MISSING_AVP_VALUE_PARSER);
        throw st;
    }

    if (avp->avpType == AAA_AVP_GROUPED_TYPE) {
        gavp = DiameterGroupedAvpList::instance()->search(avp->avpCode, avp->vendorId);
        if (!gavp) {
            AAA_LOG((LM_ERROR, "getMinSize(): Cannot grouped AVP dictionary."));
            st.set(AAA_PARSE_ERROR_TYPE_BUG,
                AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY);
            throw st;
        }

        /* Fixed AVPs */
        sum = gavp->avp_f->getMinSize();
        /* Required AVPs */
        sum = gavp->avp_r->getMinSize();
    }
    sum += avpt->getMinSize() + 8 + (avp->vendorId ? 4 : 0);
    // getMinSize() returns 0 for grouped AVP */
    return sum;
}

DiameterAvpList_S::DiameterAvpList_S()
{
    this->add(&Any);
}

DiameterAvpList_S::~DiameterAvpList_S()
{
    pop_front(); // remove ANY AVP
}

