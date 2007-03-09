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

#include "ace/Log_Msg.h"
#include "ace/INET_Addr.h"
#include "pana_config_manager.h"
#include "pana_defs.h"
#include "pana_exceptions.h"
#include "od_utl_xml_sax_parser.h"

void PANA_CfgManager::open(std::string &cfg_file)
{
    std::string cfgRoot = "pana_configuration";

    OD_Utl_XML_SaxParser parser;

    // General Section
    OD_Utl_XML_UInt32Element gen01(m_Data.m_General.m_ListenPort,
                                   "listen_port", parser);
    OD_Utl_XML_StringElement gen02(m_Data.m_General.m_Dictionary,
                                   "dictionary_filename", parser);
    OD_Utl_XML_UInt32Element gen04(m_Data.m_General.m_WPASupport,
                                   "wpa_bootstrap", parser);
    OD_Utl_XML_UInt32Element gen05(m_Data.m_General.m_SessionLifetime,
                                   "session_lifetime", parser);

    // re-transmission section
    OD_Utl_XML_UInt32Element rt01(m_Data.m_General.m_RT.m_IRT,
                                  "initial_rt_timeout", parser);
    OD_Utl_XML_UInt32Element rt02(m_Data.m_General.m_RT.m_MRC,
                                  "max_rt_count", parser);
    OD_Utl_XML_UInt32Element rt03(m_Data.m_General.m_RT.m_MRT,
                                  "max_rt_timeout", parser);
    OD_Utl_XML_UInt32Element rt04(m_Data.m_General.m_RT.m_MRD,
                                  "max_rt_duration", parser);

    // Pac config
    OD_Utl_XML_StringElement pac01(m_Data.m_PaC.m_PaaIpAddress,
                                   "paa_ip_address", parser);
    OD_Utl_XML_UInt32Element pac02(m_Data.m_PaC.m_PaaPortNumber,
                                   "paa_port_number", parser);
    OD_Utl_XML_UInt32Element pac03(m_Data.m_PaC.m_EapResponseTimeout,
                                   "eap_response_timeout", parser);
    OD_Utl_XML_UInt32Element pac04(m_Data.m_PaC.m_EapPiggyback,
                                   "eap_piggyback", parser);

    // Agent config
    OD_Utl_XML_UInt32Element paa01(m_Data.m_Paa.m_OptimizedHandshake,
                                   "optimized_handshake", parser);
    OD_Utl_XML_UInt32Element paa02(m_Data.m_Paa.m_CarryLifetime,
                                   "carry_lifetime", parser);
    OD_Utl_XML_UInt32Element paa03(m_Data.m_Paa.m_RetryPSR,
                                   "retry_psr", parser);

    try {
        parser.Load((char*)cfg_file.c_str());

        this->dump();
    }
    catch (OD_Utl_XML_SaxException &e) {
        e.Print();
        throw (PANA_Exception(PANA_Exception::CONFIG_ERROR, 
                              "Fatal: Unable to parse XML config file"));
    }
    catch (...) {
        throw (PANA_Exception(PANA_Exception::CONFIG_ERROR, 
                              "Fatal: Unable to parse XML config file"));
    }
}

void PANA_CfgManager::dump()
{
    AAA_LOG((LM_INFO, "     General configuration\n"));
    AAA_LOG((LM_INFO, "          Listen Port     : %d\n", m_Data.m_General.m_ListenPort));
    AAA_LOG((LM_INFO, "          Dictionary      : %s\n", m_Data.m_General.m_Dictionary.data()));
    AAA_LOG((LM_INFO, "          Re-Transmission\n"));
    AAA_LOG((LM_INFO, "                     IRT  : %d\n", m_Data.m_General.m_RT.m_IRT));
    AAA_LOG((LM_INFO, "                     MRC  : %d\n", m_Data.m_General.m_RT.m_MRC));
    AAA_LOG((LM_INFO, "                     MRT  : %d\n", m_Data.m_General.m_RT.m_MRT));
    AAA_LOG((LM_INFO, "                     MRD  : %d\n", m_Data.m_General.m_RT.m_MRD));
    AAA_LOG((LM_INFO, "          Session-Lifetime: %d\n", m_Data.m_General.m_SessionLifetime));

    if (m_Data.m_PaC.m_PaaPortNumber > 0) {
        AAA_LOG((LM_INFO, "     Client configuration\n"));
        AAA_LOG((LM_INFO, "           PAA IP Adress  : %s\n", m_Data.m_PaC.m_PaaIpAddress.data()));
        AAA_LOG((LM_INFO, "          PAA Port Number : %d\n", m_Data.m_PaC.m_PaaPortNumber));
        AAA_LOG((LM_INFO, "     EAP Response Timeout : %d\n", m_Data.m_PaC.m_EapResponseTimeout));
        AAA_LOG((LM_INFO, "            EAP Piggyback : %d\n", m_Data.m_PaC.m_EapPiggyback));
    }
    else {
        AAA_LOG((LM_INFO, "        PAA configuration\n"));
        AAA_LOG((LM_INFO, "      Optimized Handshake : %d\n", m_Data.m_Paa.m_OptimizedHandshake));
        AAA_LOG((LM_INFO, "           Carry Lifetime : %d\n", m_Data.m_Paa.m_CarryLifetime));
        AAA_LOG((LM_INFO, "                Retry PSR : %d\n", m_Data.m_Paa.m_RetryPSR));
    }
}
