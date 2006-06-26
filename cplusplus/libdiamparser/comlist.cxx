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
/* $Id: comlist.cxx,v 1.30 2006/05/31 17:53:33 vfajardo Exp $ */
#include <ace/OS.h>
#include "avplist.h"
#include "comlist.h"
#include "xml_parsing.h"
#include "parser_avpvalue.h"

AAACommandList_S::~AAACommandList_S()
{
  for (iterator i=begin(); i!=end(); i++)
    {
      //      delete (*i)->name;
      delete (*i)->avp_f;
      delete (*i)->avp_r;
      delete (*i)->avp_o;
      delete (*i)->avp_f2;
      delete *i;
    }
}

void
AAACommandList_S::add(AAACommand *com)
{
  if (search(com->name.c_str(), com->protocol) != NULL)
    {
      AAA_LOG(LM_ERROR, "duplicated command definition.\n");
      exit(1);
    }
  
  mutex.acquire();
  push_back(com);
  mutex.release();
#ifdef DEBUG
  cout << "Command name = " << com->name << "\n"; 
#endif
}

AAACommand*
AAACommandList_S::search(const char *name, int protocol)  // search by name 
{
  mutex.acquire();
  for (iterator c=begin(); c!=end(); c++)
    {
      if ((*c)->name == std::string(name))
	{
            if ((protocol >= 0) && ((*c)->protocol != protocol)) {
                continue;
            }
	  mutex.release();
	  return *c;
	}
    }
  mutex.release();
  return NULL;
}


AAACommand*
AAACommandList_S::search(ACE_UINT32 code, ACE_UINT32 appId,
                         int request, int protocol)  
  // search by code and applicationId
{
  mutex.acquire();
  for (iterator c=begin(); c!=end(); c++)
    {
      if ((*c)->code == code && 
	  (*c)->appId == appId &&
	  (*c)->flags.r == request)
	{
            if ((protocol >= 0) && ((*c)->protocol != protocol)) {
                continue;
            }
	  mutex.release();
	  return *c;
	}
    }
  mutex.release();
  return NULL;
}

boolean_t 
AAADictionaryManager::getCommandCode(char *commandName,
				     AAACommandCode *commandCode,
				     AAAApplicationId *appId)
{
  AAACommand *com;
  if ((com = AAACommandList::instance()->search
       (commandName, protocol)) == NULL)
    {
      return false;
    }
  *commandCode = com->code;
  *appId = com->appId;
  return true;
}

void
AAADictionaryManager::init(char *dictFile)
{
  // Parser the XML dictionary.
  parseXMLDictionary(dictFile);
}

AAADictionaryHandle *AAADictionaryManager::getDictHandle
(AAACommandCode code, AAAApplicationId id, int rflag)
{
  return AAACommandList::instance()->search(code, id, rflag, protocol);
}

AAADictionaryHandle *AAADictionaryManager::getDictHandle(char *cmdName)
{
  return AAACommandList::instance()->search(cmdName, protocol);
}
