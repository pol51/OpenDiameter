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

#ifndef __AVPLIST_H__
#define __AVPLIST_H__

#include "diameter_parser.h"

class DiameterAvpList_S :
    public AAAAvpList
{
    friend class ACE_Singleton<DiameterAvpList_S, ACE_Recursive_Thread_Mutex>;

    private:
        DiameterAvpList_S();
        virtual ~DiameterAvpList_S();
};

typedef ACE_Singleton<DiameterAvpList_S, ACE_Recursive_Thread_Mutex> DiameterAvpList;

#define DIAMETER_AVP_HEADER_LEN(avp) \
  (avp->avpCode == 0 ? 0 : \
  (avp->flags & DIAMETER_AVP_FLAG_VENDOR_SPECIFIC ? 12 : 8))

ACE_UINT32 getMinSize(AAADictionaryEntry*) throw (DiameterErrorCode);

#endif // __AVPLIST_H__
