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
/* $Id: q_avplist.h,v 1.12 2004/06/17 21:07:50 yohba Exp $ */
#ifndef __Q_AVPLIST_H__
#define __Q_AVPLIST_H__

#include <list>
#include "diameter_parser_api.h"
#include "avplist.h"

struct qual
{
  ACE_UINT16 min;
  ACE_UINT16 max;
};

#define QUAL_INFINITY 65535 /* 2^16 -1 */

typedef struct /* AVP with qualifier (min,max) */
{
  AAADictionaryEntry *avp;
  struct qual qual;
} AAAQualifiedAVP;

class AAAQualifiedAvpList: public std::list<AAAQualifiedAVP*>
{
 public:
  AAAQualifiedAvpList(AAAAvpParseType pt) { parseType = pt; };
  ~AAAQualifiedAvpList();
  inline void add(AAAQualifiedAVP* q_avp) { push_back(q_avp); }
  unsigned getMinSize(void);
  inline AAAAvpParseType& getParseType(void) { return parseType; };
 private:
  AAAAvpParseType parseType;
};

struct four_qavp_l {
  AAAQualifiedAvpList *avp_f;  
  AAAQualifiedAvpList *avp_r;  
  AAAQualifiedAvpList *avp_o;  
  AAAQualifiedAvpList *avp_f2;  
};


#endif // __Q_AVPLIST_H__
