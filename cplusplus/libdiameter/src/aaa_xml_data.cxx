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

#include "aaa_xml_data.h"
#include "aaa_log_facility.h"
#include "aaa_peer_interface.h"

class DiameterXmlContentConvUInt32List :
  public OD_Utl_XML_ContentConv<std::list<diameter_unsigned32_t> >
{
  public:
     DiameterXmlContentConvUInt32List(OD_Utl_XML_Element *element = 0) :
         OD_Utl_XML_ContentConv<std::list<diameter_unsigned32_t> >(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  std::list<diameter_unsigned32_t> &arg) {
        arg.push_back(ACE_OS::atoi(ch));
     }
};

class DiameterXmlContentConvStringList :
  public OD_Utl_XML_ContentConv<std::list<std::string> >
{
  public:
     DiameterXmlContentConvStringList(OD_Utl_XML_Element *element = 0) :
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
           DiameterXmlContentConvNull;
typedef OD_Utl_XML_RegisteredElement
           <std::list<diameter_unsigned32_t>, DiameterXmlContentConvUInt32List> 
            DiameterXmlUInt32ListElement;
typedef OD_Utl_XML_RegisteredElement
           <std::list<std::string>, DiameterXmlContentConvStringList> 
            DiameterXmlStringListElement;

class DiameterXmlVendorAppIdParser :
    public OD_Utl_XML_RegisteredElement
              <DiameterVendorSpecificIdLst, 
               OD_Utl_XML_ContentConvNull<DiameterVendorSpecificIdLst> > 
{
  public:
     DiameterXmlVendorAppIdParser(DiameterVendorSpecificIdLst &arg,
                              OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <DiameterVendorSpecificIdLst,
               OD_Utl_XML_ContentConvNull<DiameterVendorSpecificIdLst> >
                  (arg, "vendor_specific_application_id", parser) {
     }
     virtual bool startElement(ACEXML_Attributes *atts) {
     	 if (! OD_Utl_XML_Element::startElement(atts)) {
     	 	 return false;
     	 }
         m_Id.authAppId = 0;
         m_Id.acctAppId = 0;
         m_Id.vendorId = 0;
         return true;
     }
     virtual bool endElement() {
         m_arg.push_back(m_Id);
     	 return OD_Utl_XML_Element::endElement();
     }
     DiameterDataVendorSpecificApplicationId &Get() {
     	 return m_Id;
     }

    private:
         DiameterDataVendorSpecificApplicationId m_Id;
};

class DiameterXmlAcctAppIdConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlAcctAppIdConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
         if (m_element->Parent()->Name() == std::string("configuration")) {
              DIAMETER_CFG_GENERAL()->acctAppIdLst.push_back(ACE_OS::atoi(ch));
         }
         else if (m_element->Parent()->Name() == std::string("vendor_specific_application_id")) {
              DiameterXmlVendorAppIdParser *vendorIdElm = 
                 (DiameterXmlVendorAppIdParser*)m_element->Parent();
              if (vendorIdElm->Get().authAppId == 0) {
                 vendorIdElm->Get().acctAppId = ACE_OS::atoi(ch);
              }
              else {
                 AAA_LOG((LM_ERROR,
                      "While adding acct-app-id, Vendor specific application id already has auth-app-id\n"));
                 throw;
              }
         }
         else {
             AAA_LOG((LM_ERROR, 
                  "acct app id has an invalid parent !!!\n"));
             throw;
         }
     }
};

class DiameterXmlAuthAppIdConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlAuthAppIdConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
         if (m_element->Parent()->Name() == std::string("configuration")) {
              DIAMETER_CFG_GENERAL()->authAppIdLst.push_back(ACE_OS::atoi(ch));
         }
         else if (m_element->Parent()->Name() == std::string("vendor_specific_application_id")) {
              DiameterXmlVendorAppIdParser *vendorIdElm = 
                 (DiameterXmlVendorAppIdParser*)m_element->Parent();
              if (vendorIdElm->Get().acctAppId == 0) {
                 vendorIdElm->Get().authAppId = ACE_OS::atoi(ch);
              }
              else {
                 AAA_LOG((LM_ERROR,
                      "While adding auth-app-id, Vendor specific application id already has acct-app-id\n"));
                 throw;
              }
         }
         else {
             AAA_LOG((LM_ERROR, 
                  "auth app id has an invalid parent !!!\n"));
             throw;
         }
     }
};

