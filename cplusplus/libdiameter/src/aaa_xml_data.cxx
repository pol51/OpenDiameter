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

#include "aaa_xml_data.h"
#include "aaa_log_facility.h"
#include "aaa_peer_interface.h"

class AAA_XMLContentConvUInt32List :
  public OD_Utl_XML_ContentConv<std::list<diameter_unsigned32_t> >
{
  public:
     AAA_XMLContentConvUInt32List(OD_Utl_XML_Element *element = 0) :
         OD_Utl_XML_ContentConv<std::list<diameter_unsigned32_t> >(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  std::list<diameter_unsigned32_t> &arg) {
        arg.push_back(ACE_OS::atoi(ch));
     }
};

class AAA_XMLContentConvStringList :
  public OD_Utl_XML_ContentConv<std::list<std::string> >
{
  public:
     AAA_XMLContentConvStringList(OD_Utl_XML_Element *element = 0) :
         OD_Utl_XML_ContentConv<std::list<std::string> >(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  std::list<std::string> &arg) {
        arg.push_back(std::string(ch));
     }
};

typedef OD_Utl_XML_ContentConvNull<diameter_unsigned32_t>
           AAA_XMLContentConvNull;
typedef OD_Utl_XML_RegisteredElement
           <std::list<diameter_unsigned32_t>, AAA_XMLContentConvUInt32List> 
            AAA_XMLUInt32ListElement;
typedef OD_Utl_XML_RegisteredElement
           <std::list<std::string>, AAA_XMLContentConvStringList> 
            AAA_XMLStringListElement;

