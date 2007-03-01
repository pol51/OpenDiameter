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
/* $Id: parser_q_avplist.cxx,v 1.28 2006/03/16 17:01:50 vfajardo Exp $ */
#include <sys/types.h>
#include <list>
#include "aaa_parser_avp.h"
#include "aaa_parser_q_avplist.h"
#include "aaa_q_avplist.h"
#include "resultcodes.h"    

static 
void parseRawToAppWithoutDict(DiameterAvpHeaderList *ahl,
                              AAAAvpContainerList *acl);

static
void parseRawToAppWithDict(DiameterAvpHeaderList *ahl, 
                           AAAAvpContainerList *acl,
			   DiameterDictionary *dict);

static
void parseAppToRawWithoutDict(AAAMessageBlock *msg,
                              AAAAvpContainerList *acl);

static
void parseAppToRawWithDict(AAAMessageBlock *msg, 
                           AAAAvpContainerList *acl,
			   DiameterDictionary *dict);

template<> void
QualifiedAvpListParser::parseRawToApp()// throw (DiameterErrorCode)
{
  DiameterAvpHeaderList ahl;
  // read buffer and create avp header list 
  try {
    ahl.create(getRawData());
  }
  catch (DiameterErrorCode &st)
    {
      throw st;
    }

  if (getDictData() == NULL)
    {
      ::parseRawToAppWithoutDict(&ahl, getAppData());
    }
  else
    {
      ::parseRawToAppWithDict(&ahl, getAppData(), getDictData());
    }
}

template<> void
QualifiedAvpListParser::parseAppToRaw()// throw (DiameterErrorCode)
{

  if (getDictData() == NULL)
    {
      ::parseAppToRawWithoutDict(getRawData(), getAppData());
    }
  else
    {
      ::parseAppToRawWithDict(getRawData(), getAppData(), getDictData());
    }
}

static void
parseRawToAppWithoutDict(DiameterAvpHeaderList *ahl, 
                         AAAAvpContainerList *acl)
  //  throw (DiameterErrorCode)
{
  AAAAvpContainer *c;
  DiameterErrorCode st;
  const char *name;
  AAAAvpContainerList::iterator i;

  for (i = acl->begin(); i != acl->end(); i++)
    {
      c = *i;
      c->ParseType() = AAA_PARSE_TYPE_OPTIONAL;
      name = c->getAvpName();
#ifdef DEBUG
      cout << __FUNCTION__ << ": Container "<< name << "matches\n";
#endif

      AAADictionaryEntry* avp;
      // use default dictionary only
      if ((avp = DiameterAvpList::instance()->search(name)) == NULL)
	{
	  AAA_LOG((LM_ERROR, "No dictionary entry for %s avp.\n", name));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY);
	  throw;
	}

      do 
	{
	  DiameterAvpParser ap;
	  DiameterAvpRawData rawData;
	  rawData.ahl = ahl;
	  ap.setRawData(&rawData);
	  ap.setAppData(c);
	  ap.setDictData(avp);
	  try {
	    ap.parseRawToApp();
	  }
	  catch (DiameterErrorCode &st)
	    {
	      AAA_PARSE_ERROR_TYPE type;
              int code;
	      st.get(type, code);
	      if (type == AAA_PARSE_ERROR_TYPE_NORMAL && code == AAA_MISSING_AVP)
		{
		  continue;
		}
	      else
		{
		  // Parse error 
		  AAA_LOG((LM_ERROR, "Error in AVP %s.\n", name));
		  throw;
		}
	    }
	} while (0);
    }
}