class DiameterXmlRouteParser :
    public OD_Utl_XML_RegisteredElement
              <diameter_unsigned32_t, 
               OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
{
  public:
     DiameterXmlRouteParser(char *name, OD_Utl_XML_SaxParser &parser) :
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
         ACE_NEW_NORETURN(m_route, DiameterRouteEntry(empty));
         return true;
     }
     virtual bool endElement() {
         if (Name() == std::string("route")) {
             DIAMETER_ROUTE_TABLE()->Add(*m_route);
         }
         else if (Name() == std::string("default_route")) {
             DIAMETER_ROUTE_TABLE()->DefaultRoute(*m_route);
         }
         else {
             AAA_LOG((LM_ERROR, 
                  "Route xml parser has an invalid name !!!\n"));
             delete m_route;
             throw;
         }
         m_route = NULL;
     	 return OD_Utl_XML_Element::endElement();
     }
     DiameterRouteEntry *Get() {
     	 return m_route;
     }

    private:
        DiameterRouteEntry *m_route;
        diameter_unsigned32_t m_unused;
};

class DiameterXmlRealmConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlRealmConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
         if ((m_element->Parent()->Name() == std::string("route")) ||
             (m_element->Parent()->Name() == std::string("default_route"))) {
              DiameterXmlRouteParser *rteElm = 
                  (DiameterXmlRouteParser*)m_element->Parent();
              rteElm->Get()->Realm() = ch;
         }
         else if (m_element->Parent()->Name() == std::string("configuration")) {
              DIAMETER_CFG_TRANSPORT()->realm = ch;
         }
         else {
             AAA_LOG((LM_ERROR, 
                  "realm has an invalid parent !!!\n"));
             throw;
         }
     }
};

class DiameterXmlPeerEntryParser :
    public OD_Utl_XML_RegisteredElement
              <diameter_unsigned32_t, 
               OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
{
  public:
     DiameterXmlPeerEntryParser(AAA_Task &task, OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <diameter_unsigned32_t,
               OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
                  (m_unused, "peer", parser),
               m_task(task) {
     }
     virtual bool endElement() {
         DiameterPeerManager mngr(m_task);
         if (! mngr.Add(m_peerInfo.hostname,
                        m_peerInfo.port,
                        m_peerInfo.use_sctp,
                        m_peerInfo.tls_enabled,
                        0,
                        true)) {   
             AAA_LOG((LM_INFO, "(%P|%t) WARING !!! - Unable to add peer: %s\n", 
                     m_peerInfo.hostname.c_str()));
         }
     	 return OD_Utl_XML_Element::endElement();
     }
     DiameterDataPeer &Get() {
     	 return m_peerInfo;
     }

  private:
     AAA_Task &m_task;
     DiameterDataPeer m_peerInfo;
     diameter_unsigned32_t m_unused;
};

class DiameterXmlHostnameConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlHostnameConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
            DiameterXmlPeerEntryParser *peerElm = 
                 (DiameterXmlPeerEntryParser*)m_element->Parent();
             peerElm->Get().hostname = ch;
     }
};

class DiameterXmlPortConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlPortConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
            DiameterXmlPeerEntryParser *peerElm = 
                 (DiameterXmlPeerEntryParser*)m_element->Parent();
             peerElm->Get().port = ACE_OS::atoi(ch);
     }
};

class DiameterXmlUseSctpConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlUseSctpConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
            DiameterXmlPeerEntryParser *peerElm = 
                 (DiameterXmlPeerEntryParser*)m_element->Parent();
             peerElm->Get().use_sctp = ACE_OS::atoi(ch);
     }
};

class DiameterXmlTlsEnabledConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlTlsEnabledConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
            DiameterXmlPeerEntryParser *peerElm = 
                 (DiameterXmlPeerEntryParser*)m_element->Parent();
             peerElm->Get().tls_enabled = ACE_OS::atoi(ch);
     }
};

class DiameterXmlRoleConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlRoleConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
            DiameterXmlRouteParser *rteElm = 
                 (DiameterXmlRouteParser*)m_element->Parent();
             rteElm->Get()->Action() = DIAMETER_ROUTE_ACTION(ACE_OS::atoi(ch));
     }
};

