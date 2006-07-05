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
/* $Id: parser_avp.cxx,v 1.39 2006/03/16 17:01:50 vfajardo Exp $ */

#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <string>
#include "parser.h"
#include "parser_avp.h"
#include "parser_q_avplist.h"
#include "resultcodes.h"
#include "parser_avpvalue.h"

static AvpValueParser*
createAvpValueParser(AAA_AVPDataType type)// throw(AAAErrorStatus)
{
  AAAErrorStatus st;
  AvpType *t = AvpTypeList::instance()->search(type);
  if (t == NULL)
    {
      // Since it has been proven that the implementation recognize
      // the specified type as a result of successful
      // AAADictionary search by qualified AVP list parsre, if
      // the AVP value parser for the corresponding type is not found,
      // it must be the implementation bug.

      AAA_LOG(LM_ERROR, "Specified avp type not found");
      st.set(BUG, MISSING_AVP_DICTIONARY_ENTRY);
      throw (st);
    }

  AvpValueParser *p = t->createParser();
  if (p == NULL)
    {
      // This is also a bug for the same reason as above.
      AAA_LOG(LM_ERROR, "Avp value parser not found");
      st.set(BUG, MISSING_AVP_VALUE_PARSER);
      throw (st);
    }

  return p;
}

static int
checkFlags(struct avp_flag flag, AAA_AVPFlag flags)
{
  if (flag.m == 0 && (flags & AAA_AVP_FLAG_MANDATORY))
    {
      AAA_LOG(LM_ERROR, "M-flag must be set\n");
      return -1;
    }
  if (flag.m == 1 && (flags & AAA_AVP_FLAG_MANDATORY) == 0)
    {
      AAA_LOG(LM_ERROR, "M-flag must not be set\n");
      return -1;
    }
  if (flag.v == 0 && (flags & AAA_AVP_FLAG_VENDOR_SPECIFIC))
    {
      AAA_LOG(LM_ERROR, "V-flag needs to be set\n");
      return -1;
    }
  if (flag.v == 1 && (flags & AAA_AVP_FLAG_VENDOR_SPECIFIC) == 0)
    {
      AAA_LOG(LM_ERROR, "V-flag must not be set\n");
      return -1;
    }
  if (flag.p == 1 && (flags & AAA_AVP_FLAG_END_TO_END_ENCRYPT) == 0)
    {
      AAA_LOG(LM_ERROR, "P-flag must not be set\n");
      return -1;
    }
  return 0;
}

template<> void
AvpHeaderParser::parseRawToApp()// throw(AAAErrorStatus)
{
  AvpRawData *rawData = getRawData();
  AAAAvpHeader *h = getAppData();
  AAADictionaryEntry *avp = getDictData();

  AAAAvpHeaderList *ahl = rawData->ahl;
  AAAAvpHeaderList::iterator i;
  AAAErrorStatus st;
  AAAAvpParseType parseType = h->ParseType();

  if (! avp)
  {
	AAA_LOG(LM_ERROR, "AVP dictionary cannot be null.");
	st.set(BUG, MISSING_AVP_DICTIONARY_ENTRY);
	throw st;
  }
  if (ahl->empty())
    {
      st.set(NORMAL, AAA_MISSING_AVP, avp, getCodec());
      throw st;
    }

  if (parseType == PARSE_TYPE_FIXED_HEAD || 
      parseType == PARSE_TYPE_FIXED_TAIL)
    {
      if (avp->avpCode == 0)
	  {
		  AAA_LOG(LM_ERROR, "Wildcard AVP cannot be a fixed AVP.");
		  st.set(BUG, INVALID_CONTAINER_PARAM);
		  throw st;
	  }
      i = (parseType == PARSE_TYPE_FIXED_HEAD ? 
	   ahl->begin() : --ahl->end());
      if ((*i).code == avp->avpCode)
	{
	  *h = *i; ahl->erase(i);
	  h->value_p += AVP_HEADER_LEN(avp);  // restore original value_p
	  return;
	}
    }
  else
    {
      for (i=ahl->begin(); i!=ahl->end(); i++) {
	// Wildcard AVPs match any AVPs.
	if (avp->avpCode != 0)
	  {
	    // For non-wildcard AVPs, strict checking on v-flag, Vencor-Id
	    // and AVP code is performed.
	    if ((*i).flag.v != 
		((avp->flags & AAA_AVP_FLAG_VENDOR_SPECIFIC) ? 1 : 0))
	      { continue; }
	    if ((*i).vendor != avp->vendorId)
	      { continue; }
	    if ((*i).code != avp->avpCode) 
	      { continue; }
	  }

	*h = *i; ahl->erase(i);
	if (avp->avpCode > 0 && checkFlags(h->flag, avp->flags) != 0)
	  {
	    st.set(NORMAL, AAA_INVALID_AVP_BITS, avp, getCodec());
	    throw st;
	  }
	h->value_p += AVP_HEADER_LEN(avp);  // restore original value_p
	return;
      }
    }

  st.set(NORMAL, AAA_MISSING_AVP, avp, getCodec());
  throw st;
}

