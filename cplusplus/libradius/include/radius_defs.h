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

#ifndef __RADIUS_DEFS_H__
#define __RADIUS_DEFS_H__

#include "ace/OS.h"
#include <string>

typedef unsigned char  RADIUS_OCTET;
typedef unsigned short RADIUS_SHORT;

// error codes
typedef enum {
    RADIUS_SUCCESS = 0,
    RADIUS_FAILURE = -100,
    RADIUS_BUF_TO_SMALL = -101,
    RADIUS_MALFORMED_PACKET = -102,
    RADIUS_UNKNOWN_AVP = -103,
    RADIUS_ALLOC_ERROR = -104,
    RADIUS_PARSE_ERROR = -105,
    RADIUS_UNKNOWN_CODE = -106,
    RADIUS_CODE_UNSUPPORTED = -107
} RADIUS_ERR;

class RADIUS_Exception
{
   public:
      RADIUS_Exception(int code, 
                       std::string &description) :
         m_Code(code),
         m_Description(description) {
      }
      RADIUS_Exception(int code, 
                       const char *description) :
         m_Code(code),
         m_Description(description) {
      }
      int &Code() {
	 return m_Code;
      }
      std::string &Description() {
	 return m_Description;
      }

   private:
      int m_Code;
      std::string m_Description;
};

#endif /* __RADIUS_DEFS_H__ */
