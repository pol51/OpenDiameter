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

#ifndef __PANA_CONFIG_H__
#define __PANA_CONFIG_H__

#include <list>
#include <string>
#include "ace/OS.h"
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "pana_exports.h"


// Retransmission configuration information
typedef struct {
    ACE_UINT32 m_IRT;
    ACE_UINT32 m_MRC;
    ACE_UINT32 m_MRT;
    ACE_UINT32 m_MRD;
} PANA_CfgRetransmissionParam;

// Client configuration data loaded from XML file
typedef struct {
    std::string m_PaaIpAddress;
    ACE_UINT32 m_PaaPortNumber;
    ACE_UINT32 m_EapResponseTimeout;
    ACE_UINT32 m_EapPiggyback;
} PANA_CfgClient;

// Agent configuration data loaded from XML file
typedef struct {
    ACE_UINT32 m_OptimizedHandshake;
    ACE_UINT32 m_CarryLifetime;
    ACE_UINT32 m_RetryPSR;
} PANA_CfgAgent;

// General configuration
typedef struct  {
    std::string m_Dictionary;
    ACE_UINT32 m_ListenPort;
    ACE_UINT32 m_WPASupport;
    ACE_UINT32 m_SessionLifetime;
    PANA_CfgRetransmissionParam m_RT;
} PANA_CfgGeneral;

// Root configuration
typedef struct {
    PANA_CfgGeneral m_General;
    PANA_CfgClient m_PaC;
    PANA_CfgAgent m_Paa;
} PANA_Cfg;

// Configuration loader
class PANA_EXPORT PANA_CfgManager {
    public:
        void open(std::string &cfg_file);

        PANA_Cfg &data() {
            return m_Data;
        }
        PANA_CfgGeneral &general() {
            return m_Data.m_General;
        }
        PANA_CfgClient &client() {
            return m_Data.m_PaC;
        }
        PANA_CfgAgent &agent() {
            return m_Data.m_Paa;
        }

        void dump();

    private:
        PANA_Cfg m_Data;
};

/*!
 * Singleton declaration of the session database
 */
typedef ACE_Singleton<PANA_CfgManager, ACE_Null_Mutex> PANA_Cfg_S;

/*!
 * Helper macros for singleton access
 */
#define PANA_CFG_OPEN(x)      PANA_Cfg_S::instance()->open((x))
#define PANA_CFG_GENERAL()    PANA_Cfg_S::instance()->general()
#define PANA_CFG_PAC()        PANA_Cfg_S::instance()->client()
#define PANA_CFG_PAA()        PANA_Cfg_S::instance()->agent()

#endif /* __PANA_CORE_H__ */