class DiameterXmlRouteApplicationParser :
    public OD_Utl_XML_RegisteredElement
              <diameter_unsigned32_t, 
               OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
{
  public:
     DiameterXmlRouteApplicationParser(OD_Utl_XML_SaxParser &parser) :
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
         ACE_NEW_NORETURN(m_routeApp, DiameterRouteApplication);
         return true;
     }
     virtual bool endElement() {
         DiameterXmlRouteParser *rteElm = 
                 (DiameterXmlRouteParser*)Parent();
         rteElm->Get()->Add(*m_routeApp);
         m_routeApp = NULL;
     	 return OD_Utl_XML_Element::endElement();
     }
     DiameterRouteApplication *Get() {
     	 return m_routeApp;
     }

    private:
        DiameterRouteApplication *m_routeApp;
        diameter_unsigned32_t m_unused;
};

class DiameterXmlVendorIdConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlVendorIdConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
         if (m_element->Parent()->Name() == std::string("configuration")) {
              DIAMETER_CFG_GENERAL()->vendor = ACE_OS::atoi(ch);
         }
         else if (m_element->Parent()->Name() == std::string("vendor_specific_application_id")) {
              DiameterXmlVendorAppIdParser *vendorIdElm = 
                 (DiameterXmlVendorAppIdParser*)m_element->Parent();
                 vendorIdElm->Get().vendorId = ACE_OS::atoi(ch);
         }
         else if (m_element->Parent()->Name() == std::string("application")) {
            DiameterXmlRouteApplicationParser *rteAppElm = 
                 (DiameterXmlRouteApplicationParser*)m_element->Parent();
             rteAppElm->Get()->VendorId() = ACE_OS::atoi(ch);
         }
         else {
             AAA_LOG((LM_ERROR, 
                  "vendor id has an invalid parent !!!\n"));
             throw;
         }
     }
};

class DiameterXmlRteApplicationIdConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlRteApplicationIdConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
         DiameterXmlRouteApplicationParser *rteAppElm = 
               (DiameterXmlRouteApplicationParser*)m_element->Parent();
         diameter_unsigned32_t appId = ACE_OS::atoi(ch);
         if (DiameterConfigValidation::IsApplicationIdSupported(appId)) {
             rteAppElm->Get()->ApplicationId() = appId;
         }
         else {
             AAA_LOG((LM_ERROR,
                    "Configuration problem: Your advertising an app id in your route table for which your not advertising in your CER\n"));
            throw;
         }
     }
};

class DiameterXmlRouteServerEntryParser :
  public OD_Utl_XML_RegisteredElement
             <diameter_unsigned32_t, 
              OD_Utl_XML_ContentConvNull<diameter_unsigned32_t> > 
{
  public:
     DiameterXmlRouteServerEntryParser(OD_Utl_XML_SaxParser &parser) :
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
         ACE_NEW_NORETURN(m_routeServer, DiameterRouteServerEntry);   
         return true;
     }
     virtual bool endElement() {
         DiameterXmlRouteApplicationParser *rteElm = 
                 (DiameterXmlRouteApplicationParser*)Parent();
         if (rteElm->Get()->Servers().Add(*m_routeServer)) {
             delete m_routeServer;
             throw;
         }
         m_routeServer = NULL;
     	 return OD_Utl_XML_Element::endElement();
     }
     DiameterRouteServerEntry *Get() {
     	 return m_routeServer;
     }

    private:
        DiameterRouteServerEntry *m_routeServer;
        diameter_unsigned32_t m_unused;
};

class DiameterXmlRteServerConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlRteServerConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
        DiameterXmlRouteServerEntryParser *rteServerElm = 
                (DiameterXmlRouteServerEntryParser*)m_element->Parent();
        rteServerElm->Get()->Server() = ch;
     }
};

class DiameterXmlRteServerMetricConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlRteServerMetricConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
        DiameterXmlRouteServerEntryParser *rteServerElm = 
               (DiameterXmlRouteServerEntryParser*)m_element->Parent();
        rteServerElm->Get()->Metric() = ACE_OS::atoi(ch);
     }
};

class DiameterXmlSessionTimeoutConv :
   public DiameterXmlContentConvNull
{
  public:
     DiameterXmlSessionTimeoutConv(OD_Utl_XML_Element *element) :
         DiameterXmlContentConvNull(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  diameter_unsigned32_t &arg) {
         if (m_element->Parent()->Name() == std::string("auth_sessions")) {
             DIAMETER_CFG_AUTH_SESSION()->sessionTm = ACE_OS::atoi(ch);
         }
         else if (m_element->Parent()->Name() == std::string("acct_sessions")) {
             DIAMETER_CFG_ACCT_SESSION()->sessionTm = ACE_OS::atoi(ch);
         }
         else {
             AAA_LOG((LM_ERROR, 
                  "session timeout has an invalid parent !!!\n"));
             throw;
         }
     }
};

