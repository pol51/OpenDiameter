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
/* $Id: parser_q_avplist.cxx,v 1.28 2006/03/16 17:01:50 vfajardo Exp $ */
#include <sys/types.h>
#include <list>
#include "parser.h"
#include "parser_avp.h"
#include "parser_q_avplist.h"
#include "q_avplist.h"
#include "resultcodes.h"    

static 
void parseRawToAppWithoutDict(AAAAvpHeaderList *ahl,
                              AAAAvpContainerList *acl,
                              AvpHeaderCodec *codec);
  //     throw (AAAErrorStatus);

static
void parseRawToAppWithDict(AAAAvpHeaderList *ahl, AAAAvpContainerList *acl,
			   AAADictionary *dict, AvpHeaderCodec *codec);// throw (AAAErrorStatus);

static
void parseAppToRawWithoutDict(AAAMessageBlock *msg,
                              AAAAvpContainerList *acl,
                              AvpHeaderCodec *codec);
//     throw (AAAErrorStatus);

static
void parseAppToRawWithDict(AAAMessageBlock *msg, AAAAvpContainerList *acl,
			   AAADictionary *dict, AvpHeaderCodec *codec);// throw (AAAErrorStatus);

template<> void
QualifiedAvpListParser::parseRawToApp()// throw (AAAErrorStatus)
{
  AAAAvpHeaderList ahl(getCodec());
  // read buffer and create avp header list 
  try {
    ahl.create(getRawData());
  }
  catch (AAAErrorStatus &st)
    {
      throw st;
    }

  if (getDictData() == NULL)
    {
      ::parseRawToAppWithoutDict(&ahl, getAppData(), getCodec());
    }
  else
    {
      ::parseRawToAppWithDict(&ahl, getAppData(), getDictData(), getCodec());
    }
}

template<> void
QualifiedAvpListParser::parseAppToRaw()// throw (AAAErrorStatus)
{

  if (getDictData() == NULL)
    {
      ::parseAppToRawWithoutDict(getRawData(), getAppData(), getCodec());
    }
  else
    {
      ::parseAppToRawWithDict(getRawData(), getAppData(), getDictData(), getCodec());
    }
}

static void
parseRawToAppWithoutDict(AAAAvpHeaderList *ahl, 
                         AAAAvpContainerList *acl,
                         AvpHeaderCodec *codec)
  //  throw (AAAErrorStatus)
{
  AAAAvpContainer *c;
  AAAErrorStatus st;
  const char *name;
  AAAAvpContainerList::iterator i;

  for (i = acl->begin(); i != acl->end(); i++)
    {
      c = *i;
      c->ParseType() = PARSE_TYPE_OPTIONAL;
      name = c->getAvpName();
#ifdef DEBUG
      cout << __FUNCTION__ << ": Container "<< name << "matches\n";
#endif

      AAADictionaryEntry* avp;
      // use default dictionary only
      if ((avp = AAAAvpList::instance()->search(name)) == NULL)
	{
	  AAA_LOG(LM_ERROR, "No dictionary entry for %s avp.\n", name);
	  st.set(BUG, MISSING_AVP_DICTIONARY_ENTRY);
	  throw;
	}

      do 
	{
	  AvpParser ap;
	  AvpRawData rawData;
	  rawData.ahl = ahl;
	  ap.setRawData(&rawData);
	  ap.setAppData(c);
	  ap.setDictData(avp);
	  ap.setCodec(codec);
	  try {
	    ap.parseRawToApp();
	  }
	  catch (AAAErrorStatus &st)
	    {
	      int type, code;
	      st.get(type, code);
	      if (type == NORMAL && code == AAA_MISSING_AVP)
		{
		  continue;
		}
	      else
		{
		  // Parse error 
		  AAA_LOG(LM_ERROR, "Error in AVP %s.\n", name);
		  throw;
		}
	    }
	} while (0);
    }
}

static void
parseRawToAppWithDict(AAAAvpHeaderList *ahl, AAAAvpContainerList *acl,
		      AAADictionary *dict, AvpHeaderCodec *codec)// throw (AAAErrorStatus)
{
  AAAQualifiedAVP *qavp;
  AAAAvpContainer *c;
  AAAErrorStatus st;
  AAAAvpContainerManager cm;
  unsigned int min, max;
  const char *name;
  int type;
  AAAQualifiedAvpList::iterator i;
  AAAAvpParseType pt;

  AAAQualifiedAvpList *qavp_l[4] = 
    {dict->avp_f, dict->avp_f2, dict->avp_r, dict->avp_o};

  for (int j=0; j<4; j++)
    {
      for (i = qavp_l[j]->begin(); i != qavp_l[j]->end(); i++)
	{
	  pt = qavp_l[j]->getParseType();
	  qavp = *i;
	  min = qavp->qual.min;
	  max = qavp->qual.max;
	  name = qavp->avp->avpName.c_str();
	  type = qavp->avp->avpType;
#ifdef DEBUG
	  cout << __FUNCTION__ << ": Container "<< name << "matches\n";
#endif

	  c = cm.acquire(name);
	  c->ParseType() = pt;

	  do 
	    {
	      AvpParser ap;
	      AvpRawData rawData;
	      rawData.ahl = ahl;
	      ap.setRawData(&rawData);
	      ap.setAppData(c);
	      ap.setDictData(qavp->avp);
	      ap.setCodec(codec);
	      try {
		ap.parseRawToApp();
	      }
	      catch (AAAErrorStatus &st)
		{
		  int type, code;
		  st.get(type, code);
		  if (type == NORMAL && code == AAA_MISSING_AVP)
		    {
		      // AVP was not found
		      c->releaseEntries();
		      cm.release(c);

		      if (pt == PARSE_TYPE_OPTIONAL) 
			continue;

            if (0 == min)
               continue;

		      AAA_LOG(LM_ERROR, "missing %s avp.\n", name);
		      throw;
		    }
		  else
		    {
		      // Parse error 
		      AAA_LOG(LM_ERROR, "Error in AVP %s.\n", name);
		      cm.release(c);
		      throw;
		    }
		}
	      // Check number of containers
	      if (c->size() < min)
		{
		  AAA_LOG(LM_ERROR, "at lease min %s avp needed.\n", name);
		  st.set(NORMAL, AAA_MISSING_AVP, qavp->avp, codec);
		  c->releaseEntries();
		  cm.release(c);
		  throw st;
		}
	      if (c->size() > max)
		{
		  AAA_LOG(LM_ERROR, "at most max[%d] %s avp allowed.\n", max, name);
		  st.set(NORMAL, AAA_AVP_OCCURS_TOO_MANY_TIMES, qavp->avp, codec);
		  c->releaseEntries();
		  cm.release(c);
		  throw st;
		}
	      acl->add(c);
	    } while (0);
	  }
    }
}

