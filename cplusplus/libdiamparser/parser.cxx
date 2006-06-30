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
/* $Id: parser.cxx,v 1.31 2006/04/05 17:57:41 vfajardo Exp $ */
#include <ace/OS.h>
#include <list>
#include <typeinfo>
#include "comlist.h"
#include "avplist.h"
#include "q_avplist.h"
#include "parser.h"
#include "parser_avp.h"
#include "parser_q_avplist.h"
#include "resultcodes.h"    /* file location will be changed */
#include "diameter_parser_api.h"

template<> void
HeaderParser::parseRawToApp()// throw(AAAErrorStatus)
{
  AAAMessageBlock *aBuffer = getRawData();
  AAADiameterHeader *aHeader = getAppData();
  ParseOption opt = getDictData();

  AAADiameterHeader &h = *aHeader;
  aBuffer->rd_ptr(aBuffer->base());
  char *p = aBuffer->rd_ptr();
  AAAErrorStatus st;
  AAACommand *com;

  h.ver = *((AAA_UINT8*)(p));
  h.length = ntohl(*((ACE_UINT32*)(p))) & 0x00ffffff;
  p += 4;
  h.flags.r = *((AAA_UINT8*)(p)) >> 7;
  h.flags.p = *((AAA_UINT8*)(p)) >> 6;
  h.flags.e = *((AAA_UINT8*)(p)) >> 5;
  h.flags.t = *((AAA_UINT8*)(p)) >> 4;

  h.code = ntohl(*((ACE_UINT32*)(p))) & 0x00ffffff;
  p += 4;
  h.appId = ntohl(*((ACE_UINT32*)(p)));
  p += 4;
  h.hh = ntohl(*((ACE_UINT32*)(p)));
  p += 4;
  h.ee = ntohl(*((ACE_UINT32*)(p)));
  p += 4;

  aBuffer->rd_ptr(p);

  if (opt == PARSE_LOOSE)
    {
      return;
    }

  if ((com = AAACommandList::instance()
       ->search(h.code, h.appId, h.flags.r)) == NULL)
  {
    AAA_LOG(LM_ERROR, "command (%d,r-flag=%1d) not found\n", 
	       h.code, h.flags.r);
    st.set(NORMAL, AAA_COMMAND_UNSUPPORTED);
    throw st;
  }

  if (com->flags.r && h.flags.e)
    {
      AAA_LOG(LM_ERROR, 
		      "command (%d) e flag cannot be set for request\n", 
		      h.code);
      st.set(NORMAL, AAA_INVALID_BIT_IN_HEADER);
      throw st;
    }

  if (com->flags.p == 0 && h.flags.p == 1)
    {
      AAA_LOG(LM_ERROR, 
		      "command (%d) contains mismatch in p flag\n", 
		      h.code);
      st.set(NORMAL, AAA_INVALID_BIT_IN_HEADER);
      throw st;
    }

  /* special handling for answer message with error flag set */
  if (! com->flags.r && h.flags.e)
    {
      if (h.flags.p)
	{
	  com = AAACommandList::instance()
	    ->search("PROXYABLE-ERROR-Answer");
	}
      else
	{
	  com = AAACommandList::instance()
	    ->search("NON-PROXYABLE-ERROR-Answer");
	}
      if (com == NULL)
	{
	  AAA_LOG(LM_ERROR, 
			  "No way to handle answer with error\n");
	  st.set(NORMAL, AAA_COMMAND_UNSUPPORTED);
	  throw st;
	}
    }
  h.dictHandle = com;
}