void DiameterXMLConfigParser::Load(AAA_Task &task, char *cfgfile)
{
    DiameterDataRoot &root = *DIAMETER_CFG_ROOT();

    OD_Utl_XML_SaxParser parser;
    diameter_unsigned32_t m_unused;

    // Root marker
    OD_Utl_XML_UInt32Element marker01(m_unused, "configuration", parser);

    // General Section    
    OD_Utl_XML_StringElement gen01(root.general.product, 
                                   "product", parser);
    OD_Utl_XML_UInt32Element gen02(root.general.version, 
                                   "version", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlVendorIdConv>
                                  gen03(m_unused, "vendor_id", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlAcctAppIdConv> 
                                  gen04(m_unused, "acct_application_id", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlAuthAppIdConv> 
                                  gen05(m_unused, "auth_application_id", parser);
    DiameterXmlUInt32ListElement gen06(root.general.supportedVendorIdLst,
                                   "supported_vendor_id", parser);
    DiameterXmlVendorAppIdParser gen07(root.general.vendorSpecificId, parser);

    // Parser Section
    OD_Utl_XML_StringElement parser01(root.parser.dictionary, 
                                   "dictionary", parser);

    // Transport Section
    OD_Utl_XML_StringElement trans01(root.transport.identity, 
                                   "identity", parser);
    OD_Utl_XML_UInt32Element trans02(root.transport.tcp_listen_port,
                                   "tcp_listen_port", parser);
    OD_Utl_XML_UInt32Element trans03(root.transport.sctp_listen_port,
                                   "sctp_listen_port", parser);
    OD_Utl_XML_UInt32Element trans04(root.transport.use_ipv6,
                                   "use_ipv6", parser);
    OD_Utl_XML_UInt32Element trans05(root.transport.watchdog_timeout, 
                                   "watchdog_timeout", parser);
    OD_Utl_XML_UInt32Element trans06(root.transport.reconnect_interval, 
                                   "reconnect_interval", parser);
    OD_Utl_XML_UInt32Element trans07(root.transport.reconnect_max,
                                   "reconnect_max", parser);
    OD_Utl_XML_UInt32Element trans08(root.transport.retx_interval, 
                                   "request_retransmission_interval", parser);
    OD_Utl_XML_UInt32Element trans09(root.transport.retx_max_count, 
                                   "max_request_retransmission_count", parser);
    OD_Utl_XML_UInt32Element trans10(root.transport.rx_buffer_size, 
                                   "receive_buffer_size", parser);
    DiameterXmlStringListElement trans11(root.transport.advertised_hostname,
                                   "advertised_hostname", parser);

    // Peer table
    OD_Utl_XML_UInt32Element trans12((ACE_UINT32&)DIAMETER_PEER_TABLE()->ExpirationTime(), 
                                   "expiration_time", parser);
    DiameterXmlPeerEntryParser trans13(task, parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlHostnameConv> 
               trans14(m_unused, "hostname", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlUseSctpConv> 
               trans15(m_unused, "use_sctp", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlPortConv> 
               trans16(m_unused, "port", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlTlsEnabledConv> 
               trans17(m_unused, "tls_enabled", parser);

    // Route table
    ACE_UINT32 transp_01 = 0;
    OD_Utl_XML_UInt32Element trans18(transp_01, "expire_time", parser);
    DiameterXmlRouteParser trans19("route", parser);
    DiameterXmlRouteParser trans20("default_route", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlRoleConv> 
               trans21(m_unused, "role", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlRealmConv> 
               trans22(m_unused, "realm", parser);

    // Application table
    DiameterXmlRouteApplicationParser trans23(parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlRteApplicationIdConv> 
               trans24(m_unused, "application_id", parser);

   // Server entry
   DiameterXmlRouteServerEntryParser trans25(parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlRteServerConv> 
               trans26(m_unused, "server", parser);
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlRteServerMetricConv> 
               trans27(m_unused, "metric", parser);

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
    OD_Utl_XML_RegisteredElement<diameter_unsigned32_t,  DiameterXmlSessionTimeoutConv> 
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
        DIAMETER_ROUTE_TABLE()->ExpireTime(transp_01);

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
            (root.transport.retx_interval > DIAMETER_ROUTER_MIN_RETX_INTERVAL)) {
             root.transport.retx_interval = DIAMETER_ROUTER_MIN_RETX_INTERVAL;
        }
        if (root.transport.retx_max_count &&
            (root.transport.retx_max_count > DIAMETER_ROUTER_MAX_RETX_COUNT)) {
             root.transport.retx_max_count = DIAMETER_ROUTER_MAX_RETX_COUNT;
        }

        // buffer size validation
        if (root.transport.rx_buffer_size < MSG_COLLECTOR_MAX_MSG_LENGTH) {
            root.transport.rx_buffer_size = MSG_COLLECTOR_MAX_MSG_LENGTH;
        }
        else if (root.transport.rx_buffer_size >
            (MSG_COLLECTOR_MAX_MSG_LENGTH * MSG_COLLECTOR_MAX_MSG_BLOCK)) {
            root.transport.rx_buffer_size = MSG_COLLECTOR_MAX_MSG_LENGTH * (MSG_COLLECTOR_MAX_MSG_BLOCK/2);
        }

        if (! root.session.authSessions.stateful) {
            root.session.authSessions.stateful = DIAMETER_SESSION_STATE_MAINTAINED;
        }
        else {
            root.session.authSessions.stateful = DIAMETER_SESSION_NO_STATE_MAINTAINED;
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

void DiameterXMLConfigParser::dump()
{
    DiameterDataRoot &root = *DIAMETER_CFG_ROOT();

    AAA_LOG((LM_INFO, "(%P|%t)             Product : %s\n", 
                  root.general.product.c_str()));
    AAA_LOG((LM_INFO, "(%P|%t)             Version : %d\n", 
                  root.general.version));
    AAA_LOG((LM_INFO, "(%P|%t)           Vendor Id : %d\n", 
                  root.general.vendor));

    DiameterApplicationIdLst *idList[] = { &root.general.supportedVendorIdLst,
                                       &root.general.authAppIdLst,
                                       &root.general.acctAppIdLst };
    char *label[] = { "Supported Vendor",
                      "Auth Application",
                      "Acct Application" };
    for (unsigned int i=0;
         i < sizeof(idList)/sizeof(DiameterApplicationIdLst*);
        i++) {
        DiameterApplicationIdLst::iterator x = idList[i]->begin();
        for (; x != idList[i]->end(); x++) {
           AAA_LOG((LM_INFO, "(%P|%t)    %s : %d\n",
                    label[i], *x));
        }
    }

    DiameterVendorSpecificIdLst::iterator n = 
        root.general.vendorSpecificId.begin();
    for (; n != root.general.vendorSpecificId.end(); n ++) {

        AAA_LOG((LM_INFO, "(%P|%t)  Vendor Specific Id : "));
        if ((*n).vendorId > 0) {
            AAA_LOG((LM_INFO, "(%P|%t)      Vendor=%d, ", (*n)));
        }
        else {
            AAA_LOG((LM_INFO, "(%P|%t)      Vendor=--- "));
        }
        if ((*n).authAppId > 0) {
            AAA_LOG((LM_INFO, " Auth=%d ", (*n).authAppId));
        }
        else if ((*n).acctAppId > 0) {
            AAA_LOG((LM_INFO, " Acct=%d ", (*n).acctAppId));
        }
        AAA_LOG((LM_INFO, "%s\n", (((*n).authAppId == 0) && 
                ((*n).acctAppId == 0)) ? "---" : ""));
    }

    AAA_LOG((LM_INFO, "(%P|%t)          Dictionary : %s\n", 
                  root.parser.dictionary.c_str()));  

    AAA_LOG((LM_INFO, "(%P|%t)            Identity : %s\n", 
                  root.transport.identity.c_str()));
    AAA_LOG((LM_INFO, "(%P|%t)               Realm : %s\n", 
                  root.transport.realm.c_str()));
    AAA_LOG((LM_INFO, "(%P|%t)          TCP Listen : %d\n", 
                  root.transport.tcp_listen_port));
    AAA_LOG((LM_INFO, "(%P|%t)         SCTP Listen : %d\n", 
                  root.transport.sctp_listen_port));
    AAA_LOG((LM_INFO, "(%P|%t)   Watch-Dog timeout : %d\n", 
                  root.transport.watchdog_timeout));
    AAA_LOG((LM_INFO, "(%P|%t)            Use IPv6 : %d\n", 
                  root.transport.use_ipv6));
    AAA_LOG((LM_INFO, "(%P|%t) Re-transmission Int : %d\n", 
                  root.transport.retx_interval));
    AAA_LOG((LM_INFO, "(%P|%t)    Max Re-trans Int : %d\n", 
                  root.transport.retx_max_count));
    AAA_LOG((LM_INFO, "(%P|%t)    Recv Buffer Size : %d\n", 
                  root.transport.rx_buffer_size));

    std::list<std::string>::iterator i = 
         root.transport.advertised_hostname.begin();
    for (; i != root.transport.advertised_hostname.end(); i++) {
         AAA_LOG((LM_INFO, "(%P|%t)      Hostnames Used : %s\n",
                       (*i).c_str()));
    }

    // peer table
    DIAMETER_PEER_TABLE()->Dump();

    // route table
    DIAMETER_ROUTE_TABLE()->Dump();

    AAA_LOG((LM_INFO, "(%P|%t)            Max Sess : %d\n", 
                 root.session.maxSessions));

    AAA_LOG((LM_INFO, "(%P|%t)  Auth Stateful Auth : %s\n", 
                 root.session.authSessions.stateful ? "stateful" : "stateless"));
    AAA_LOG((LM_INFO, "(%P|%t)     Auth Session(T) : %d\n", 
                 root.session.authSessions.sessionTm));
    AAA_LOG((LM_INFO, "(%P|%t)    Auth Lifetime(T) : %d\n", 
                 root.session.authSessions.lifetimeTm));
    AAA_LOG((LM_INFO, "(%P|%t)       Auth Grace(T) : %d\n", 
                 root.session.authSessions.graceTm));
    AAA_LOG((LM_INFO, "(%P|%t)       Auth Abort(T) : %d\n", 
                 root.session.authSessions.abortRetryTm));

    AAA_LOG((LM_INFO, "(%P|%t)     Acct Session(T) : %d\n", 
                 root.session.acctSessions.sessionTm));
    AAA_LOG((LM_INFO, "(%P|%t)    Acct Interim Int : %d\n", 
                 root.session.acctSessions.recIntervalTm));
    AAA_LOG((LM_INFO, "(%P|%t)      Acct Real-Time : %d\n", 
                 root.session.acctSessions.realtime));

    AAA_LOG((LM_INFO, "(%P|%t)           Debug Log : %s\n", 
                 root.log.flags.debug ? "enabled" : "disabled"));
    AAA_LOG((LM_INFO, "(%P|%t)           Trace Log : %s\n", 
                 root.log.flags.trace ? "enabled" : "disabled"));
    AAA_LOG((LM_INFO, "(%P|%t)            Info Log : %s\n", 
                 root.log.flags.info ? "enabled" : "disabled"));

    AAA_LOG((LM_INFO, "(%P|%t)         Console Log : %s\n", 
                 root.log.targets.console ? "enabled" : "disabled"));
    AAA_LOG((LM_INFO, "(%P|%t)          Syslog Log : %s\n", 
                 root.log.targets.syslog  ? "enabled" : "disabled"));
}

bool DiameterConfigValidation::IsApplicationIdSupported(diameter_unsigned32_t appId)
{
    // Validate with acct app Id against what is advertised
    DiameterApplicationIdLst::iterator i;
    for (i = DIAMETER_CFG_GENERAL()->acctAppIdLst.begin();
        i != DIAMETER_CFG_GENERAL()->acctAppIdLst.end();
        i ++) {
        if ((*i) == DIAMETER_RELAY_APPLICATION_ID) {
            return (true);
        }
        else if ((*i) == appId) {
            return (true);
        }
    }

    // Validate with auth app Id against what is advertised
    for (i = DIAMETER_CFG_GENERAL()->authAppIdLst.begin();
        i != DIAMETER_CFG_GENERAL()->authAppIdLst.end();
        i ++) {
        if ((*i) == DIAMETER_RELAY_APPLICATION_ID) {
            return (true);
        }
        else if ((*i) == appId) {
            return (true);
        }
    }

    // Validate with vendor-specific-app-id againts what is advertised
    DiameterVendorSpecificIdLst::iterator y;
    for (y = DIAMETER_CFG_GENERAL()->vendorSpecificId.begin();
        y != DIAMETER_CFG_GENERAL()->vendorSpecificId.end();
        y ++) {
        if (((*y).authAppId == DIAMETER_RELAY_APPLICATION_ID) ||
            ((*y).acctAppId == DIAMETER_RELAY_APPLICATION_ID)) {
            return (true);
        }
        if (((*y).authAppId != 0) && ((*y).authAppId == appId)) {
            return (true);
        }
        else if (((*y).acctAppId != 0) && ((*y).acctAppId == appId)) {
            return (true);
        }
    }

    return (false);
}

