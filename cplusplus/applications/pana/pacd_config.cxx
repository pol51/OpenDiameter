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

#include <iostream>
#include "pacd_config.h"
#include "od_utl_xml_sax_parser.h"

int PACD_Config::Open(std::string &cfgfile)
{
    OD_Utl_XML_SaxParser parser;

    OD_Utl_XML_StringElement setup01(m_Data.m_PaCCfgFile, 
                                     "pana_cfg_file", parser);

    OD_Utl_XML_StringElement setup02(m_Data.m_Username, 
                                     "username", parser);

    OD_Utl_XML_StringElement setup03(m_Data.m_Password, 
                                     "password", parser);

    OD_Utl_XML_StringElement setup04(m_Data.m_Secret, 
                                     "secret", parser);

    OD_Utl_XML_StringElement setup05(m_Data.m_AuthScript, 
                                     "auth_script", parser);

    OD_Utl_XML_UInt32Element setup07(m_Data.m_UseArchie, 
                                     "use_archie", parser);

    OD_Utl_XML_UInt32Element setup08(m_Data.m_AuthPeriod, 
                                     "auth_period", parser);

    OD_Utl_XML_UInt32Element setup09(m_Data.m_ThreadCount, 
                                     "thread_count", parser);

    try {    
        parser.Load((char*)cfgfile.c_str());
        this->print();
    }
    catch (OD_Utl_XML_SaxException &e) {
        e.Print();
        throw;
    }
    catch (...) {
        throw;
    }
    return (0);
}

void PACD_Config::print()
{
   std::cout << "PACD configuration (ver 1.0.0)" 
             << std::endl;
   std::cout << "     PANA config file: " 
             << m_Data.m_PaCCfgFile
             << std::endl;
   std::cout << "     Username        : " 
             << m_Data.m_Username
             << std::endl;
   std::cout << "     Auth script     : " 
             << m_Data.m_AuthScript
             << std::endl;
   std::cout << "     Use Archie      : ";
   std::cout <<  m_Data.m_UseArchie;
   std::cout << std::endl;
   std::cout << "     Auth Period     : ";
   std::cout <<  m_Data.m_AuthPeriod;
   std::cout << std::endl;
   std::cout << "     Thread Count    : ";
   std::cout <<  m_Data.m_ThreadCount;
   std::cout << std::endl;
}