static void
parseAppToRawWithoutDict(AAAMessageBlock *msg, AAAAvpContainerList *acl,
                         AvpHeaderCodec *codec)
  //  throw (AAAErrorStatus)
{
  AAAAvpContainer *c;
  AAAErrorStatus st;
  const char *name;
  AAAAvpContainerList::iterator i;

  for (i = acl->begin(); i != acl->end(); i++)
    {
      c = *i;
      c->ParseType() = PARSE_TYPE_OPTIONAL;
      name = c->getAvpName();

      AAADictionaryEntry* avp;
      // use default dictionary only
      if ((avp = AAAAvpList::instance()->search(name)) == NULL)
	{
	  AAA_LOG(LM_ERROR, "No dictionary entry for %s avp.\n", name);
	  st.set(BUG, MISSING_AVP_DICTIONARY_ENTRY);
	  throw;
	}

      if (c->size() == 0)
	{
	  continue;
	}

      AvpParser ap;
      AvpRawData rawData;
      rawData.msg = msg;
      ap.setRawData(&rawData);
      ap.setAppData(c);
      ap.setDictData(avp);
      ap.setCodec(codec);

      try {
	ap.parseAppToRaw();
      }
      catch (AAAErrorStatus &st)
	{
	  AAA_LOG(LM_ERROR, "Error in AVP %s.\n", name);
	  throw;
	}
    }
}

static void
parseAppToRawWithDict(AAAMessageBlock *msg, AAAAvpContainerList *acl,
   	              AAADictionary *dict, AvpHeaderCodec *codec)// throw (AAAErrorStatus);
{
  AAAQualifiedAVP *qavp;
  AAAAvpContainer *c;
  AAAErrorStatus st;
  unsigned int min, max;
  const char *name;
  int type;
  AAAQualifiedAvpList::iterator i;
  AAAAvpParseType pt;

  AAAQualifiedAvpList *qavp_l[4] = 
    {dict->avp_f, dict->avp_f2, dict->avp_r, dict->avp_o};

  for (int j=0; j<4; j++)
    {
      for (i = qavp_l[j]->begin(); i != qavp_l[j]->end(); i++)
	{
	  pt = qavp_l[j]->getParseType();
	  qavp = *i;
	  min = qavp->qual.min;
	  max = qavp->qual.max;
	  name = qavp->avp->avpName.c_str();
	  type = qavp->avp->avpType;

	  if ((c = acl->search(qavp->avp)) == NULL)
	    {
	      if (min > 0 && max > 0 && pt != PARSE_TYPE_OPTIONAL)
		{
		  AAA_LOG(LM_ERROR, "missing avp %s in container.\n", name);
		  st.set(BUG, MISSING_CONTAINER);
		  throw st;
		}
	      continue;
	    }

	  if (max == 0)
	    {
	      AAA_LOG(LM_ERROR, "%s must not appear in container.\n", name);
	      st.set(BUG, PROHIBITED_CONTAINER);
	      throw st;
	    }
	  if (c->size() < min)
	    {
	      AAA_LOG(LM_ERROR, "less than min entries for the AVP.\n");
	      st.set(BUG, TOO_LESS_AVP_ENTRIES);
	      throw st;
	    }
	  if (c->size() > max)
	    {
	      AAA_LOG(LM_ERROR, "more than max entries for the AVP.\n");
	      st.set(BUG, TOO_MUCH_AVP_ENTRIES);
	      throw st;
	    }
	  if (c->size() == 0)
	    {
	      AAA_LOG(LM_INFO, "container is empty.\n");
	      continue;
	    }

#ifdef DEBUG
	  cout << __FUNCTION__ << ": Container "<< name << "matches.\n";
#endif
	  c->ParseType() = pt;
	  AvpParser ap;
	  AvpRawData rawData;
	  rawData.msg = msg;
	  ap.setRawData(&rawData);
	  ap.setAppData(c);
	  ap.setDictData(qavp->avp);
	  ap.setCodec(codec);

	  try {
	    ap.parseAppToRaw();
	  }
	  catch (AAAErrorStatus &st)
	    {
	      AAA_LOG(LM_ERROR, "Error in AVP %s.\n", name);
	      throw;
	    }
	}
    }
}

