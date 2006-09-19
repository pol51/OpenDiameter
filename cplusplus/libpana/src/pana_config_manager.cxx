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

#include "ace/Log_Msg.h"
#include "ace/INET_Addr.h"
#include "pana_config_manager.h"
#include "pana_defs.h"
#include "pana_exceptions.h"
#include "od_utl_xml_sax_parser.h"

class PANA_CfgNapInfoParser :
    public OD_Utl_XML_RegisteredElement
              <PANA_CfgProviderInfo, 
               OD_Utl_XML_ContentConvNull<PANA_CfgProviderInfo> > 
{
  public:
     PANA_CfgNapInfoParser(PANA_CfgProviderInfo &arg,
                           OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <PANA_CfgProviderInfo,
               OD_Utl_XML_ContentConvNull<PANA_CfgProviderInfo> > 
                  (arg, "nap_info", parser) {
     }
};
               
class PANA_CfgIspInfoParser :
    public OD_Utl_XML_RegisteredElement
              <PANA_CfgProviderList, 
               OD_Utl_XML_ContentConvNull<PANA_CfgProviderList> > 
{
  public:
     PANA_CfgIspInfoParser(PANA_CfgProviderList &arg,
                           OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <PANA_CfgProviderList,
               OD_Utl_XML_ContentConvNull<PANA_CfgProviderList> > 
                  (arg, "isp_info", parser),
                  m_pInfo(NULL) {
     }        
     virtual bool startElement(ACEXML_Attributes *atts) {
     	 if (! OD_Utl_XML_Element::startElement(atts)) {
     	 	 return false;
     	 }
         ACE_NEW_NORETURN(m_pInfo, PANA_CfgProviderInfo);
	     m_arg.push_back(m_pInfo);
	     return true;
     }
     virtual bool endElement() {
         m_pInfo = NULL;
     	 return OD_Utl_XML_Element::endElement();
     }
     PANA_CfgProviderInfo *Get() {
     	 return m_pInfo;
     }
     
    private:
         PANA_CfgProviderInfo *m_pInfo;
};
               
class PANA_CfgEpIdParser :
    public OD_Utl_XML_RegisteredElement
              <PANA_DeviceIdContainer, 
               OD_Utl_XML_ContentConvNull<PANA_DeviceIdContainer> > 
{
  public:
     PANA_CfgEpIdParser(PANA_DeviceIdContainer &arg,                     
                        OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <PANA_DeviceIdContainer, 
               OD_Utl_XML_ContentConvNull<PANA_DeviceIdContainer> > 
                  (arg, "ep_id", parser) {
     }        
     virtual bool startElement(ACEXML_Attributes *atts) {
     	 if (! OD_Utl_XML_Element::startElement(atts)) {
     	 	 return false;
     	 }
     	 m_EpId.type = AAA_ADDR_FAMILY_IPV4;
     	 m_EpId.value = "";
     	 return true;
     }
     virtual bool endElement() {
	     m_arg.clone(m_EpId);
     	 return OD_Utl_XML_Element::endElement();
     }
     PANA_DeviceId &DeviceId() {
     	 return m_EpId;
     }
     
    private:
         PANA_DeviceId m_EpId;
};
               
class PANA_IdParser :
    public OD_Utl_XML_RegisteredElement
              <std::string, OD_Utl_XML_ContentConvString> 
{
  public:
     PANA_IdParser(OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <std::string, OD_Utl_XML_ContentConvString> 
                  (m_unused, "id", parser) {
     }        
     virtual bool characters(const ACEXML_Char *ch,
                             int start,
                             int length) {
         if (! OD_Utl_XML_Element::characters(ch, start, length)) {
             return false;
         }
         if (Parent()->Name() == std::string("nap_info")) {
             PANA_CfgNapInfoParser *napElm = 
                 (PANA_CfgNapInfoParser*)Parent();
             napElm->Arg().m_Id = ACE_OS::atoi(ch);
         }
         else if (Parent()->Name() == std::string("isp_info")) {
             PANA_CfgIspInfoParser *ispElm = 
                 (PANA_CfgIspInfoParser*)Parent();
             ispElm->Get()->m_Id = ACE_OS::atoi(ch);
         }
         else if (Parent()->Name() == std::string("ep_id")) {
             PANA_CfgEpIdParser *epElm = 
                 (PANA_CfgEpIdParser*)Parent();
             if ((epElm->DeviceId().type != AAA_ADDR_FAMILY_IPV4) &&
                 (epElm->DeviceId().type != AAA_ADDR_FAMILY_IPV6)) {
                 epElm->DeviceId().value.assign(ch, length);
             }
             else {
                 char buf[256];
                 ACE_OS::sprintf(buf, "%s:0", ch);
                 ACE_INET_Addr addr(buf);   
                 PANA_DeviceIdConverter::PopulateFromAddr(addr, epElm->DeviceId());
             }
         }
         else {
             AAA_LOG((LM_ERROR, 
                  "Id has an invalid parent !!!\n"));
             throw;
         }
         return true;
     }
   private:
     std::string m_unused;
};

class PANA_TypeParser :
    public OD_Utl_XML_RegisteredElement
              <ACE_UINT32, OD_Utl_XML_ContentConvUInt32> 
{
  public:
     PANA_TypeParser(OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <ACE_UINT32, OD_Utl_XML_ContentConvUInt32> 
                  (m_unused, "type", parser) {
     }        
     virtual bool characters(const ACEXML_Char *ch,
                             int start,
                             int length) {
         if (! OD_Utl_XML_Element::characters(ch, start, length)) {
             return false;
         }
         if (Parent()->Name() == std::string("ep_id")) {
             PANA_CfgEpIdParser *epElm = 
                 (PANA_CfgEpIdParser*)Parent();
             epElm->DeviceId().type = ACE_OS::atoi(ch);
         }
         else {
             AAA_LOG((LM_ERROR, 
                  "EP type has an invalid parent !!!\n"));
             throw;
         }
         return true;
     }
     
   private:
     ACE_UINT32 m_unused;
};

class PANA_NameParser :
    public OD_Utl_XML_RegisteredElement
              <std::string, OD_Utl_XML_ContentConvString> 
{
  public:
     PANA_NameParser(OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <std::string, OD_Utl_XML_ContentConvString> 
                  (m_unused, "name", parser) {
     }        
     virtual bool characters(const ACEXML_Char *ch,
                             int start,
                             int length) {
         if (! OD_Utl_XML_Element::characters(ch, start, length)) {
             return false;
         }
         if (Parent()->Name() == std::string("nap_info")) {
             PANA_CfgNapInfoParser *napElm = 
                 (PANA_CfgNapInfoParser*)Parent();
             napElm->Arg().m_Name = ch;
         }
         else if (Parent()->Name() == std::string("isp_info")) {
             PANA_CfgIspInfoParser *ispElm = 
                 (PANA_CfgIspInfoParser*)Parent();
             ispElm->Get()->m_Name = ch;
         }
         else {
             AAA_LOG((LM_ERROR, 
                  "Name has an invalid parent !!!\n"));
             throw;
         }
         return true;
     }
     
   private:
     std::string m_unused;
};

PANA_CfgManager::PANA_CfgManager()
{
    m_hasClient = false;
    m_hasServer = false;
}

PANA_CfgManager::~PANA_CfgManager()
{
    // do nothing
}

void PANA_CfgManager::open(std::string &cfg_file)
{
    std::string cfgRoot = "pana_configuration";

    OD_Utl_XML_SaxParser parser;

    // General Section    
    OD_Utl_XML_UInt32Element gen01(m_Data.m_General.m_ListenPort, 
                                   "listen_port", parser);
    OD_Utl_XML_StringElement gen02(m_Data.m_General.m_Interface, 
                                   "interface_name", parser);
    OD_Utl_XML_UInt32Element gen03(m_Data.m_General.m_IPv6Enabled, 
                                   "ipv6", parser);
    OD_Utl_XML_UInt32Element gen04(m_Data.m_General.m_ProtectionCap, 
                                   "protection_capability", parser);
    OD_Utl_XML_StringElement gen05(m_Data.m_General.m_Dictionary, 
                                   "dictionary_filename", parser);
    OD_Utl_XML_UInt32Element gen06(m_Data.m_General.m_MobilityEnabled, 
                                   "mobility", parser);
    OD_Utl_XML_UInt32Element gen09(m_Data.m_General.m_EapPiggyback, 
                                   "eap_piggyback", parser);
    OD_Utl_XML_UInt32Element gen10(m_Data.m_General.m_KeepAliveInterval, 
                                   "keep_alive_interval", parser);
    OD_Utl_XML_UInt32Element gen11(m_Data.m_General.m_WPASupport, 
                                   "wpa_bootstrap", parser);

    // re-transmission section
    OD_Utl_XML_UInt32Element rt01(m_Data.m_General.m_RT.m_IRT, 
                                  "initial_rt_timeout", parser);
    OD_Utl_XML_UInt32Element rt02(m_Data.m_General.m_RT.m_MRC, 
                                  "max_rt_count", parser);
    OD_Utl_XML_UInt32Element rt03(m_Data.m_General.m_RT.m_MRT, 
                                  "max_rt_timeout", parser);
    OD_Utl_XML_UInt32Element rt04(m_Data.m_General.m_RT.m_MRD, 
                                  "max_rt_duration", parser);
                                  
    // PPAC configuration
    ACE_UINT32 ppac01_data = 0, ppac02_data = 0, ppac03_data = 0;
    ACE_UINT32 ppac04_data = 0, ppac05_data = 0;
    OD_Utl_XML_UInt32Element ppac01(ppac01_data, "dhcpv4", parser);
    OD_Utl_XML_UInt32Element ppac02(ppac02_data, "dhcpv6", parser);
    OD_Utl_XML_UInt32Element ppac03(ppac03_data, "autoconfv6", parser);
    OD_Utl_XML_UInt32Element ppac04(ppac04_data, "dhcpv4_ipsec", parser);
    OD_Utl_XML_UInt32Element ppac05(ppac05_data, "ikev2", parser);
    
    // client and server flags    
    ACE_UINT32 flg_data;
    OD_Utl_XML_UInt32Element flg01(flg_data, "client", parser);
    OD_Utl_XML_UInt32Element flg02(flg_data, "auth_agent", parser);

    // PAA config
    OD_Utl_XML_UInt32Element paa01(m_Data.m_PAA.m_UseCookie, 
                                   "use_cookie", parser);
    OD_Utl_XML_UInt32Element paa02(m_Data.m_PAA.m_SessionLifetime, 
                                   "session_lifetime", parser);
    OD_Utl_XML_UInt32Element paa03(m_Data.m_PAA.m_GracePeriod, 
                                   "grace_period", parser);
    OD_Utl_XML_StringElement paa04(m_Data.m_PAA.m_McastAddress, 
                                   "mcast_address", parser);
    OD_Utl_XML_UInt32Element paa05(m_Data.m_PAA.m_CarryDeviceId, 
                                   "carry_device_id", parser);
                                   
    // NAP and ISP info
    PANA_CfgNapInfoParser paa06(m_Data.m_PAA.m_NapInfo, parser);
    PANA_CfgIspInfoParser paa07(m_Data.m_PAA.m_IspInfo, parser);
    
    // EP ids
    PANA_CfgEpIdParser paa08(m_Data.m_PAA.m_EpIdList, parser);
    
    // elements
    PANA_IdParser paa09(parser);
    PANA_TypeParser paa10(parser);
    PANA_NameParser paa11(parser);
    
    // Pac config
    OD_Utl_XML_StringElement pac01(m_Data.m_PaC.m_PaaIpAddress, 
                                   "paa_ip_address", parser);
    OD_Utl_XML_UInt32Element pac02(m_Data.m_PaC.m_PaaPortNumber, 
                                   "paa_port_number", parser);
    OD_Utl_XML_StringElement pac03(m_Data.m_PaC.m_PaaMcastAddress, 
                                   "paa_mcast_address", parser);

    try {    
        parser.Load((char*)cfg_file.c_str());

        // populate real structures        
        m_Data.m_General.m_PPAC.Get().u.m_DHCPv4 = ppac01_data;
        m_Data.m_General.m_PPAC.Get().u.m_DHCPv6 = ppac02_data;
        m_Data.m_General.m_PPAC.Get().u.m_AUTOv6 = ppac03_data;
        m_Data.m_General.m_PPAC.Get().u.m_DHCPv4IPsec = ppac04_data;
        m_Data.m_General.m_PPAC.Get().u.m_IKEv2 = ppac05_data;
        m_Data.m_General.m_PPAC.Get().u.m_NoConfig = 
               (m_Data.m_General.m_PPAC.Get().v > 0) ? false : true;
               
        // flags
        m_hasClient = (flg01.NumInstance() > 0) ? true : false;
        m_hasServer = (flg02.NumInstance() > 0) ? true : false;
         
        if (m_hasClient && m_hasServer) {
            throw (PANA_Exception(PANA_Exception::CONFIG_ERROR, 
                                  "Fatal: Cannot have both client and server entries"));
        }
              
        // for efficiency, just use the 1st entry in PAA isp list
        if (m_hasClient) {
        	    if (! m_Data.m_PAA.m_IspInfo.empty()) {
        	        PANA_CfgProviderInfo *i = m_Data.m_PAA.m_IspInfo.front();
                m_Data.m_PaC.m_IspInfo = (*i);
                delete i;
                m_Data.m_PAA.m_IspInfo.pop_front();
        	    }
        }
        
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

void PANA_CfgManager::close()
{
   // free up allocated provider list
   PANA_CfgProviderInfo *i;
   PANA_CfgProviderList &ispList = m_Data.m_PAA.m_IspInfo;

   while (! ispList.empty()) {
      i = ispList.front();
      if (i) {
         delete i;
      }
      ispList.pop_front();
   }
}

void PANA_CfgManager::dump()
{
    AAA_LOG((LM_INFO, "     General configuration\n"));
    AAA_LOG((LM_INFO, "          Listen Port     : %d\n", m_Data.m_General.m_ListenPort));
    AAA_LOG((LM_INFO, "          Interface name  : %s\n", m_Data.m_General.m_Interface.data()));
    AAA_LOG((LM_INFO, "          IPv6            : %d\n", m_Data.m_General.m_IPv6Enabled));
    AAA_LOG((LM_INFO, "          Protection Cap  : %d\n", m_Data.m_General.m_ProtectionCap));
    AAA_LOG((LM_INFO, "          Dictionary      : %s\n", m_Data.m_General.m_Dictionary.data()));
    AAA_LOG((LM_INFO, "          Mobility        : %d\n", m_Data.m_General.m_MobilityEnabled));
    AAA_LOG((LM_INFO, "          EAP Piggyback   : %d\n", m_Data.m_General.m_EapPiggyback));
    AAA_LOG((LM_INFO, "          EAP Piggyback   : %d\n", m_Data.m_General.m_EapPiggyback));
    AAA_LOG((LM_INFO, "          Re-Transmission\n"));
    AAA_LOG((LM_INFO, "                     IRT  : %d\n", m_Data.m_General.m_RT.m_IRT));
    AAA_LOG((LM_INFO, "                     MRC  : %d\n", m_Data.m_General.m_RT.m_MRC));
    AAA_LOG((LM_INFO, "                     MRT  : %d\n", m_Data.m_General.m_RT.m_MRT));
    AAA_LOG((LM_INFO, "                     MRD  : %d\n", m_Data.m_General.m_RT.m_MRD));
    AAA_LOG((LM_INFO, "   Post-Pana Address Config\n"));
    AAA_LOG((LM_INFO, "                No Config : %d\n", m_Data.m_General.m_PPAC.Get().u.m_NoConfig));
    AAA_LOG((LM_INFO, "                   DHCPv4 : %d\n", m_Data.m_General.m_PPAC.Get().u.m_DHCPv4));
    AAA_LOG((LM_INFO, "                   DHCPv6 : %d\n", m_Data.m_General.m_PPAC.Get().u.m_DHCPv6));
    AAA_LOG((LM_INFO, "             Auto Conf v6 : %d\n", m_Data.m_General.m_PPAC.Get().u.m_AUTOv6));
    AAA_LOG((LM_INFO, "             DHCPv4 IPsec : %d\n", m_Data.m_General.m_PPAC.Get().u.m_DHCPv4IPsec));
    AAA_LOG((LM_INFO, "                    IKEv2 : %d\n", m_Data.m_General.m_PPAC.Get().u.m_IKEv2));
    
    if (m_hasClient) {
        AAA_LOG((LM_INFO, "     Client configuration\n"));
        AAA_LOG((LM_INFO, "          PAA IP Adress   : %s\n", m_Data.m_PaC.m_PaaIpAddress.data()));
        AAA_LOG((LM_INFO, "          PAA Port Number : %d\n", m_Data.m_PaC.m_PaaPortNumber));
        AAA_LOG((LM_INFO, "          PAA Mcast Addr  : %s\n", m_Data.m_PaC.m_PaaMcastAddress.data()));        
        AAA_LOG((LM_INFO, "             ISP Provider Info\n"));
        AAA_LOG((LM_INFO, "                     Name : %s\n", m_Data.m_PaC.m_IspInfo.m_Name.c_str()));
        AAA_LOG((LM_INFO, "                     ID   : %d\n", m_Data.m_PaC.m_IspInfo.m_Id));
    }
    
    if (m_hasServer) {
        AAA_LOG((LM_INFO, "     Auth Agent configuration\n"));
        AAA_LOG((LM_INFO, "          Use Cookie      : %d\n", m_Data.m_PAA.m_UseCookie));
        AAA_LOG((LM_INFO, "          Session-Lifetime: %d\n", m_Data.m_PAA.m_SessionLifetime));
        AAA_LOG((LM_INFO, "          Grace Period    : %d\n", m_Data.m_PAA.m_GracePeriod));
        AAA_LOG((LM_INFO, "          Multicast Addr  : %s\n", m_Data.m_PAA.m_McastAddress.data()));        
        AAA_LOG((LM_INFO, "             NAP Provider Info\n"));
        AAA_LOG((LM_INFO, "                     Name : %s\n", m_Data.m_PAA.m_NapInfo.m_Name.data()));
        AAA_LOG((LM_INFO, "                     ID   : %d\n", m_Data.m_PAA.m_NapInfo.m_Id));
        
        PANA_CfgProviderList::iterator i = m_Data.m_PAA.m_IspInfo.begin();
        for (; i != m_Data.m_PAA.m_IspInfo.end(); i++) {
            AAA_LOG((LM_INFO, "             ISP Provider Info\n"));
            AAA_LOG((LM_INFO, "                     Name : %s\n", (*i)->m_Name.c_str()));
            AAA_LOG((LM_INFO, "                     ID   : %d\n", (*i)->m_Id));
        }
        
        PANA_DeviceIdIterator n = m_Data.m_PAA.m_EpIdList.begin();
        for (; n != m_Data.m_PAA.m_EpIdList.end(); n++) {
            AAA_LOG((LM_INFO, "          EP Device Id    : %d, %s\n",
                      (*n)->type, (*n)->value.c_str()));
        }
        AAA_LOG((LM_INFO, "           Cary Device-Id : %d\n", m_Data.m_PAA.m_CarryDeviceId));        
    }
}
