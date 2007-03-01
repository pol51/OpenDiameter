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
/* $Id: parser.cxx,v 1.31 2006/04/05 17:57:41 vfajardo Exp $ */
#include <ace/OS.h>
#include <list>
#include <typeinfo>
#include "aaa_comlist.h"
#include "aaa_avplist.h"
#include "aaa_q_avplist.h"
#include "aaa_parser_avp.h"
#include "aaa_parser_q_avplist.h"
#include "resultcodes.h"    /* file location will be changed */
#include "diameter_parser.h"

template<> void
DiameterMsgHeaderParser::parseRawToApp()// throw(DiameterErrorCode)
{
  AAAMessageBlock *aBuffer = getRawData();
  DiameterMsgHeader *aHeader = getAppData();
  DiameterParseOption opt = getDictData();

  DiameterMsgHeader &h = *aHeader;
  aBuffer->rd_ptr(aBuffer->base());
  char *p = aBuffer->rd_ptr();
  DiameterErrorCode st;
  DiameterCommand *com;

  h.ver = *((AAAUInt8*)(p));
  h.length = ntohl(*((ACE_UINT32*)(p))) & 0x00ffffff;
  p += 4;
  h.flags.r = *((AAAUInt8*)(p)) >> 7;
  h.flags.p = *((AAAUInt8*)(p)) >> 6;
  h.flags.e = *((AAAUInt8*)(p)) >> 5;
  h.flags.t = *((AAAUInt8*)(p)) >> 4;

  h.code = ntohl(*((ACE_UINT32*)(p))) & 0x00ffffff;
  p += 4;
  h.appId = ntohl(*((ACE_UINT32*)(p)));
  p += 4;
  h.hh = ntohl(*((ACE_UINT32*)(p)));
  p += 4;
  h.ee = ntohl(*((ACE_UINT32*)(p)));
  p += 4;

  aBuffer->rd_ptr(p);

  if (opt == DIAMETER_PARSE_LOOSE)
    {
      return;
    }

  if ((com = DiameterCommandList::instance()
       ->search(h.code, h.appId, h.flags.r)) == NULL)
  {
    AAA_LOG((LM_ERROR, "command (%d,r-flag=%1d) not found\n", 
	       h.code, h.flags.r));
    st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_COMMAND_UNSUPPORTED);
    throw st;
  }

  if (com->flags.r && h.flags.e)
    {
      AAA_LOG((LM_ERROR, 
		      "command (%d) e flag cannot be set for request\n", 
		      h.code));
      st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_BIT_IN_HEADER);
      throw st;
    }

  if (com->flags.p == 0 && h.flags.p == 1)
    {
      AAA_LOG((LM_ERROR, 
		      "command (%d) contains mismatch in p flag\n", 
		      h.code));
      st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_BIT_IN_HEADER);
      throw st;
    }

  /* special handling for answer message with error flag set */
  if (! com->flags.r && h.flags.e)
    {
      if (h.flags.p)
	{
	  com = DiameterCommandList::instance()
	    ->search("PROXYABLE-ERROR-Answer");
	}
      else
	{
	  com = DiameterCommandList::instance()
	    ->search("NON-PROXYABLE-ERROR-Answer");
	}
      if (com == NULL)
	{
	  AAA_LOG((LM_ERROR, 
			  "No way to handle answer with error\n"));
	  st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_COMMAND_UNSUPPORTED);
	  throw st;
	}
    }
  h.dictHandle = com;
}

