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
/* $Id: comlist.h,v 1.15 2004/06/17 21:07:49 yohba Exp $ */
#ifndef __COMLIST_H__
#define __COMLIST_H__

#include "diameter_parser_api.h"
#include "q_avplist.h"

class AAADictionary : public AAADictionaryHandle
{
 public:
  AAACommandCode code;
  AAAApplicationId appId;
  AAAVendorId vendorId;
  AAAQualifiedAvpList* avp_f;  /* fixed */
  AAAQualifiedAvpList* avp_r;  /* required */
  AAAQualifiedAvpList* avp_o;  /* optional */
  AAAQualifiedAvpList* avp_f2; /* fixed */
};

struct comflags
{
  AAA_UINT8 r:1;
  AAA_UINT8 p:1;
  AAA_UINT8 e:1;
};

class AAACommand : public AAADictionary
{
 public:
  std::string name;
  struct comflags flags;
  int protocol;  
};

class AAACommandList_S : public std::list<AAACommand*>
{
  friend class ACE_Singleton<AAACommandList_S, ACE_Recursive_Thread_Mutex>;
 public:
  void add(AAACommand*);
  AAACommand* search(const char*name, int protocol = -1);
  AAACommand* search(ACE_UINT32 code, ACE_UINT32 appId,
                     int request, int protocol = -1);
 private:
  AAACommandList_S() {}
  ~AAACommandList_S();
  ACE_Thread_Mutex mutex;
};

typedef ACE_Singleton<AAACommandList_S, ACE_Recursive_Thread_Mutex> AAACommandList;

#endif // __COMLIST_H__
