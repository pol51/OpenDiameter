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

#ifndef __PACD_CONFIG_H__
#define __PACD_CONFIG_H__

#include <map>
#include <string>
#include "ace/OS.h"
#include "ace/Singleton.h"

typedef struct {
  std::string m_PaCCfgFile;
  std::string m_Username;
  std::string m_Password;
  std::string m_Secret;
  std::string m_AuthScript;
  ACE_UINT32  m_UseArchie;
  ACE_UINT32  m_AuthPeriod;
  ACE_UINT32  m_ThreadCount;
} PACD_Data;

class PACD_Config {
    public:
        int Open(std::string &cfg_file);
        PACD_Data &Data() {
	   return m_Data;
	}
  
    protected:
        void print();

    private:
        PACD_Data m_Data;
};

typedef ACE_Singleton<PACD_Config, ACE_Null_Mutex> PACD_Config_S;

#define PACD_CONFIG_OPEN(x) PACD_Config_S::instance()->Open((x))
#define PACD_CONFIG()       PACD_Config_S::instance()->Data()

#endif /* __PACD_CONFIG_H__ */
