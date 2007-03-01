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

#ifndef __AAAD_CONFIG_H__
#define __AAAD_CONFIG_H__

#include "od_utl_xml_sax_parser.h"
#include "aaad_defs.h"

typedef struct
{
   std::string name;
   std::string enabled;
} AAAD_AppIdentity;

class AAAD_DiameterEapParser :
    public OD_Utl_XML_RegisteredElement
              <AAAD_ApplicationTable, 
               OD_Utl_XML_ContentConvNull<AAAD_ApplicationTable> > 
{
  public:
     AAAD_DiameterEapParser(AAAD_ApplicationTable &arg,
                            OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <AAAD_ApplicationTable,
               OD_Utl_XML_ContentConvNull<AAAD_ApplicationTable> > 
                  (arg, "diameter_eap", parser),
                  m_pEapApp(NULL) {
     }        
     virtual bool startElement(ACEXML_Attributes *atts) {
     	 if (! OD_Utl_XML_Element::startElement(atts)) {
     	 	 return false;
     	 }
         ACE_NEW_NORETURN(m_pEapApp, AAAD_AppDiamEapData);
         return true;
     }
     virtual bool endElement() {
         m_arg.Register(std::auto_ptr<AAAD_MapElement>(m_pEapApp));
         m_pEapApp = NULL;
     	 return OD_Utl_XML_Element::endElement();
     }
     AAAD_AppDiamEapData *Get() {
     	 return m_pEapApp;
     }
     
    private:
         AAAD_AppDiamEapData *m_pEapApp;
};
               
class AAAD_DiameterEapIdentityConv :
   public OD_Utl_XML_ContentConvNull<AAAD_AppIdentity>
{
  public:
     AAAD_DiameterEapIdentityConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<AAAD_AppIdentity>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  AAAD_AppIdentity &arg) {
         if (m_element->Parent()->Name() == std::string("diameter_eap")) {
             AAAD_DiameterEapParser *eapElm = 
                 (AAAD_DiameterEapParser*)m_element->Parent();
                 eapElm->Get()->Application().LocalIdentity() = ch;               
                 eapElm->Get()->Enabled() = (arg.enabled == std::string("true")) ?
                                            true : false;
                 eapElm->Get()->Name() = arg.name;
         }
         else {
             std::cout << "local identity has an invalid parent !!!\n";
             throw;
         }
     }
};

class AAAD_DiameterEapUserDbConv :
   public OD_Utl_XML_ContentConvNull<AAAD_AppIdentity>
{
  public:
     AAAD_DiameterEapUserDbConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<AAAD_AppIdentity>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  AAAD_AppIdentity &arg) {
         if (m_element->Parent()->Name() == std::string("diameter_eap")) {
             AAAD_DiameterEapParser *eapElm = 
                 (AAAD_DiameterEapParser*)m_element->Parent();
                 eapElm->Get()->Application().UserDbFile() = ch;               
         }
         else {
             std::cout << "user db has an invalid parent !!!\n";
             throw;
         }
     }
};

class AAAD_CfgLoader
{
   public:
      AAAD_CfgLoader(const char *name) {
         Open(name);
      }
   protected:
      int Open(const char *name) {

         AAAD_CfgData &cfg = *(AAAD_CFG_DATA());
         AAAD_AppIdentity ident;

         OD_Utl_XML_SaxParser parser;

         OD_Utl_XML_UInt32Element setup01((unsigned int&)cfg.ThreadCount(), 
                                          "thread_count", parser);

         OD_Utl_XML_StringElement setup02(cfg.DiameterCfgFile(),
                                          "diameter_cfg_file", parser);

         OD_Utl_XML_StringElement appName(ident.name,
                                          "name", parser);
         OD_Utl_XML_StringElement appEnabled(ident.enabled,
                                          "enabled", parser);

         AAAD_DiameterEapParser eap01(cfg.ApplicationTable(), parser);
         OD_Utl_XML_RegisteredElement<AAAD_AppIdentity,  AAAD_DiameterEapIdentityConv> 
                                      eap02(ident, "local_identity", parser);
         OD_Utl_XML_RegisteredElement<AAAD_AppIdentity,  AAAD_DiameterEapUserDbConv> 
                                      eap03(ident, "user_db", parser);

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

#endif // __AAAD_CONFIG_H__
