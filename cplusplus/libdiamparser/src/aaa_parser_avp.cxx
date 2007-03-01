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
/* $Id: parser_avp.cxx,v 1.39 2006/03/16 17:01:50 vfajardo Exp $ */

#include <ace/OS.h>
#include <ace/Log_Msg.h>
#include <string>
#include "resultcodes.h"
#include "aaa_parser_avp.h"
#include "aaa_parser_q_avplist.h"
#include "aaa_parser_avpvalue.h"

static DiameterAvpValueParser*
createAvpValueParser(AAAAvpDataType type)// throw(DiameterErrorCode)
{
  DiameterErrorCode st;
  DiameterAvpType *t = (DiameterAvpType*)DiameterAvpTypeList::instance()->search(type);
  if (t == NULL)
    {
      // Since it has been proven that the implementation recognize
      // the specified type as a result of successful
      // DiameterDictionary search by qualified AVP list parsre, if
      // the AVP value parser for the corresponding type is not found,
      // it must be the implementation bug.

      AAA_LOG((LM_ERROR, "Specified avp type not found"));
      st.set(AAA_PARSE_ERROR_TYPE_BUG, AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY);
      throw (st);
    }

  DiameterAvpValueParser *p = t->createParser();
  if (p == NULL)
    {
      // This is also a bug for the same reason as above.
      AAA_LOG((LM_ERROR, "Avp value parser not found"));
      st.set(AAA_PARSE_ERROR_TYPE_BUG, AAA_PARSE_ERROR_MISSING_AVP_VALUE_PARSER);
      throw (st);
    }

  return p;
}

static int
checkFlags(struct diameter_avp_flag flag, AAAAVPFlag flags)
{
  if (flag.m == 0 && (flags & DIAMETER_AVP_FLAG_MANDATORY))
    {
      AAA_LOG((LM_ERROR, "M-flag must be set\n"));
      return -1;
    }
  if (flag.m == 1 && (flags & DIAMETER_AVP_FLAG_MANDATORY) == 0)
    {
      AAA_LOG((LM_ERROR, "M-flag must not be set\n"));
      return -1;
    }
  if (flag.v == 0 && (flags & DIAMETER_AVP_FLAG_VENDOR_SPECIFIC))
    {
      AAA_LOG((LM_ERROR, "V-flag needs to be set\n"));
      return -1;
    }
  if (flag.v == 1 && (flags & DIAMETER_AVP_FLAG_VENDOR_SPECIFIC) == 0)
    {
      AAA_LOG((LM_ERROR, "V-flag must not be set\n"));
      return -1;
    }
  if (flag.p == 1 && (flags & DIAMETER_AVP_FLAG_END_TO_END_ENCRYPT) == 0)
    {
      AAA_LOG((LM_ERROR, "P-flag must not be set\n"));
      return -1;
    }
  return 0;
}

template<> void
DiameterAvpHeaderParser::parseRawToApp()// throw(DiameterErrorCode)
{
  DiameterAvpRawData *rawData = getRawData();
  DiameterAvpHeader *h = getAppData();
  AAADictionaryEntry *avp = getDictData();

  DiameterAvpHeaderList *ahl = rawData->ahl;
  DiameterAvpHeaderList::iterator i;
  DiameterErrorCode st;
  AAAAvpParseType parseType = h->ParseType();

  if (! avp)
  {
	AAA_LOG((LM_ERROR, "AVP dictionary cannot be null."));
	st.set(AAA_PARSE_ERROR_TYPE_BUG,
               AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY);
	throw st;
  }
  if (ahl->empty())
    {
      st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_MISSING_AVP, avp);
      throw st;
    }

  if (parseType == AAA_PARSE_TYPE_FIXED_HEAD)
    {
      if (avp->avpCode == 0)
	  {
            AAA_LOG((LM_ERROR, "Wildcard AVP cannot be a fixed AVP."));
            st.set(AAA_PARSE_ERROR_TYPE_BUG,
                   AAA_PARSE_ERROR_INVALID_CONTAINER_PARAM);
            throw st;
	  }
      i = ahl->begin();
      if ((*i).code == avp->avpCode)
	{
	  *h = *i; ahl->erase(i);
	  h->value_p += DIAMETER_AVP_HEADER_LEN(avp);  // restore original value_p
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
		((avp->flags & DIAMETER_AVP_FLAG_VENDOR_SPECIFIC) ? 1 : 0))
	      { continue; }
	    if ((*i).vendor != avp->vendorId)
	      { continue; }
	    if ((*i).code != avp->avpCode) 
	      { continue; }
	  }

	*h = *i; ahl->erase(i);
	if (avp->avpCode > 0 && checkFlags(h->flag, avp->flags) != 0)
	  {
	    st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_AVP_BITS, avp);
	    throw st;
	  }
	h->value_p += DIAMETER_AVP_HEADER_LEN(avp);  // restore original value_p
	return;
      }
    }

  st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_MISSING_AVP, avp);
  throw st;
}

