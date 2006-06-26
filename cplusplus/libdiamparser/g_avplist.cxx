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
/* $Id: g_avplist.cxx,v 1.12 2004/06/17 21:07:49 yohba Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include "avplist.h"
#include "g_avplist.h"

AAAGroupedAvpList_S::~AAAGroupedAvpList_S()
{
  for (iterator i=begin(); i!=end(); i++)
    {
      delete (*i)->avp_f;
      delete (*i)->avp_r;
      delete (*i)->avp_o;
      delete (*i)->avp_f2;
      delete *i;
    }
}

void
AAAGroupedAvpList_S::add(AAAGroupedAVP *gavp)
{
  if (search(gavp->code, gavp->vendorId) != NULL)
    {
      AAA_LOG(LM_ERROR, "duplicated grouped AVP.\n");
      exit(1);
    }
  mutex.acquire();
  push_back(gavp);
  mutex.release();
}

AAAGroupedAVP*
AAAGroupedAvpList_S::search(ACE_UINT32 code, ACE_UINT32 vendorId)
{
  mutex.acquire();
  for (iterator c=begin(); c!=end(); c++)
    {
      if ((*c)->code == code && (*c)->vendorId == vendorId)
	{
	  mutex.release();
	  return *c;
	}
    }
  mutex.release();
  return NULL;
}


