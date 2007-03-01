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
/* $Id: error_status.cxx,v 1.18 2006/03/17 13:36:54 vfajardo Exp $ */
#include "aaa_parser_avp.h"

void
DiameterErrorCode::set(AAA_PARSE_ERROR_TYPE type,
                       int code,
                       AAADictionaryEntry* dict)
{
    DiameterAvpHeader h;
    AAADictionaryEntry *avp = (AAADictionaryEntry*)dict;
    ACE_UINT32 avpSize = h.length = getMinSize(avp);
    char *buffer = new char[avpSize+sizeof(ACE_UINT32)];
    DiameterAvpHeaderParser ahp;
    DiameterAvpRawData rawData;

    memset(buffer, 0, avpSize+sizeof(ACE_UINT32));
    memset(&rawData, 0, sizeof(rawData));
    AAAMessageBlock *aBuffer = AAAMessageBlock::Acquire(buffer, avpSize+sizeof(ACE_UINT32));
    rawData.msg = aBuffer;
    ahp.setAppData(&h);
    ahp.setRawData(&rawData);
    ahp.setDictData(avp);
    this->type = type;
    this->code = code;
    ahp.parseAppToRaw();
    this->avp.assign(buffer, avpSize);
    aBuffer->Release();
    delete[] buffer;
}
