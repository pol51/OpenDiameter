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
#include "od_utl_xml_sax_parser.h"
#include "framework.h"
#include "user_db.h"

class AAA_XMLUserEntry :
    public OD_Utl_XML_RegisteredElement
              <AAA_UserEntryDb, 
               OD_Utl_XML_ContentConvNull<AAA_UserEntryDb> > 
{
  public:
     AAA_XMLUserEntry(AAA_UserEntryDb &arg,
                      OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <AAA_UserEntryDb,
               OD_Utl_XML_ContentConvNull<AAA_UserEntryDb> > 
                  (arg, "user", parser),
                  m_pEntry(NULL) {
     }        
     virtual bool startElement(ACEXML_Attributes *atts) {
     	 if (! OD_Utl_XML_Element::startElement(atts)) {
     	 	 return false;
     	 }
         ACE_NEW_NORETURN(m_pEntry, AAA_UserEntry);
         return true;
     }
     virtual bool endElement() {
         m_arg.insert(std::pair<std::string, AAA_UserEntry*>
                      (m_pEntry->m_Username, m_pEntry));
         m_pEntry = NULL;
     	 return OD_Utl_XML_Element::endElement();
     }
     AAA_UserEntry *Get() {
     	 return m_pEntry;
     }
     
    private:
         AAA_UserEntry *m_pEntry;
};
               
class AAA_XMLNameParser :
    public OD_Utl_XML_RegisteredElement
              <std::string, OD_Utl_XML_ContentConvString> 
{
  public:
     AAA_XMLNameParser(OD_Utl_XML_SaxParser &parser) :
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
         if (Parent()->Name() == std::string("user")) {
             AAA_XMLUserEntry *entryElm = 
                 (AAA_XMLUserEntry*)Parent();
             entryElm->Get()->m_Username = ch;
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

class AAA_XMLPassParser :
    public OD_Utl_XML_RegisteredElement
              <std::string, OD_Utl_XML_ContentConvString> 
{
  public:
     AAA_XMLPassParser(OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <std::string, OD_Utl_XML_ContentConvString> 
                  (m_unused, "pass", parser) {
     }        
     virtual bool characters(const ACEXML_Char *ch,
                             int start,
                             int length) {
         if (! OD_Utl_XML_Element::characters(ch, start, length)) {
             return false;
         }
         if (Parent()->Name() == std::string("user")) {
             AAA_XMLUserEntry *entryElm = 
                 (AAA_XMLUserEntry*)Parent();
             entryElm->Get()->m_Passphrase = ch;
         }
         else {
             AAA_LOG((LM_ERROR, 
                  "Auth has an invalid parent !!!\n"));
             throw;
         }
         return true;
     }
   private:
     std::string m_unused;
};

class AAA_XMLAuthParser :
    public OD_Utl_XML_RegisteredElement
              <std::string, OD_Utl_XML_ContentConvString> 
{
  public:
     AAA_XMLAuthParser(OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <std::string, OD_Utl_XML_ContentConvString> 
                  (m_unused, "auth", parser) {
     }        
     virtual bool characters(const ACEXML_Char *ch,
                             int start,
                             int length) {
         if (! OD_Utl_XML_Element::characters(ch, start, length)) {
             return false;
         }
         if (Parent()->Name() == std::string("user")) {
             AAA_XMLUserEntry *entryElm = 
                 (AAA_XMLUserEntry*)Parent();
             std::string Type(ch);
             if (Type == std::string("md5")) {
                 entryElm->Get()->m_AuthType = 4;
             }
             else if (Type == std::string("archie")) {
                 entryElm->Get()->m_AuthType = 100;
             }
             else {
                 AAA_LOG((LM_ERROR, 
                      "Invalid type !!!\n"));
                 throw;
             }
         }
         else {
             AAA_LOG((LM_ERROR, 
                  "Pass has an invalid parent !!!\n"));
             throw;
         }
         return true;
     }
     
   private:
     std::string m_unused;
};

int AAA_UserDb::open(std::string &cfgFile)
{	
    OD_Utl_XML_SaxParser parser;
            
    AAA_XMLUserEntry user(m_UserDb, parser);
    AAA_XMLNameParser name(parser);
    AAA_XMLPassParser pass(parser);
    AAA_XMLAuthParser auth(parser);
        
    try {    
        parser.Load((char*)cfgFile.c_str());
    }
    catch (OD_Utl_XML_SaxException &e) {
        e.Print();
        return (0);
    }
    catch (...) {
        return (0);
    }
    return (1);
}

AAA_UserEntry *AAA_UserDb::lookup(const std::string &uname)
{
    AAA_UserEntryDbIterator i = m_UserDb.find(uname);
    return (i == m_UserDb.end()) ? NULL : i->second;
}

void AAA_UserDb::close()
{
    AAA_UserEntryDbIterator i;
    while (! m_UserDb.empty()) {
        i = m_UserDb.begin();
        delete i->second;
        m_UserDb.erase(i);
    }
}