class AAA_XMLVendorAppIdParser :
    public OD_Utl_XML_RegisteredElement
              <AAA_VendorSpecificIdLst, 
               OD_Utl_XML_ContentConvNull<AAA_VendorSpecificIdLst> > 
{
  public:
     AAA_XMLVendorAppIdParser(AAA_VendorSpecificIdLst &arg,
                              OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <AAA_VendorSpecificIdLst,
               OD_Utl_XML_ContentConvNull<AAA_VendorSpecificIdLst> > 
                  (arg, "vendor_specific_application_id", parser) {
     }        
     virtual bool startElement(ACEXML_Attributes *atts) {
     	 if (! OD_Utl_XML_Element::startElement(atts)) {
     	 	 return false;
     	 }
         m_Id.authAppId = 0;
         m_Id.acctAppId = 0;
         while (! m_Id.vendorIdLst.empty()) {
            m_Id.vendorIdLst.pop_front();
         }
         return true;
     }
     virtual bool endElement() {
         m_arg.push_back(m_Id);
     	 return OD_Utl_XML_Element::endElement();
     }
     AAA_DataVendorSpecificApplicationId &Get() {
     	 return m_Id;
     }
     
    private:
         AAA_DataVendorSpecificApplicationId m_Id;
};
               
class AAA_XMLAcctAppIdConv :
   public AAA_XMLContentConvNull
{
  public:
     AAA_XMLAcctAppIdConv(OD_Utl_XML_Element *element) :
         AAA_XMLContentConvNull(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
         if (m_element->Parent()->Name() == std::string("configuration")) {
              AAA_CFG_GENERAL()->acctAppIdLst.push_back(ACE_OS::atoi(ch));
         }
         else if (m_element->Parent()->Name() == std::string("vendor_specific_application_id")) {
              AAA_XMLVendorAppIdParser *vendorIdElm = 
                 (AAA_XMLVendorAppIdParser*)m_element->Parent();
                 vendorIdElm->Get().acctAppId = ACE_OS::atoi(ch);               
         }
         else {
             AAA_LOG(LM_ERROR, 
                  "acct app id has an invalid parent !!!\n");
             throw;
         }
     }
};

class AAA_XMLAuthAppIdConv :
   public AAA_XMLContentConvNull
{
  public:
     AAA_XMLAuthAppIdConv(OD_Utl_XML_Element *element) :
         AAA_XMLContentConvNull(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
         if (m_element->Parent()->Name() == std::string("configuration")) {
              AAA_CFG_GENERAL()->authAppIdLst.push_back(ACE_OS::atoi(ch));
         }
         else if (m_element->Parent()->Name() == std::string("vendor_specific_application_id")) {
              AAA_XMLVendorAppIdParser *vendorIdElm = 
                 (AAA_XMLVendorAppIdParser*)m_element->Parent();
                 vendorIdElm->Get().authAppId = ACE_OS::atoi(ch);
         }
         else {
             AAA_LOG(LM_ERROR, 
                  "auth app id has an invalid parent !!!\n");
             throw;
         }
     }
};

class AAA_XMLRouteParser :
    public OD_Utl_XML_RegisteredElement
              <diameter_unsigned32_t, 
               OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
{
  public:
     AAA_XMLRouteParser(char *name, OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <diameter_unsigned32_t,
               OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
                  (m_unused, name, parser),
                   m_route(NULL) {
     }        
     virtual bool startElement(ACEXML_Attributes *atts) {
     	 if (! OD_Utl_XML_Element::startElement(atts)) {
     	 	 return false;
     	 }
         std::string empty("");
         ACE_NEW_NORETURN(m_route, AAA_RouteEntry(empty));
         return true;
     }
     virtual bool endElement() {
         if (Name() == std::string("route")) {
             AAA_ROUTE_TABLE()->Add(*m_route);
         }
         else if (Name() == std::string("default_route")) {
             AAA_ROUTE_TABLE()->DefaultRoute(*m_route);
         }
         else {
             AAA_LOG(LM_ERROR, 
                  "Route xml parser has an invalid name !!!\n");
             delete m_route;
             throw;
         }
         m_route = NULL;
     	 return OD_Utl_XML_Element::endElement();
     }
     AAA_RouteEntry *Get() {
     	 return m_route;
     }
     
    private:
        AAA_RouteEntry *m_route;
        diameter_unsigned32_t m_unused;
};
               
class AAA_XMLRealmConv :
   public AAA_XMLContentConvNull
{
  public:
     AAA_XMLRealmConv(OD_Utl_XML_Element *element) :
         AAA_XMLContentConvNull(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
         if ((m_element->Parent()->Name() == std::string("route")) ||
             (m_element->Parent()->Name() == std::string("default_route"))) {
              AAA_XMLRouteParser *rteElm = 
                  (AAA_XMLRouteParser*)m_element->Parent();
              rteElm->Get()->Realm() = ch;
         }
         else if (m_element->Parent()->Name() == std::string("configuration")) {
              AAA_CFG_TRANSPORT()->realm = ch;
         }
         else {
             AAA_LOG(LM_ERROR, 
                  "realm has an invalid parent !!!\n");
             throw;
         }
     }
};

class AAA_XMLPeerEntryParser :
    public OD_Utl_XML_RegisteredElement
              <diameter_unsigned32_t, 
               OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
{
  public:
     AAA_XMLPeerEntryParser(AAA_Task &task, OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <diameter_unsigned32_t,
               OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
                  (m_unused, "peer", parser),
               m_task(task) {
     }        
     virtual bool endElement() {
         AAA_PeerManager mngr(m_task);
         if (! mngr.Add(m_peerInfo.hostname,
                        m_peerInfo.port,
                        m_peerInfo.tls_enabled,
                        0,
                        true)) {   
             AAA_LOG(LM_INFO, "(%P|%t) WARING !!! - Unable to add peer: %s\n", 
                     m_peerInfo.hostname.data());
         }
     	 return OD_Utl_XML_Element::endElement();
     }
     AAA_DataPeer &Get() {
     	 return m_peerInfo;
     }
     
  private:
     AAA_Task &m_task;
     AAA_DataPeer m_peerInfo;
     diameter_unsigned32_t m_unused;
};

class AAA_XMLHostnameConv :
   public AAA_XMLContentConvNull
{
  public:
     AAA_XMLHostnameConv(OD_Utl_XML_Element *element) :
         AAA_XMLContentConvNull(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
            AAA_XMLPeerEntryParser *peerElm = 
                 (AAA_XMLPeerEntryParser*)m_element->Parent();
             peerElm->Get().hostname = ch;
     }
};

class AAA_XMLPortConv :
   public AAA_XMLContentConvNull
{
  public:
     AAA_XMLPortConv(OD_Utl_XML_Element *element) :
         AAA_XMLContentConvNull(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
            AAA_XMLPeerEntryParser *peerElm = 
                 (AAA_XMLPeerEntryParser*)m_element->Parent();
             peerElm->Get().port = ACE_OS::atoi(ch);
     }
};

class AAA_XMLTlsEnabledConv :
   public AAA_XMLContentConvNull
{
  public:
     AAA_XMLTlsEnabledConv(OD_Utl_XML_Element *element) :
         AAA_XMLContentConvNull(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
            AAA_XMLPeerEntryParser *peerElm = 
                 (AAA_XMLPeerEntryParser*)m_element->Parent();
             peerElm->Get().tls_enabled = ACE_OS::atoi(ch);
     }
};

class AAA_XMLRoleConv :
   public AAA_XMLContentConvNull
{
  public:
     AAA_XMLRoleConv(OD_Utl_XML_Element *element) :
         AAA_XMLContentConvNull(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
            AAA_XMLRouteParser *rteElm = 
                 (AAA_XMLRouteParser*)m_element->Parent();
             rteElm->Get()->Action() = AAA_ROUTE_ACTION(ACE_OS::atoi(ch));
     }
};

class AAA_XMLRouteApplicationParser :
    public OD_Utl_XML_RegisteredElement
              <diameter_unsigned32_t, 
               OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
{
  public:
     AAA_XMLRouteApplicationParser(OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <diameter_unsigned32_t,
               OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
                  (m_unused, "application", parser),
                   m_routeApp(NULL) {
     }        
     virtual bool startElement(ACEXML_Attributes *atts) {
     	 if (! OD_Utl_XML_Element::startElement(atts)) {
     	 	 return false;
     	 }
         ACE_NEW_NORETURN(m_routeApp, AAA_RouteApplication);   
         return true;
     }
     virtual bool endElement() {
         AAA_XMLRouteParser *rteElm = 
                 (AAA_XMLRouteParser*)Parent();
         rteElm->Get()->Add(*m_routeApp);
         m_routeApp = NULL;
     	 return OD_Utl_XML_Element::endElement();
     }
     AAA_RouteApplication *Get() {
     	 return m_routeApp;
     }
     
    private:
        AAA_RouteApplication *m_routeApp;
        diameter_unsigned32_t m_unused;
};
               
class AAA_XMLVendorIdConv :
   public AAA_XMLContentConvNull
{
  public:
     AAA_XMLVendorIdConv(OD_Utl_XML_Element *element) :
         AAA_XMLContentConvNull(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
         if (m_element->Parent()->Name() == std::string("configuration")) {
              AAA_CFG_GENERAL()->vendor = ACE_OS::atoi(ch);
         }
         else if (m_element->Parent()->Name() == std::string("vendor_specific_application_id")) {
              AAA_XMLVendorAppIdParser *vendorIdElm = 
                 (AAA_XMLVendorAppIdParser*)m_element->Parent();
                 vendorIdElm->Get().vendorIdLst.push_back(ACE_OS::atoi(ch));
         }
         else if (m_element->Parent()->Name() == std::string("application")) {
            AAA_XMLRouteApplicationParser *rteAppElm = 
                 (AAA_XMLRouteApplicationParser*)m_element->Parent();
             rteAppElm->Get()->VendorId() = ACE_OS::atoi(ch);
         }
         else {
             AAA_LOG(LM_ERROR, 
                  "vendor id has an invalid parent !!!\n");
             throw;
         }
     }
};

class AAA_XMLRteApplicationIdConv :
   public AAA_XMLContentConvNull
{
  public:
     AAA_XMLRteApplicationIdConv(OD_Utl_XML_Element *element) :
         AAA_XMLContentConvNull(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
         AAA_XMLRouteApplicationParser *rteAppElm = 
               (AAA_XMLRouteApplicationParser*)m_element->Parent();
         rteAppElm->Get()->ApplicationId() = ACE_OS::atoi(ch);
     }
};

class AAA_XMLRouteServerEntryParser :
  public OD_Utl_XML_RegisteredElement
             <diameter_unsigned32_t, 
              OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
{
  public:
     AAA_XMLRouteServerEntryParser(OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <diameter_unsigned32_t,
               OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
                  (m_unused, "peer_entry", parser),
                   m_routeServer(NULL) {
     }        
     virtual bool startElement(ACEXML_Attributes *atts) {
     	 if (! OD_Utl_XML_Element::startElement(atts)) {
             return false;
     	 }
         ACE_NEW_NORETURN(m_routeServer, AAA_RouteServerEntry);   
         return true;
     }
     virtual bool endElement() {
         AAA_XMLRouteApplicationParser *rteElm = 
                 (AAA_XMLRouteApplicationParser*)Parent();
         if (rteElm->Get()->Servers().Add(*m_routeServer)) {
             delete m_routeServer;
             throw;
         }
         m_routeServer = NULL;
     	 return OD_Utl_XML_Element::endElement();
     }
     AAA_RouteServerEntry *Get() {
     	 return m_routeServer;
     }
     
    private:
        AAA_RouteServerEntry *m_routeServer;
        diameter_unsigned32_t m_unused;
};
               
class AAA_XMLRteServerConv :
   public AAA_XMLContentConvNull
{
  public:
     AAA_XMLRteServerConv(OD_Utl_XML_Element *element) :
         AAA_XMLContentConvNull(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
        AAA_XMLRouteServerEntryParser *rteServerElm = 
                (AAA_XMLRouteServerEntryParser*)m_element->Parent();
        rteServerElm->Get()->Server() = ch;
     }
};

class AAA_XMLRteServerMetricConv :
   public AAA_XMLContentConvNull
{
  public:
     AAA_XMLRteServerMetricConv(OD_Utl_XML_Element *element) :
         AAA_XMLContentConvNull(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
        AAA_XMLRouteServerEntryParser *rteServerElm = 
               (AAA_XMLRouteServerEntryParser*)m_element->Parent();
        rteServerElm->Get()->Metric() = ACE_OS::atoi(ch);
     }
};

class AAA_XMLSessionTimeoutConv :
   public AAA_XMLContentConvNull
{
  public:
     AAA_XMLSessionTimeoutConv(OD_Utl_XML_Element *element) :
         AAA_XMLContentConvNull(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
         if (m_element->Parent()->Name() == std::string("auth_sessions")) {
             AAA_CFG_AUTH_SESSION()->sessionTm = ACE_OS::atoi(ch);
         }
         else if (m_element->Parent()->Name() == std::string("acct_sessions")) {
             AAA_CFG_ACCT_SESSION()->sessionTm = ACE_OS::atoi(ch);
         }
         else {
             AAA_LOG(LM_ERROR, 
                  "session timeout has an invalid parent !!!\n");
             throw;
         }
     }
};

void AAA_XMLConfigParser::Load(AAA_Task &task, char *cfgfile)
{
    AAA_DataRoot &root = *AAA_CFG_ROOT();

    OD_Utl_XML_SaxParser parser;
    diameter_unsigned32_t m_unused;

    // Root marker
    OD_Utl_XML_UInt32Element marker01(m_unused, "configuration", parser);

    // General Section    
    OD_Utl_XML_StringElement gen01(root.general.product, 
                                   "product", parser);
    OD_Utl_XML_UInt32Element gen02(root.general.version, 
                                   "version", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  AAA_XMLVendorIdConv> 
                                  gen03(m_unused, "vendor_id", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  AAA_XMLAcctAppIdConv> 
                                  gen04(m_unused, "acct_application_id", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  AAA_XMLAuthAppIdConv> 
                                  gen05(m_unused, "auth_application_id", parser);
    AAA_XMLUInt32ListElement gen06(root.general.supportedVendorIdLst,
                                   "supported_vendor_id", parser);
    AAA_XMLVendorAppIdParser gen07(root.general.vendorSpecificId, parser);

    // Parser Section
    OD_Utl_XML_StringElement parser01(root.parser.dictionary, 
                                   "dictionary", parser);

    // Transport Section
    OD_Utl_XML_StringElement trans01(root.transport.identity, 
                                   "identity", parser);
    OD_Utl_XML_UInt32Element trans02(root.transport.tcp_port, 
                                   "tcp_port", parser);
    OD_Utl_XML_UInt32Element trans03(root.transport.tls_port, 
                                   "tls_port", parser);
    OD_Utl_XML_UInt32Element trans04(root.transport.watchdog_timeout, 
                                   "watchdog_timeout", parser);
    OD_Utl_XML_UInt32Element trans05(root.transport.retry_interval, 
                                   "retry_interval", parser);
    OD_Utl_XML_UInt32Element trans06(root.transport.retx_interval, 
                                   "request_retransmission_interval", parser);
    OD_Utl_XML_UInt32Element trans07(root.transport.retx_max_count, 
                                   "max_request_retransmission_count", parser);
    AAA_XMLStringListElement trans08(root.transport.advertised_host_ip,
                                   "advertised_host_ip", parser);

    // Peer table
    OD_Utl_XML_UInt32Element trans09((ACE_UINT32&)AAA_PEER_TABLE()->ExpirationTime(), 
                                   "expiration_time", parser);
    AAA_XMLPeerEntryParser trans10(task, parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  AAA_XMLHostnameConv> 
               trans11(m_unused, "hostname", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  AAA_XMLPortConv> 
               trans12(m_unused, "port", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  AAA_XMLTlsEnabledConv> 
               trans13(m_unused, "tls_enabled", parser);
    
    // Route table
    ACE_UINT32 transp_01 = 0;
    OD_Utl_XML_UInt32Element trans14(transp_01, "expire_time", parser);
    AAA_XMLRouteParser trans15("route", parser);
    AAA_XMLRouteParser trans16("default_route", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  AAA_XMLRoleConv> 
               trans17(m_unused, "role", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  AAA_XMLRealmConv> 
               trans18(m_unused, "realm", parser);

    // Application table
    AAA_XMLRouteApplicationParser trans19(parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  AAA_XMLRteApplicationIdConv> 
               trans20(m_unused, "application_id", parser);

   // Server entry
   AAA_XMLRouteServerEntryParser trans21(parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  AAA_XMLRteServerConv> 
               trans22(m_unused, "server", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  AAA_XMLRteServerMetricConv> 
               trans23(m_unused, "metric", parser);

    // Session management
    OD_Utl_XML_UInt32Element sess01(root.session.maxSessions,
                                   "max_sessions", parser);
    OD_Utl_XML_UInt32Element sess02(root.session.authSessions.stateful,
                                   "stateful", parser);
    OD_Utl_XML_UInt32Element sess03(root.session.authSessions.lifetimeTm,
                                   "lifetime_timeout", parser);
    OD_Utl_XML_UInt32Element sess04(root.session.authSessions.graceTm,
                                   "grace_period_timeout", parser);
    OD_Utl_XML_UInt32Element sess05(root.session.authSessions.abortRetryTm,
                                   "abort_retry_timeout", parser);
    OD_Utl_XML_UInt32Element sess06(root.session.acctSessions.recIntervalTm,
                                   "interim_interval", parser);
    OD_Utl_XML_UInt32Element sess07(root.session.acctSessions.realtime,
                                   "realtime", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  AAA_XMLSessionTimeoutConv> 
               sess08(m_unused, "session_timeout", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  
                          OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
               sess09(m_unused, "auth_sessions", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  
                          OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
               sess10(m_unused, "acct_sessions", parser);

    // logs
    std::string lgp01, lgp02, lgp03, lgp04, lgp05;
    OD_Utl_XML_StringElement log01(lgp01, "debug", parser);
    OD_Utl_XML_StringElement log02(lgp02, "trace", parser);
    OD_Utl_XML_StringElement log03(lgp03, "info", parser);
    OD_Utl_XML_StringElement log04(lgp04, "console", parser);
    OD_Utl_XML_StringElement log05(lgp05, "syslog", parser);

    try {    
        parser.Load(cfgfile);

        // post setup for routing
        AAA_ROUTE_TABLE()->ExpireTime(transp_01);

        // post parsing setup for logs
        root.log.flags.reserved = 0;
        root.log.flags.debug = (lgp01 == std::string("enabled")) ? 1 : 0;
        root.log.flags.trace = (lgp02 == std::string("enabled")) ? 1 : 0;
        root.log.flags.info = (lgp03 == std::string("enabled")) ? 1 : 0;

        root.log.targets.reserved = 0;
        root.log.targets.console = (lgp04 == std::string("enabled")) ? 1 : 0;
        root.log.targets.syslog = (lgp05 == std::string("enabled")) ? 1 : 0;

        // post parsing setup for tx retries
        if (root.transport.retx_interval &&
            (root.transport.retx_interval > AAA_ROUTER_MIN_RETX_INTERVAL)) {
             root.transport.retx_interval = AAA_ROUTER_MIN_RETX_INTERVAL;
        }
        if (root.transport.retx_max_count &&
            (root.transport.retx_max_count > AAA_ROUTER_MAX_RETX_COUNT)) {
             root.transport.retx_max_count = AAA_ROUTER_MAX_RETX_COUNT;
        }

        // post validation for advertised host
        std::list<std::string>::iterator i = root.transport.advertised_host_ip.begin();
        for (; i != root.transport.advertised_host_ip.end(); i++) {
              ACE_INET_Addr hostAddr;
              std::string testAddr(*i + ":0");

             if (hostAddr.set(testAddr.data())) {
                  AAA_LOG(LM_INFO, "(%P|%t) WARNING: Invalid Advertised Host IP Addr [%s], will be ignored\n",
                                   (*i).data());
                  root.transport.advertised_host_ip.erase(i);
                  i = root.transport.advertised_host_ip.begin();
             }       
        }

        // post validation for advertised host
        if (! root.session.authSessions.stateful) {
            root.session.authSessions.stateful = AAA_SESSION_STATE_MAINTAINED;
        }
        else {
            root.session.authSessions.stateful = AAA_SESSION_NO_STATE_MAINTAINED;
        }

        this->dump();
    }
    catch (OD_Utl_XML_SaxException &e) {
        e.Print();
        throw;
    }
    catch (...) {
        throw;
    }
}

void AAA_XMLConfigParser::dump()
{
    AAA_DataRoot &root = *AAA_CFG_ROOT();

    AAA_LOG(LM_INFO, "(%P|%t)             Product : %s\n", 
                  root.general.product.data());
    AAA_LOG(LM_INFO, "(%P|%t)             Version : %d\n", 
                  root.general.version);
    AAA_LOG(LM_INFO, "(%P|%t)           Vendor Id : %d\n", 
                  root.general.vendor);

    AAA_ApplicationIdLst *idList[] = { &root.general.supportedVendorIdLst,
                                       &root.general.authAppIdLst,
                                       &root.general.acctAppIdLst };
    char *label[] = { "Supported Vendor",
                      "Auth Application",
                      "Acct Application" };
    for (int i=0;
         i < sizeof(idList)/sizeof(AAA_ApplicationIdLst*);
        i++) {
        AAA_ApplicationIdLst::iterator x = idList[i]->begin();
        for (; x != idList[i]->end(); x++) {
           AAA_LOG(LM_INFO, "(%P|%t)    %s : %d\n",
                    label[i], *x);
        }
    }

    AAA_VendorSpecificIdLst::iterator n = 
        root.general.vendorSpecificId.begin();
    for (; n != root.general.vendorSpecificId.end(); n ++) {
  
        AAA_LOG(LM_INFO, "(%P|%t)  Vendor Specific Id : ");
        if ((*n).authAppId > 0) {
            AAA_LOG(LM_INFO, " Auth=%d ", (*n).authAppId);
        }
        if ((*n).acctAppId > 0) {
            AAA_LOG(LM_INFO, " Acct=%d ", (*n).acctAppId);
        }
        AAA_LOG(LM_INFO, "%s\n", (((*n).authAppId == 0) && 
                ((*n).acctAppId == 0)) ? "---" : "");
        AAA_ApplicationIdLst::iterator i = (*n).vendorIdLst.begin();
        for (; i != (*n).vendorIdLst.end(); i++) {
            AAA_LOG(LM_INFO, "(%P|%t)                        Vendor=%d\n",
                    (*i));
        }
    }

    AAA_LOG(LM_INFO, "(%P|%t)          Dictionary : %s\n", 
                  root.parser.dictionary.data());  

    AAA_LOG(LM_INFO, "(%P|%t)            Identity : %s\n", 
                  root.transport.identity.data());
    AAA_LOG(LM_INFO, "(%P|%t)               Realm : %s\n", 
                  root.transport.realm.data());
    AAA_LOG(LM_INFO, "(%P|%t)          TCP Listen : %d\n", 
                  root.transport.tcp_port);
    AAA_LOG(LM_INFO, "(%P|%t)          TLS Listen : %d\n", 
                  root.transport.tls_port);
    AAA_LOG(LM_INFO, "(%P|%t)   Watch-Dog timeout : %d\n", 
                  root.transport.watchdog_timeout);
    AAA_LOG(LM_INFO, "(%P|%t) Re-transmission Int : %d\n", 
                  root.transport.retx_interval);
    AAA_LOG(LM_INFO, "(%P|%t)    Max Re-trans Int : %d\n", 
                  root.transport.retx_max_count);
    AAA_LOG(LM_INFO, "(%P|%t) Peer Conn Retry Int : %d\n", 
                  root.transport.retry_interval);
                 
    std::list<std::string>::iterator i = 
         root.transport.advertised_host_ip.begin();
    for (; i != root.transport.advertised_host_ip.end(); i++) {
         AAA_LOG(LM_INFO, "(%P|%t)        Host IP Addr : %s\n", 
                       (*i).data());
    }

    // peer table
    AAA_PEER_TABLE()->Dump();

    // route table
    AAA_ROUTE_TABLE()->Dump();

    AAA_LOG(LM_INFO, "(%P|%t)            Max Sess : %d\n", 
                 root.session.maxSessions);

    AAA_LOG(LM_INFO, "(%P|%t)  Auth Stateful Auth : %s\n", 
                 root.session.authSessions.stateful ? "stateful" : "stateless");
    AAA_LOG(LM_INFO, "(%P|%t)     Auth Session(T) : %d\n", 
                 root.session.authSessions.sessionTm);
    AAA_LOG(LM_INFO, "(%P|%t)    Auth Lifetime(T) : %d\n", 
                 root.session.authSessions.lifetimeTm);
    AAA_LOG(LM_INFO, "(%P|%t)       Auth Grace(T) : %d\n", 
                 root.session.authSessions.graceTm);
    AAA_LOG(LM_INFO, "(%P|%t)       Auth Abort(T) : %d\n", 
                 root.session.authSessions.abortRetryTm);

    AAA_LOG(LM_INFO, "(%P|%t)     Acct Session(T) : %d\n", 
                 root.session.acctSessions.sessionTm);
    AAA_LOG(LM_INFO, "(%P|%t)    Acct Interim Int : %d\n", 
                 root.session.acctSessions.recIntervalTm);
    AAA_LOG(LM_INFO, "(%P|%t)      Acct Real-Time : %d\n", 
                 root.session.acctSessions.realtime);

    AAA_LOG(LM_INFO, "(%P|%t)           Debug Log : %s\n", 
                 root.log.flags.debug ? "enabled" : "disabled");
    AAA_LOG(LM_INFO, "(%P|%t)           Trace Log : %s\n", 
                 root.log.flags.trace ? "enabled" : "disabled");
    AAA_LOG(LM_INFO, "(%P|%t)            Info Log : %s\n", 
                 root.log.flags.info ? "enabled" : "disabled");

    AAA_LOG(LM_INFO, "(%P|%t)         Console Log : %s\n", 
                 root.log.targets.console ? "enabled" : "disabled");
    AAA_LOG(LM_INFO, "(%P|%t)          Syslog Log : %s\n", 
                 root.log.targets.syslog  ? "enabled" : "disabled");
}

