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
/* $Id: avp_header.h,v 1.15 2005/06/17 16:50:55 vfajardo Exp $ */
#ifndef __AVP_HEADER_H__
#define __AVP_HEADER_H__

#include <list>
#include <string>
#include "parser.h"
#include "diameter_parser_api.h"

#define adjust_word_boundary(len)  ((ACE_UINT32)((((len-1)>>2)+1)<<2))

class AAAAvpHeaderList : public std::list<AAAAvpHeader>
{
  public:
    AAAAvpHeaderList(AvpHeaderCodec *c = 0) : codec(c) {
    }
    void create(AAAMessageBlock *aBuffer) throw(AAAErrorStatus);
  private:
    AvpHeaderCodec *codec;
};

#endif // __AVP_HEADER_H__


