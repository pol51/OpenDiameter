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

#ifndef __NASD_CONFIG_H__
#define __NASD_CONFIG_H__

#include "od_utl_xml_sax_parser.h"
#include "nasd_defs.h"

typedef struct
{
   std::string name;
   std::string enabled;
} NASD_Identity;

template<class ELEMENT>
class NASD_EntryMap :
    public OD_Utl_XML_RegisteredElement
              <NASD_Map, 
               OD_Utl_XML_ContentConvNull<NASD_Map> > 
{
  public:
     NASD_EntryMap(NASD_Map &arg, char *name,
                   NASD_Identity &ident,
                   OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <NASD_Map,
               OD_Utl_XML_ContentConvNull<NASD_Map> > 
                  (arg, name, parser),
                  m_pElm(NULL),
                  m_ident(ident) {
     }        
     virtual bool startElement(ACEXML_Attributes *atts) {
     	 if (! OD_Utl_XML_Element::startElement(atts)) {
     	 	 return false;
     	 }
         ACE_NEW_NORETURN(m_pElm, ELEMENT);
         return true;
     }
     virtual bool endElement() {
         m_pElm->Name() = m_ident.name;
         m_arg.Register(std::auto_ptr<NASD_MapElement>(m_pElm));
         m_pElm = NULL;
     	 return OD_Utl_XML_Element::endElement();
     }
     ELEMENT *Get() {
     	 return (ELEMENT*)m_pElm;
     }
     
  private:
     NASD_MapElement *m_pElm;
     NASD_Identity &m_ident;
};
               
typedef NASD_EntryMap<NASD_ApPanaData> 
                      NASD_XML_ApPanaElm;
typedef NASD_EntryMap<NASD_Ap8021xData> 
                      NASD_XML_Ap8021xElm;
typedef NASD_EntryMap<NASD_AaaLocalEapAuthData> 
                      NASD_XML_AaaLocalEapElm;
typedef NASD_EntryMap<NASD_AaaDiameterEapData> 
                      NASD_XML_AaaDiameterEapElm;
typedef NASD_EntryMap<NASD_PolicyScriptData> 
                      NASD_XML_PolicyScriptElm;
typedef NASD_EntryMap<NASD_RouteEntry> 
                      NASD_XML_RouteEntryElm;

class NASD_CfgFileConv :
   public OD_Utl_XML_ContentConvNull<NASD_Identity>
{
  public:
     NASD_CfgFileConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<NASD_Identity>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  NASD_Identity &arg) {
         if (m_element->Parent()->Name() == std::string("pana")) {
             NASD_XML_ApPanaElm *panaElm = 
                 (NASD_XML_ApPanaElm*)m_element->Parent();
             panaElm->Get()->Protocol().CfgFile() = ch;
             panaElm->Get()->Enabled() = 
                 (arg.enabled == std::string("true")) ? true : false;
         }
         else {
             std::cout << "cfg file has an invalid parent !!!\n";
             throw;
         }
     }
};

class NASD_EpScriptConv :
   public OD_Utl_XML_ContentConvNull<NASD_Identity>
{
  public:
     NASD_EpScriptConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<NASD_Identity>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  NASD_Identity &arg) {
         if (m_element->Parent()->Name() == std::string("pana")) {
             NASD_XML_ApPanaElm *panaElm = 
                 (NASD_XML_ApPanaElm*)m_element->Parent();
             panaElm->Get()->Protocol().EpScript() = ch;
         }
         else {
             std::cout << "ep script has an invalid parent !!!\n";
             throw;
         }
     }
};

class NASD_SharedSecretConv :
   public OD_Utl_XML_ContentConvNull<NASD_Identity>
{
  public:
     NASD_SharedSecretConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<NASD_Identity>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  NASD_Identity &arg) {
         if (m_element->Parent()->Name() == std::string("local_eap_auth")) {
             NASD_XML_AaaLocalEapElm *loalEapElm = 
                 (NASD_XML_AaaLocalEapElm*)m_element->Parent();
             loalEapElm->Get()->Protocol().SharedSecretFile() = ch;
             loalEapElm->Get()->Enabled() = 
                 (arg.enabled == std::string("true")) ? true : false;
         }
         else {
             std::cout << "shared secret file has an invalid parent !!!\n";
             throw;
         }
     }
};

