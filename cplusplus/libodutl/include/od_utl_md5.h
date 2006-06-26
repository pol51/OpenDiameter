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

#ifndef __OD_UTL_MD5_H__
#define __OD_UTL_MD5_H__

#include "ace/OS.h"
#include "od_utl_exports.h"

#if !defined(UINT8)
typedef unsigned char UINT8;
#endif

class OD_UTL_EXPORT OD_Utl_Md5
{
    /* RFC 1321 compliant MD5 implementation */
    public:
       OD_Utl_Md5() {
	  Reset();
       } 
       void Update(UINT8 *input, ACE_UINT32 length);
       void Final(UINT8 *digest);
       void Reset();

    private:
       void Process(UINT8 *data);

       ACE_UINT32 m_Total[2];
       ACE_UINT32 m_State[4];
       UINT8  m_Buffer[64];
};

#endif // __OD_UTL_MD5_H__
