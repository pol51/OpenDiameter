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
/* $Id: avplist.cxx,v 1.24 2004/06/17 21:07:49 yohba Exp $ */
#include <ace/OS.h>
#include "avplist.h"
#include "g_avplist.h"
#include "diameter_parser_api.h"

static AAADictionaryEntry Any(0, "AVP", AAA_AVP_DATA_TYPE, 0, 0, 0);

AAADictionaryEntry::AAADictionaryEntry(AAA_AVPCode code,
				       const char* name, 
				       AAA_AVPDataType type, 
				       AAAVendorId vid, 
				       AAA_AVPFlag flgs,
                                       int proto)
{
  avpCode = code;
  avpName = name;
  avpType = type;
  vendorId = vid;
  flags = flgs;
  protocol = proto;
}

// calculate minimum AVP length
ACE_UINT32
getMinSize(AAADictionaryEntry *avp) throw (AAAErrorStatus) 
{
  AAAGroupedAVP *gavp;
  AvpType *avpt;
  int sum=0;
  AAAErrorStatus st;

  if (!avp)
  {
	AAA_LOG(LM_ERROR, "getMinSize(): AVP dictionary cannot be null.");
    st.set(BUG, MISSING_AVP_DICTIONARY_ENTRY);
	throw st;
  }

  avpt = AvpTypeList::instance()->search(avp->avpType);
  if (!avpt)
  {
	AAA_LOG(LM_ERROR, "getMinSize(): Cannot find AVP type.");
    st.set(BUG,MISSING_AVP_VALUE_PARSER);
	throw st;
  }

  if (avp->avpType == AAA_AVP_GROUPED_TYPE)
    {
      gavp = AAAGroupedAvpList::instance()->search(avp->avpCode, avp->vendorId);
      if (!gavp)
	    {
		  AAA_LOG(LM_ERROR, "getMinSize(): Cannot grouped AVP dictionary.");
		  st.set(BUG,MISSING_AVP_DICTIONARY_ENTRY);
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

AAAAvpList_S::AAAAvpList_S() { this->add(&Any); }

AAAAvpList_S::~AAAAvpList_S()
{
  std::list<AAADictionaryEntry*>::iterator i;
  pop_front();  // remove Any AVP
  for (i=begin(); i!=end(); i++) { delete *i; }
}

void
AAAAvpList_S::add(AAADictionaryEntry *avp)
{
  if (this->search(avp->avpName, avp->protocol) != NULL)
    {
      AAA_LOG(LM_ERROR, "duplicated AVP definition [%s].\n", 
		   avp->avpName.c_str());
      exit(1);
    }

  mutex.acquire();
  push_back(avp);
  mutex.release();
}

AAADictionaryEntry*
AAAAvpList_S::search(const std::string& avpName, int protocol)
{
  mutex.acquire();
  std::list<AAADictionaryEntry*>::iterator i;
  for (i = begin(); i!=end(); i++)
    {
      if ((*i)->avpName == avpName)
	{
            if (((*i)->avpCode != 0) &&
                (protocol >= 0) &&
                ((*i)->protocol != protocol)) {
                continue;
            }
	  mutex.release();
	  return *i;
	}
    }
  mutex.release();
  return NULL;
}

AAADictionaryEntry*
AAAAvpList_S::search(AAA_AVPCode code, AAAVendorId vendor,
                     int protocol)
{
  mutex.acquire();
  std::list<AAADictionaryEntry*>::iterator i;
  for (i = begin(); i!=end(); i++)
    {
        if (((*i)->avpCode == code) && ((*i)->vendorId == vendor))
	{
            if (((*i)->avpCode != 0) &&
                (protocol >= 0) &&
                ((*i)->protocol != protocol)) {
                continue;
            }
	  mutex.release();
	  return *i;
	}
    }
  mutex.release();
  return NULL;
}