class NASD_IdentityConv :
   public OD_Utl_XML_ContentConvNull<NASD_Identity>
{
  public:
     NASD_IdentityConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<NASD_Identity>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  NASD_Identity &arg) {
         if (m_element->Parent()->Name() == std::string("local_eap_auth")) {
             NASD_XML_AaaLocalEapElm *loalEapElm = 
                 (NASD_XML_AaaLocalEapElm*)m_element->Parent();
             loalEapElm->Get()->Protocol().Identity() = ch;
         }
         else {
             std::cout << "identity has an invalid parent !!!\n";
             throw;
         }
     }
};

class NASD_DiameterEapCfgConv :
   public OD_Utl_XML_ContentConvNull<NASD_Identity>
{
  public:
     NASD_DiameterEapCfgConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<NASD_Identity>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  NASD_Identity &arg) {
         if (m_element->Parent()->Name() == std::string("diameter_eap")) {
             NASD_XML_AaaDiameterEapElm *loalEapElm = 
                 (NASD_XML_AaaDiameterEapElm*)m_element->Parent();
             loalEapElm->Get()->Protocol().DiameterCfgFile() = ch;
         }
         else {
             std::cout << "diameter eap cfg file has an invalid parent !!!\n";
             throw;
         }
     }
};

class NASD_PolicyScriptConv :
   public OD_Utl_XML_ContentConvNull<NASD_Identity>
{
  public:
     NASD_PolicyScriptConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<NASD_Identity>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  NASD_Identity &arg) {
         if (m_element->Parent()->Name() == std::string("policy_entry")) {
             NASD_XML_PolicyScriptElm *policyElm = 
                 (NASD_XML_PolicyScriptElm*)m_element->Parent();
             policyElm->Get()->Policy().ScriptFile() = ch;
         }
         else {
             std::cout << "policy script has an invalid parent !!!\n";
             throw;
         }
     }
};

class NASD_CallNaiConv :
   public OD_Utl_XML_ContentConvNull<NASD_Identity>
{
  public:
     NASD_CallNaiConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<NASD_Identity>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  NASD_Identity &arg) {
         if (m_element->Parent()->Name() == std::string("call_route_entry")) {
             NASD_XML_RouteEntryElm *callRteElm = 
                 (NASD_XML_RouteEntryElm*)m_element->Parent();
             arg.name = ch;
             callRteElm->Get()->Nai() = ch;
         }
         else {
             std::cout << "nai has an invalid parent !!!\n";
             throw;
         }
     }
};

class NASD_CallPolicyConv :
   public OD_Utl_XML_ContentConvNull<NASD_Identity>
{
  public:
     NASD_CallPolicyConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<NASD_Identity>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  NASD_Identity &arg) {
         std::string policy(ch);
         if (m_element->Parent()->Name() == std::string("call_route_entry")) {
             NASD_XML_RouteEntryElm *callRteElm = 
                 (NASD_XML_RouteEntryElm*)m_element->Parent();
             callRteElm->Get()->PolicyList().push_back(policy);
         }
         else if (m_element->Parent()->Name() == std::string("call_route_default")) {
             NASD_XML_RouteEntryElm *callRteElm = 
                 (NASD_XML_RouteEntryElm*)m_element->Parent();
             NASD_RouteMap &rteMap = (NASD_RouteMap&)callRteElm->Arg();
             rteMap.DefaultRoute().PolicyList().push_back(policy);
         }
         else {
             std::cout << "call policy has an invalid parent !!!\n";
             throw;
         }
     }
};

