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

#ifndef __PANA_CONFIG_H__
#define __PANA_CONFIG_H__

#include <list>
#include <string>
#include "ace/OS.h"
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "pana_exports.h"
#include "pana_device_id.h"

// PPAC 
class PANA_CfgPPAC {
    private:
        typedef union {
           struct {
              ACE_UINT32 m_NoConfig    : 1;
              ACE_UINT32 m_DHCPv4      : 1;
              ACE_UINT32 m_DHCPv6      : 1;
              ACE_UINT32 m_AUTOv6      : 1;
              ACE_UINT32 m_DHCPv4IPsec : 1;
              ACE_UINT32 m_IKEv2       : 1;
              ACE_UINT32 reserved      : 26;
           } u;
           ACE_UINT32 v;
        } Flags;
    public:
        PANA_CfgPPAC() {
            ACE_OS::memset(&m_Flags, 0, sizeof(m_Flags));
        }
        PANA_CfgPPAC(ACE_UINT32 flg) {
            set(flg);
        }
        ACE_UINT32 operator()() {
            ACE_UINT32 *flg = (ACE_UINT32*)&m_Flags;
            return *flg;
        }
        ACE_UINT32 operator=(PANA_CfgPPAC &p) {
            set(p());
            return (*this)();
        }
        bool common(PANA_CfgPPAC &p) {
            if (p.Get().u.m_NoConfig == m_Flags.u.m_NoConfig) {
                if (m_Flags.u.m_NoConfig) {
                    return true;
                }
                return (p.Get().v & m_Flags.v) ? true : false;
            }
            return false;
        }
        bool common(ACE_UINT32 p) {
            PANA_CfgPPAC f(p);
            return common(f);
        }
        void set(ACE_UINT32 newFlags) {
            m_Flags.v = newFlags;
        }
        bool NoConfig() {
            return (m_Flags.u.m_NoConfig) ? true : false;
        }
        bool DhcpV4() {
            return (m_Flags.u.m_DHCPv4) ? true : false;
        }
        bool DhcpV6() {
            return (m_Flags.u.m_DHCPv6) ? true : false;
        }
        bool AutoConfV6() {
            return (m_Flags.u.m_AUTOv6) ? true : false;
        }
        bool DHCPv4IPsec() {
            return (m_Flags.u.m_DHCPv4IPsec) ? true : false;
        }
        bool IKEv2() {
            return (m_Flags.u.m_IKEv2) ? true : false;
        }
        Flags &Get() {
            return m_Flags;
        }

    private:
        Flags m_Flags;
};

// Retransmission configuration information
typedef struct {
    ACE_UINT32 m_IRT;
    ACE_UINT32 m_MRC;
    ACE_UINT32 m_MRT;
    ACE_UINT32 m_MRD;    
} PANA_CfgRetransmissionParam;

// Provider configuration information
typedef struct {
    ACE_UINT32 m_Id;
    std::string m_Name;
} PANA_CfgProviderInfo;

// Provider list
typedef std::list<PANA_CfgProviderInfo*> PANA_CfgProviderList;

// Client configuration data loaded from XML file
typedef struct {
    std::string m_PaaIpAddress;
    std::string m_PaaMcastAddress;
    ACE_UINT32 m_PaaPortNumber;
    PANA_CfgProviderInfo m_IspInfo;
} PANA_CfgClient;

// PAA configuration data loaded from XML file
typedef struct {
    ACE_UINT32 m_UseCookie;
    ACE_UINT32 m_SessionLifetime;
    ACE_UINT32 m_GracePeriod;
    ACE_UINT32 m_CarryDeviceId;
    std::string m_McastAddress;
    PANA_CfgProviderInfo m_NapInfo;
    PANA_CfgProviderList m_IspInfo;
    PANA_DeviceIdContainer m_EpIdList;
} PANA_CfgAuthAgent;

// General configuration
typedef struct  {
    std::string m_Interface;
    std::string m_Dictionary;
    ACE_UINT32 m_ListenPort;
    ACE_UINT32 m_IPv6Enabled;
    ACE_UINT32 m_ProtectionCap;
    ACE_UINT32 m_EapPiggyback;
    ACE_UINT32 m_MobilityEnabled;
    ACE_UINT32 m_KeepAliveInterval;
    ACE_UINT32 m_WPASupport;
    PANA_CfgPPAC m_PPAC;
    PANA_CfgRetransmissionParam m_RT;
} PANA_CfgGeneral;

// Root configuration
typedef struct {
    PANA_CfgGeneral m_General;
    PANA_CfgClient m_PaC;
    PANA_CfgAuthAgent m_PAA;
} PANA_Cfg;

// Configuration loader
class PANA_EXPORT PANA_CfgManager {
    public:
        PANA_CfgManager();
        virtual ~PANA_CfgManager();

        void open(std::string &cfg_file);
        void close();

        PANA_Cfg &data() { 
            return m_Data; 
        }
        PANA_CfgGeneral &general() { 
            return m_Data.m_General; 
        }
        PANA_CfgClient &client() { 
            return m_Data.m_PaC; 
        }
        PANA_CfgAuthAgent &authAgent() { 
            return m_Data.m_PAA; 
        }
        
        void dump();

    private:
        PANA_Cfg m_Data;
        bool m_hasClient;
        bool m_hasServer;
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
#define PANA_CFG_PAA()        PANA_Cfg_S::instance()->authAgent()
#define PANA_CFG_CLOSE()      PANA_Cfg_S::instance()->close()

#endif /* __PANA_CORE_H__ */