template<> void
DiameterMsgHeaderParser::parseAppToRaw()// throw(DiameterErrorCode)
{
  AAAMessageBlock *aBuffer = getRawData();
  DiameterMsgHeader *aHeader = getAppData();
  DiameterParseOption opt = getDictData();
  DiameterMsgHeader &h = *aHeader;
  char *p = aBuffer->base();
  DiameterErrorCode st;
  DiameterCommand *com=0;

  aBuffer->wr_ptr(aBuffer->base());

  if (opt != DIAMETER_PARSE_LOOSE)
    {

      if ((com = DiameterCommandList::instance()->
	   search(h.code, h.appId, h.flags.r)) == NULL)
	{
	  AAA_LOG((LM_ERROR, "command (%d,r-flag=%1d) not found\n", 
			  h.code, h.flags.r));
	  st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_COMMAND_UNSUPPORTED);
	  throw st;
	}

      if (com->flags.r && h.flags.e)
	{
	  AAA_LOG((LM_ERROR, 
			  "command (%d) e flag cannot be set for request\n", 
			  h.code));
	  st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_HDR_BITS);
	  throw st;
	}

      if (com->flags.p == 0 && h.flags.p == 1)
	{
	  AAA_LOG((LM_ERROR, 
			  "command (%d) contains mismatch in p flag\n", 
			  h.code));
	  st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_INVALID_HDR_BITS);
	  throw st;
	}

      /* special handling for answer message with error flag set */
      if (! com->flags.r && h.flags.e)
	{
	  if (h.flags.p)
	    {
	      com = DiameterCommandList::instance()->search("PROXYABLE-ERROR-Answer");
	    }
	  else
	    {
	      com = DiameterCommandList::instance()->search("NON-PROXYABLE-ERROR-Answer");
	    }
	  if (com == NULL)
	    {
	      AAA_LOG((LM_ERROR, 
			      "No way to handle answer with error\n"));
	      st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_COMMAND_UNSUPPORTED);
	      throw st;
	    }
	}
    }

  *((ACE_UINT32*)(p)) = ntohl(h.length);
  *((AAAUInt8*)(p)) = h.ver;
  p += 4;
  *((ACE_UINT32*)(p)) = ntohl(h.code);
  *((AAAUInt8*)(p)) = 
    (h.flags.r << 7) | (h.flags.p << 6)  | 
    (h.flags.e << 5) | (h.flags.t << 4);;

  p += 4;
  *((ACE_UINT32*)(p)) = ntohl(h.appId);
  p += 4;
  *((ACE_UINT32*)(p)) = ntohl(h.hh);
  p += 4;
  *((ACE_UINT32*)(p)) = ntohl(h.ee);
  p += 4;

  aBuffer->wr_ptr(p);
  h.dictHandle = com;
}

template<> void
DiameterMsgPayloadParser::parseRawToApp()// throw(DiameterErrorCode)
{
  AAAMessageBlock *aBuffer = getRawData();
  AAAAvpContainerList *acl = getAppData();
  DiameterDictionary *dict = (DiameterDictionary*)getDictData();

  QualifiedAvpListParser qc;
  qc.setRawData(aBuffer);
  qc.setAppData(acl);
  qc.setDictData(dict);

  try {
    qc.parseRawToApp();
  }
  catch (DiameterErrorCode st) {
    AAA_LOG((LM_ERROR, "Parse error"));
    throw;
  }
}

template<> void
DiameterMsgPayloadParser::parseAppToRaw()// throw(DiameterErrorCode)
{
  AAAMessageBlock *aBuffer = getRawData();
  AAAAvpContainerList *acl = getAppData();
  DiameterDictionary *dict = (DiameterDictionary*)getDictData();

  ACE_OS::memset(aBuffer->wr_ptr(), 0,
		 aBuffer->size() - (size_t)aBuffer->wr_ptr() +
		 (size_t)aBuffer->base());

  QualifiedAvpListParser qc;
  qc.setRawData(aBuffer);
  qc.setAppData(acl);
  qc.setDictData(dict);

  try {
    qc.parseAppToRaw();
  }
  catch (DiameterErrorCode &st) {
    AAA_LOG((LM_ERROR, "Parse error"));
    throw;
  }
}

const char* DiameterMsgHeader::getCommandName()
{
  return dictHandle ? ((DiameterCommand*)dictHandle)->name.c_str() : NULL;
}