class NASD_CallAaaProtocolConv :
   public OD_Utl_XML_ContentConvNull<NASD_Identity>
{
  public:
     NASD_CallAaaProtocolConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<NASD_Identity>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  NASD_Identity &arg) {
         if (m_element->Parent()->Name() == 
             std::string("call_route_entry")) {
             NASD_XML_RouteEntryElm *callRteElm = 
                 (NASD_XML_RouteEntryElm*)m_element->Parent();
             callRteElm->Get()->AaaProtocol() = ch;
         }
         else if (m_element->Parent()->Name() == 
             std::string("call_route_default")) {
             NASD_XML_RouteEntryElm *callRteElm = 
                 (NASD_XML_RouteEntryElm*)m_element->Parent();
             NASD_RouteMap &rteMap = (NASD_RouteMap&)callRteElm->Arg();
             rteMap.DefaultRoute().AaaProtocol() = ch;
         }
         else {
             std::cout << "call aaa has an invalid parent !!!\n";
             throw;
         }
     }
};

class NASD_CfgLoader
{
   public:
      NASD_CfgLoader(const char *name) {
         Open(name);
      }
   protected:
      int Open(const char *name) {

         NASD_CallManagementData &cfg = *(NASD_CALLMNGT_DATA());
         NASD_Identity ident;
         std::string unused;

         OD_Utl_XML_SaxParser parser;

         OD_Utl_XML_UInt32Element setup01((unsigned int&)cfg.ThreadCount(),
                                          "thread_count", parser);
         OD_Utl_XML_StringElement setup02(ident.name, "name", parser);
         OD_Utl_XML_StringElement setup03(ident.enabled, "enabled", parser);

         NASD_XML_ApPanaElm setup04(NASD_APPROTO_TBL(), 
                                    "pana", ident, parser);
         NASD_XML_Ap8021xElm setup05(NASD_APPROTO_TBL(), 
                                    "eap_8021X", ident, parser);

         NASD_XML_AaaLocalEapElm setup06(NASD_AAAPROTO_TBL(), 
                                    "local_eap_auth", ident, parser);
         NASD_XML_AaaDiameterEapElm setup07(NASD_AAAPROTO_TBL(), 
                                    "diameter_eap", ident, parser);

         NASD_XML_PolicyScriptElm setup08(NASD_POLICY_TBL(), 
                                    "policy_entry", ident, parser);

         NASD_XML_RouteEntryElm setup09(NASD_CALLROUTE_TBL(), 
                                    "call_route_entry", ident, parser);
         NASD_XML_RouteEntryElm setup10(NASD_CALLROUTE_TBL(), 
                                    "call_route_default", ident, parser);

         OD_Utl_XML_RegisteredElement<NASD_Identity, NASD_CfgFileConv> 
                                      setup11(ident, "cfg_file", parser);
         OD_Utl_XML_RegisteredElement<NASD_Identity, NASD_EpScriptConv> 
                                      setup12(ident, "ep_script", parser);

         OD_Utl_XML_RegisteredElement<NASD_Identity, NASD_IdentityConv> 
                                      setup14(ident, "identity", parser);
         OD_Utl_XML_RegisteredElement<NASD_Identity, NASD_DiameterEapCfgConv> 
                                      setup15(ident, "diameter_cfg_file", parser);

         OD_Utl_XML_RegisteredElement<NASD_Identity, NASD_PolicyScriptConv> 
                                      setup16(ident, "file", parser);

         OD_Utl_XML_RegisteredElement<NASD_Identity, NASD_CallNaiConv> 
                                      setup17(ident, "nai", parser);
         OD_Utl_XML_RegisteredElement<NASD_Identity, NASD_CallPolicyConv> 
                                      setup18(ident, "access_policy", parser);
         OD_Utl_XML_RegisteredElement<NASD_Identity, NASD_CallAaaProtocolConv> 
                                      setup19(ident, "aaa_protocol", parser);

         OD_Utl_XML_StringElement header01(unused, "pana", parser);
         OD_Utl_XML_StringElement header02(unused, "diameter_eap", parser);

         try {    
            parser.Load((char*)name);
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
};

#endif // __NASD_CONFIG_H__