template<> void
HeaderParser::parseAppToRaw()// throw(AAAErrorStatus)
{
  AAAMessageBlock *aBuffer = getRawData();
  AAADiameterHeader *aHeader = getAppData();
  ParseOption opt = getDictData();
  AAADiameterHeader &h = *aHeader;
  char *p = aBuffer->base();
  AAAErrorStatus st;
  AAACommand *com=0;

  aBuffer->wr_ptr(aBuffer->base());

  if (opt != PARSE_LOOSE)
    {

      if ((com = AAACommandList::instance()->
	   search(h.code, h.appId, h.flags.r)) == NULL)
	{
	  AAA_LOG(LM_ERROR, "command (%d,r-flag=%1d) not found\n", 
			  h.code, h.flags.r);
	  st.set(NORMAL, AAA_COMMAND_UNSUPPORTED);
	  throw st;
	}

      if (com->flags.r && h.flags.e)
	{
	  AAA_LOG(LM_ERROR, 
			  "command (%d) e flag cannot be set for request\n", 
			  h.code);
	  st.set(NORMAL, AAA_INVALID_HDR_BITS);
	  throw st;
	}

      if (com->flags.p == 0 && h.flags.p == 1)
	{
	  AAA_LOG(LM_ERROR, 
			  "command (%d) contains mismatch in p flag\n", 
			  h.code);
	  st.set(NORMAL, AAA_INVALID_HDR_BITS);
	  throw st;
	}

      /* special handling for answer message with error flag set */
      if (! com->flags.r && h.flags.e)
	{
	  if (h.flags.p)
	    {
	      com = AAACommandList::instance()->search("PROXYABLE-ERROR-Answer");
	    }
	  else
	    {
	      com = AAACommandList::instance()->search("NON-PROXYABLE-ERROR-Answer");
	    }
	  if (com == NULL)
	    {
	      AAA_LOG(LM_ERROR, 
			      "No way to handle answer with error\n");
	      st.set(NORMAL, AAA_COMMAND_UNSUPPORTED);
	      throw st;
	    }
	}
    }

  *((ACE_UINT32*)(p)) = ntohl(h.length);
  *((AAA_UINT8*)(p)) = h.ver;
  p += 4;
  *((ACE_UINT32*)(p)) = ntohl(h.code);
  *((AAA_UINT8*)(p)) = 
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
PayloadParserWithEmptyCodec::parseRawToApp()// throw(AAAErrorStatus)
{
  AAAMessageBlock *aBuffer = getRawData();
  AAAAvpContainerList *acl = getAppData();
  AAADictionary *dict = (AAADictionary*)getDictData();
  AvpHeaderCodec *c = getCodec();

  QualifiedAvpListParser qc;
  qc.setRawData(aBuffer);
  qc.setAppData(acl);
  qc.setDictData(dict);
  qc.setCodec(c);

  try {
    qc.parseRawToApp();
  }
  catch (AAAErrorStatus st) {
    AAA_LOG(LM_ERROR, "Parse error");
    throw;
  }
}

template<> void
PayloadParserWithEmptyCodec::parseAppToRaw()// throw(AAAErrorStatus)
{
  AAAMessageBlock *aBuffer = getRawData();
  AAAAvpContainerList *acl = getAppData();
  AAADictionary *dict = (AAADictionary*)getDictData();
  AvpHeaderCodec *c = getCodec();

  ACE_OS::memset(aBuffer->wr_ptr(), 0, 
		 aBuffer->size() - (size_t)aBuffer->wr_ptr() +
		 (size_t)aBuffer->base());

  QualifiedAvpListParser qc;
  qc.setRawData(aBuffer);
  qc.setAppData(acl);
  qc.setDictData(dict);
  qc.setCodec(c);

  try {
    qc.parseAppToRaw();
  }
  catch (AAAErrorStatus st) {
    AAA_LOG(LM_ERROR, "Parse error");
    throw;
  }
}

#if 0
char* AAADiameterHeader::getCommandName()
{ return dictHandle ? ((AAACommand*)dictHandle)->name : NULL; }
#else
const char* AAADiameterHeader::getCommandName()
{ return dictHandle ? ((AAACommand*)dictHandle)->name.c_str() : NULL; }
#endif

template<> void
HeaderParserWithProtocol::parseRawToApp()// throw(AAAErrorStatus)
{
  AAAMessageBlock *aBuffer = getRawData();
  AAADiameterHeader *aHeader = getAppData();
  AAADictionaryOption *opt = getDictData();

  AAADiameterHeader &h = *aHeader;
  aBuffer->rd_ptr(aBuffer->base());
  char *p = aBuffer->rd_ptr();
  AAAErrorStatus st;
  AAACommand *com;

  h.ver = *((AAA_UINT8*)(p));
  h.length = ntohl(*((ACE_UINT32*)(p))) & 0x00ffffff;
  p += 4;
  h.flags.r = *((AAA_UINT8*)(p)) >> 7;
  h.flags.p = *((AAA_UINT8*)(p)) >> 6;
  h.flags.e = *((AAA_UINT8*)(p)) >> 5;
  h.flags.t = *((AAA_UINT8*)(p)) >> 4;

  h.code = ntohl(*((ACE_UINT32*)(p))) & 0x00ffffff;
  p += 4;
  h.appId = ntohl(*((ACE_UINT32*)(p)));
  p += 4;
  h.hh = ntohl(*((ACE_UINT32*)(p)));
  p += 4;
  h.ee = ntohl(*((ACE_UINT32*)(p)));
  p += 4;

  aBuffer->rd_ptr(p);

  if (opt->option == PARSE_LOOSE)
    {
      return;
    }

  if ((com = AAACommandList::instance()
       ->search(h.code, h.appId, h.flags.r, opt->protocolId)) == NULL)
  {
    AAA_LOG(LM_ERROR, "command (%d,r-flag=%1d) not found\n", 
	       h.code, h.flags.r);
    st.set(NORMAL, AAA_COMMAND_UNSUPPORTED);
    throw st;
  }

  if (com->flags.r && h.flags.e)
    {
      AAA_LOG(LM_ERROR, 
		      "command (%d) e flag cannot be set for request\n", 
		      h.code);
      st.set(NORMAL, AAA_INVALID_HDR_BITS);
      throw st;
    }

  if (com->flags.p == 0 && h.flags.p == 1)
    {
      AAA_LOG(LM_ERROR, 
		      "command (%d) contains mismatch in p flag\n", 
		      h.code);
      st.set(NORMAL, AAA_INVALID_HDR_BITS);
      throw st;
    }

  /* special handling for answer message with error flag set */
  if (! com->flags.r && h.flags.e)
    {
      if (h.flags.p)
	{
	  com = AAACommandList::instance()
	    ->search("PROXYABLE-ERROR-Answer", opt->protocolId);
	}
      else
	{
	  com = AAACommandList::instance()
	    ->search("NON-PROXYABLE-ERROR-Answer", opt->protocolId);
	}
      if (com == NULL)
	{
	  AAA_LOG(LM_ERROR, 
			  "No way to handle answer with error\n");
	  st.set(NORMAL, AAA_COMMAND_UNSUPPORTED);
	  throw st;
	}
    }
  h.dictHandle = com;
}

template<> void
HeaderParserWithProtocol::parseAppToRaw()// throw(AAAErrorStatus)
{
  AAAMessageBlock *aBuffer = getRawData();
  AAADiameterHeader *aHeader = getAppData();
  AAADictionaryOption *opt = getDictData();
  AAADiameterHeader &h = *aHeader;
  char *p = aBuffer->base();
  AAAErrorStatus st;
  AAACommand *com=0;

  aBuffer->wr_ptr(aBuffer->base());

  if (opt->option != PARSE_LOOSE)
    {

      if ((com = AAACommandList::instance()->
	   search(h.code, h.appId, h.flags.r, opt->protocolId)) == NULL)
	{
	  AAA_LOG(LM_ERROR, "command (%d,r-flag=%1d) not found\n", 
			  h.code, h.flags.r);
	  st.set(NORMAL, AAA_COMMAND_UNSUPPORTED);
	  throw st;
	}

      if (com->flags.r && h.flags.e)
	{
	  AAA_LOG(LM_ERROR, 
			  "command (%d) e flag cannot be set for request\n", 
			  h.code);
	  st.set(NORMAL, AAA_INVALID_HDR_BITS);
	  throw st;
	}

      if (com->flags.p == 0 && h.flags.p == 1)
	{
	  AAA_LOG(LM_ERROR, 
			  "command (%d) contains mismatch in p flag\n", 
			  h.code);
	  st.set(NORMAL, AAA_INVALID_HDR_BITS);
	  throw st;
	}

      /* special handling for answer message with error flag set */
      if (! com->flags.r && h.flags.e)
	{
	  if (h.flags.p)
	    {
              com = AAACommandList::instance()->search("PROXYABLE-ERROR-Answer",
                                                       opt->protocolId);
	    }
	  else
	    {
	      com = AAACommandList::instance()->search("NON-PROXYABLE-ERROR-Answer",
                                                       opt->protocolId);
	    }
	  if (com == NULL)
	    {
	      AAA_LOG(LM_ERROR, 
			      "No way to handle answer with error\n");
	      st.set(NORMAL, AAA_COMMAND_UNSUPPORTED);
	      throw st;
	    }
	}
    }

  *((ACE_UINT32*)(p)) = ntohl(h.length);
  *((AAA_UINT8*)(p)) = h.ver;
  p += 4;
  *((ACE_UINT32*)(p)) = ntohl(h.code);
  *((AAA_UINT8*)(p)) = 
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