static void
parseRawToAppWithDict(DiameterAvpHeaderList *ahl, 
                      AAAAvpContainerList *acl,
		      DiameterDictionary *dict)
{
  AAAQualifiedAVP *qavp;
  AAAAvpContainer *c;
  DiameterErrorCode st;
  DiameterAvpContainerManager cm;
  unsigned int min, max;
  const char *name;
  int type;
  DiameterQualifiedAvpList::iterator i;
  AAAAvpParseType pt;

  DiameterQualifiedAvpList *qavp_l[3] =
    {dict->avp_f, dict->avp_r, dict->avp_o};

  for (int j=0; j<3; j++)
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
	      DiameterAvpParser ap;
	      DiameterAvpRawData rawData;
	      rawData.ahl = ahl;
	      ap.setRawData(&rawData);
	      ap.setAppData(c);
	      ap.setDictData(qavp->avp);
	      try {
		ap.parseRawToApp();
	      }
	      catch (DiameterErrorCode &st)
		{
                  AAA_PARSE_ERROR_TYPE type;
                  int code;
		  st.get(type, code);
		  if (type == AAA_PARSE_ERROR_TYPE_NORMAL && code == AAA_MISSING_AVP)
		    {
		      // AVP was not found
		      c->releaseEntries();
		      cm.release(c);

		      if (pt == AAA_PARSE_TYPE_OPTIONAL) 
			continue;

                      if (0 == min)
                        continue;

		      AAA_LOG((LM_ERROR, "missing %s avp.\n", name));
		      throw st;
		    }
		  else
		    {
		      // Parse error 
		      AAA_LOG((LM_ERROR, "Error in AVP %s.\n", name));
		      cm.release(c);
		      throw st;
		    }
		}
	        // Check number of containers
                if (c->size() < min)
		{
		  AAA_LOG((LM_ERROR, "at lease min %s avp needed.\n", name));
		  st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_MISSING_AVP, qavp->avp);
		  c->releaseEntries();
		  cm.release(c);
		  throw st;
		}
	        if (c->size() > max)
		{
		  AAA_LOG((LM_ERROR, "at most max[%d] %s avp allowed.\n", max, name));
		  st.set(AAA_PARSE_ERROR_TYPE_NORMAL, AAA_AVP_OCCURS_TOO_MANY_TIMES, qavp->avp);
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
parseAppToRawWithoutDict(AAAMessageBlock *msg, AAAAvpContainerList *acl)
{
  AAAAvpContainer *c;
  DiameterErrorCode st;
  const char *name;
  AAAAvpContainerList::iterator i;

  for (i = acl->begin(); i != acl->end(); i++)
    {
      c = *i;
      c->ParseType() = AAA_PARSE_TYPE_OPTIONAL;
      name = c->getAvpName();

      AAADictionaryEntry* avp;
      // use default dictionary only
      if ((avp = DiameterAvpList::instance()->search(name)) == NULL)
	{
	  AAA_LOG((LM_ERROR, "No dictionary entry for %s avp.\n", name));
	  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                 AAA_PARSE_ERROR_MISSING_AVP_DICTIONARY_ENTRY);
	  throw;
	}

      if (c->size() == 0)
	{
	  continue;
	}

      DiameterAvpParser ap;
      DiameterAvpRawData rawData;
      rawData.msg = msg;
      ap.setRawData(&rawData);
      ap.setAppData(c);
      ap.setDictData(avp);

      try {
	ap.parseAppToRaw();
      }
      catch (DiameterErrorCode &st)
	{
	  AAA_LOG((LM_ERROR, "Error in AVP %s.\n", name));
	  throw;
	}
    }
}

static void
parseAppToRawWithDict(AAAMessageBlock *msg, 
                      AAAAvpContainerList *acl,
   	              DiameterDictionary *dict)
{
  AAAQualifiedAVP *qavp;
  AAAAvpContainer *c;
  DiameterErrorCode st;
  unsigned int min, max;
  const char *name;
  int type;
  DiameterQualifiedAvpList::iterator i;
  AAAAvpParseType pt;

  DiameterQualifiedAvpList *qavp_l[3] =
    {dict->avp_f, dict->avp_r, dict->avp_o};

  for (int j=0; j<3; j++)
    {
      for (i = qavp_l[j]->begin(); i != qavp_l[j]->end(); i++)
	{
	  pt = qavp_l[j]->getParseType();
	  qavp = *i;
	  min = qavp->qual.min;
	  max = qavp->qual.max;
	  name = qavp->avp->avpName.c_str();
	  type = qavp->avp->avpType;

	  if ((c = acl->search(qavp->avp->avpName.c_str())) == NULL)
	    {
	      if (min > 0 && max > 0 && pt != AAA_PARSE_TYPE_OPTIONAL)
		{
		  AAA_LOG((LM_ERROR, "missing avp %s in container.\n", name));
		  st.set(AAA_PARSE_ERROR_TYPE_BUG,
                         AAA_PARSE_ERROR_MISSING_CONTAINER);
		  throw st;
		}
	      continue;
	    }

	  if (max == 0)
	    {
	      AAA_LOG((LM_ERROR, "%s must not appear in container.\n", name));
	      st.set(AAA_PARSE_ERROR_TYPE_BUG,
                     AAA_PARSE_ERROR_PROHIBITED_CONTAINER);
	      throw st;
	    }
	  if (c->size() < min)
	    {
	      AAA_LOG((LM_ERROR, "less than min entries for the AVP.\n"));
              st.set(AAA_PARSE_ERROR_TYPE_BUG,
                     AAA_PARSE_ERROR_TOO_LESS_AVP_ENTRIES);
	      throw st;
	    }
	  if (c->size() > max)
	    {
	      AAA_LOG((LM_ERROR, "more than max entries for the AVP.\n"));
	      st.set(AAA_PARSE_ERROR_TYPE_BUG,
                     AAA_PARSE_ERROR_TOO_MUCH_AVP_ENTRIES);
	      throw st;
	    }
	  if (c->size() == 0)
	    {
	      AAA_LOG((LM_INFO, "container is empty.\n"));
	      continue;
	    }

#ifdef DEBUG
	  cout << __FUNCTION__ << ": Container "<< name << "matches.\n";
#endif
	  c->ParseType() = pt;
	  DiameterAvpParser ap;
	  DiameterAvpRawData rawData;
	  rawData.msg = msg;
	  ap.setRawData(&rawData);
	  ap.setAppData(c);
	  ap.setDictData(qavp->avp);

	  try {
	    ap.parseAppToRaw();
	  }
	  catch (DiameterErrorCode &st)
	    {
	      AAA_LOG((LM_ERROR, "Error in AVP %s.\n", name));
	      throw;
	    }
	}
    }
}

