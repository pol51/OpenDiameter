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
/* $Id: avp_container.cxx,v 1.29 2005/07/21 13:04:06 vfajardo Exp $ */
#include "diameter_parser_api.h"

AAAAvpContainerEntry*
AAAAvpContainerEntryManager::acquire(AAAAvpDataType type)
{
  AAAAvpContainerEntry *e;
  // Search creator;
  DiameterAvpType *avpType =  DiameterAvpTypeList::instance()->search(type);
  if (avpType == NULL)
    {
      DiameterErrorCode st;
      AAA_LOG(LM_ERROR, "Pre-defined type not found", type);
      st.set(BUG, AAA_PARSE_ERROR_INVALID_CONTAINER_PARAM);
      throw st;
    }
  e = avpType->createContainerEntry(type);
  return e;
}

AAAAvpContainer*
AAAAvpContainerManager::acquire(const char *avpName)
{
  AAAAvpContainer *c = new AAAAvpContainer;
  c->setAvpName(avpName);
  return (c);
}

void
AAAAvpContainerManager::release(AAAAvpContainer *c)
{
  delete c;
}
