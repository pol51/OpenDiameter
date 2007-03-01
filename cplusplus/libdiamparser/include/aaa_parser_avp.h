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
/* $Id: parser_avp.h,v 1.13 2005/06/17 16:50:56 vfajardo Exp $ */
#ifndef __PARSER_AVP_H__
#define __PARSER_AVP_H__

#include "diameter_parser.h"
#include "aaa_avplist.h"
#include "aaa_avp_header.h"
#include "aaa_q_avplist.h"
#include "aaa_comlist.h"

class DiameterAvpRawData 
{
 public:
  union 
  {
    AAAMessageBlock *msg;
    DiameterAvpHeaderList *ahl;
  };
};

typedef AAAParser<DiameterAvpRawData*,
                  DiameterAvpHeader*,
                  AAADictionaryEntry*>
                  DiameterAvpHeaderParser;
typedef AAAParser<DiameterAvpRawData*,
                  AAAAvpContainer*,
                  AAADictionaryEntry*>
                  DiameterAvpParser;

#endif // __PARSER_AVP_H__