/* a set of functions used for payload construction */
template<> void
DiameterAvpHeaderParser::parseAppToRaw()// throw(DiameterErrorCode)
{
  DiameterAvpRawData *rawData = getRawData();
  DiameterAvpHeader *h = getAppData();
  AAADictionaryEntry *avp = getDictData();

  AAAMessageBlock *aBuffer = rawData->msg;
  char *p=aBuffer->wr_ptr();
  DiameterErrorCode st;

  /* length check */
  if ((unsigned)DIAMETER_AVP_HEADER_LEN(avp) >
      (aBuffer->size() - (aBuffer->wr_ptr() - aBuffer->base())))
    {
      AAA_LOG((LM_ERROR, "Header buffer overflow\n"));
      st.set(AAA_PARSE_ERROR_TYPE_BUG, AAA_PARSE_ERROR_INVALID_PARSER_USAGE);
      throw st;
    }

  *((ACE_UINT32*)p) = ACE_HTONL(h->code);
  p+=4;
  /* initialize this field to prepare for bit OR operation */
  *((ACE_UINT32*)p) = 0;
  if (h->flag.v)
    {
      *p|=0x80;
    }
  if (h->flag.m)
    {
      *p|=0x40;
    }
  if (h->flag.p)
    {
      *p|=0x20;
    }

  *((ACE_UINT32*)p) |= ACE_HTONL(h->length & 0x00ffffff);
  p+=4;

  if (h->flag.v)
    {
      *((ACE_UINT32*)p) = ACE_HTONL(h->vendor);
      p+=4;
    }

  aBuffer->wr_ptr(p);
}

template<> void
DiameterAvpParser::parseRawToApp()// throw(DiameterErrorCode)
{
  DiameterAvpRawData* rawData = getRawData();
  AAAAvpContainer *c = getAppData();
  AAADictionaryEntry *avp = getDictData();

  DiameterErrorCode st;
  AAAMessageBlock *aBuffer;

  for (int i=0; ; i++) 
    {
      DiameterAvpContainerEntryManager em;
      DiameterAvpValueParser *vp;
      AAAAvpContainerEntry* e;
      DiameterAvpHeader h;
      h.ParseType() = c->ParseType();

      /* header check */
      DiameterAvpHeaderParser hp;
      hp.setRawData(rawData);
      hp.setAppData(&h);
      hp.setDictData(avp);
      try {
	hp.parseRawToApp();
      }
      catch (DiameterErrorCode &st)
	{
	  AAA_PARSE_ERROR_TYPE type;
          int code;
	  st.get(type, code);

	  if (i>0 && type == AAA_PARSE_ERROR_TYPE_NORMAL && code == AAA_MISSING_AVP)
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
      catch (DiameterErrorCode &st) {
	throw st;
      }
	
      aBuffer = 
	AAAMessageBlock::Acquire(h.value_p, h.length-DIAMETER_AVP_HEADER_LEN(avp));
      vp->setRawData(aBuffer);
      vp->setAppData(e);
      vp->setDictData(avp);

      try {
	vp->parseRawToApp();
      }
      catch (DiameterErrorCode &st)
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
DiameterAvpParser::parseAppToRaw()// throw(DiameterErrorCode)
{
  DiameterAvpRawData* rawData = getRawData();
  AAAAvpContainer *c = getAppData();
  AAADictionaryEntry *avp = getDictData();

  AAAMessageBlock *aBuffer = rawData->msg;
  DiameterAvpHeader h;
  DiameterErrorCode st;
  if (!avp)
  {
	  AAA_LOG((LM_ERROR, "AVP dictionary cannot be null."));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG, AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY);
	  throw st;
  }
  h.ParseType() = c->ParseType();

  if (avp->avpType == AAA_AVP_DATA_TYPE)  /* Any AVP */
    {
      for (unsigned int i=0; i<c->size(); i++)
	{
	  DiameterAvpValueParser *vp;
	  try {
	    vp = createAvpValueParser(avp->avpType);
	  }
	  catch (DiameterErrorCode &st) {
	    throw st;
	  }
	  vp->setRawData(aBuffer);
	  vp->setAppData((*c)[i]);
	  vp->setDictData(avp);
	  vp->parseAppToRaw();

          if (adjust_word_boundary(aBuffer->wr_ptr() - aBuffer->base()) <=
              aBuffer->size()) {
              aBuffer->wr_ptr
	        (aBuffer->base() + 
	         adjust_word_boundary(aBuffer->wr_ptr() - aBuffer->base()));
          }
          else {
              AAA_LOG((LM_ERROR, "AVP value parsing, out of space "));
	      st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_OUT_OF_SPACE);
	      throw st;
          }

	  delete vp;

	}
      return;
    }

  for (unsigned int i=0; i<c->size(); i++)
    {
      char *saved_p = aBuffer->wr_ptr();
      
      ACE_OS::memset(&h, 0, sizeof(h));
      h.code = avp->avpCode;
      h.flag.v = (avp->flags & DIAMETER_AVP_FLAG_VENDOR_SPECIFIC) ? 1 : 0;
      h.flag.m = (avp->flags & DIAMETER_AVP_FLAG_MANDATORY) ? 1 : 0;
      h.flag.p = (avp->flags & DIAMETER_AVP_FLAG_END_TO_END_ENCRYPT) ? 1 : 0;
      h.vendor = avp->vendorId;
      
      DiameterAvpHeaderParser hp;
      hp.setRawData(rawData);
      hp.setAppData(&h);
      hp.setDictData(avp);
      hp.parseAppToRaw();

      DiameterAvpValueParser *vp = createAvpValueParser(avp->avpType);
      vp->setRawData(aBuffer);
      vp->setAppData((*c)[i]);
      vp->setDictData(avp);
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