/* a set of functions used for payload construction */
template<> void
AvpHeaderParser::parseAppToRaw()// throw(AAAErrorStatus)
{
  AvpRawData *rawData = getRawData();
  AAAAvpHeader *h = getAppData();
  AAADictionaryEntry *avp = getDictData();

  AAAMessageBlock *aBuffer = rawData->msg;
  char *p=aBuffer->wr_ptr();
  AAAErrorStatus st;

  /* length check */
  if ((unsigned)AVP_HEADER_LEN(avp) > aBuffer->size())
    {
      AAA_LOG(LM_ERROR, "Header buffer overflow\n");
      st.set(BUG, INVALID_PARSER_USAGE);
      throw st;
    }

  /* codec checks */
  if (codec == NULL)
    {
      AAA_LOG(LM_ERROR, "No codec available\n");
      st.set(BUG, INVALID_PARSER_USAGE);
      throw st;
    }
    
  /* header encoding */
  codec->setRawData(std::pair<char*, int>(p, aBuffer->size()));
  codec->setAppData(h);
  codec->parseAppToRaw();
      
  aBuffer->wr_ptr(codec->getRawData().first);
}

template<> void
AvpParser::parseRawToApp()// throw(AAAErrorStatus)
{
  AvpRawData* rawData = getRawData();
  AAAAvpContainer *c = getAppData();
  AAADictionaryEntry *avp = getDictData();
  
  AAAErrorStatus st;
  AAAMessageBlock *aBuffer;

  for (int i=0; ; i++) 
    {
      AAAAvpContainerEntryManager em;
      AvpValueParser *vp;
      AAAAvpContainerEntry* e;
      AAAAvpHeader h;
      h.ParseType() = c->ParseType();
      
      /* header check */
      AvpHeaderParser hp;
      hp.setRawData(rawData);
      hp.setAppData(&h);
      hp.setDictData(avp);
      hp.setCodec(getCodec());
      try {
	hp.parseRawToApp();
      }
      catch (AAAErrorStatus &st)
	{
	  int type, code;
	  st.get(type, code);

	  if (i>0 && type == NORMAL && code == AAA_MISSING_AVP)
	    {
	      // return if no more entry is found once after getting 
	      // at lease one entry received.
	      return;
	    }
	  throw st;
	}

      /* payload check */
      e = em.acquire(avp->avpType);
      c->add(e);

      try {
	vp = createAvpValueParser(avp->avpType);
      }
      catch (AAAErrorStatus &st) {
	throw st;
      }
	
      aBuffer = 
	AAAMessageBlock::Acquire(h.value_p, h.length-AVP_HEADER_LEN(avp));
      vp->setRawData(aBuffer);
      vp->setAppData(e);
      vp->setDictData(avp);
      vp->setCodec(getCodec());

      try {
	vp->parseRawToApp();
      }
      catch (AAAErrorStatus &st)
	{
	  aBuffer->Release();
	  delete vp;
	  throw st;
	}	
      delete vp;
      aBuffer->Release();
    }
}

template<> void
AvpParser::parseAppToRaw()// throw(AAAErrorStatus)
{
  AvpRawData* rawData = getRawData();
  AAAAvpContainer *c = getAppData();
  AAADictionaryEntry *avp = getDictData();

  AAAMessageBlock *aBuffer = rawData->msg;
  AAAAvpHeader h;
  AAAErrorStatus st;
  if (!avp)
  {
	  AAA_LOG(LM_ERROR, "AVP dictionary cannot be null.");
	  st.set(BUG, MISSING_AVP_DICTIONARY_ENTRY);
	  throw st;
  }
  h.ParseType() = c->ParseType();

  if (avp->avpType == AAA_AVP_DATA_TYPE)  /* Any AVP */
    {
      for (unsigned int i=0; i<c->size(); i++)
	{
	  AvpValueParser *vp;
	  try {
	    vp = createAvpValueParser(avp->avpType);
	  }
	  catch (AAAErrorStatus &st) {
	    throw st;
	  }
	  vp->setRawData(aBuffer);
	  vp->setAppData((*c)[i]);
	  vp->setDictData(avp);
	  vp->setCodec(getCodec());
	  vp->parseAppToRaw();

          aBuffer->wr_ptr
	    (aBuffer->base() + 
	     adjust_word_boundary(aBuffer->wr_ptr() - aBuffer->base()));

	  delete vp;

	}
      return;
    }

  for (unsigned int i=0; i<c->size(); i++)
    {
      char *saved_p = aBuffer->wr_ptr();
      
      ACE_OS::memset(&h, 0, sizeof(h));
      h.code = avp->avpCode;
      h.flag.v = (avp->flags & AAA_AVP_FLAG_VENDOR_SPECIFIC) ? 1 : 0;
      h.flag.m = (avp->flags & AAA_AVP_FLAG_MANDATORY) ? 1 : 0;
      h.flag.p = (avp->flags & AAA_AVP_FLAG_END_TO_END_ENCRYPT) ? 1 : 0;
      h.vendor = avp->vendorId;
      
      AvpHeaderParser hp;
      hp.setRawData(rawData);
      hp.setAppData(&h);
      hp.setDictData(avp);
      hp.setCodec(getCodec());
      hp.parseAppToRaw();

      AvpValueParser *vp = createAvpValueParser(avp->avpType);
      vp->setRawData(aBuffer);
      vp->setAppData((*c)[i]);
      vp->setDictData(avp);
      vp->setCodec(getCodec());
      vp->parseAppToRaw();
      delete vp;
      // calculate the actual header length 
      h.length = ACE_UINT32(aBuffer->wr_ptr() - saved_p);

      // save the current write pointer
      saved_p = aBuffer->base() +
	adjust_word_boundary(aBuffer->wr_ptr() - aBuffer->base());

      // set the header again
      aBuffer->wr_ptr(aBuffer->wr_ptr() - h.length);
      hp.parseAppToRaw();

      // restore the write pointer
      aBuffer->wr_ptr(saved_p);
    }
}

