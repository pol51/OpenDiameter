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
// PANAPacNetworkSupport.h : header file
//

#ifndef __PANA_PAC_NETWORK_SUPPORT__
#define __PANA_PAC_NETWORK_SUPPORT__

#include "stdafx.h"
#include "ace/INET_Addr.h"
#include <list>
#include <string>
#include <wbemidl.h>

class PANA_AdapterProperties
{
public:
    BOOL Get(std::string &if_desc);
    BOOL DhcpEnabled() {
        return m_DhcpEnabled;
    }
    std::string &AdapterName() {
        return m_AdapterName;
    }
    ACE_INET_Addr &CurrentAddress() {
        return m_CurrentAddress;
    }

private:
    BOOL m_DhcpEnabled;
    std::string m_AdapterName;
    ACE_INET_Addr m_CurrentAddress;
};

class PANA_DhcpRnewal
{
public:
    BOOL Get(std::string &name,
             std::string &errMsg,
             BOOL release = FALSE);
};

typedef std::list< std::string > PANA_SSID_LIST;

class PANA_MonitorAccessPoint
{
public:
    PANA_MonitorAccessPoint();
    virtual ~PANA_MonitorAccessPoint();
    virtual HRESULT GetSSId(std::string &ifname,
                            std::string &ssid);
    virtual HRESULT GetMaC(std::string &ifname,
                           std::string &mac);
    virtual HRESULT EnumerateDevices(PANA_SSID_LIST &SsidList);
private:
    IWbemLocator * m_pIWbemLocator;
    IWbemServices * m_pWbemServices;
};

#endif