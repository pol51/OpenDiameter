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

#ifndef __AAAD_USER_DB_H__
#define __AAAD_USER_DB_H__

#include "aaad_config.h"

class AAAD_UserEapArchieMethod
{
   public:
      std::string &SharedSecretFile() {
	 return m_SharedSecretFile;
      }
      void Dump() {
         AAAD_LOG(LM_INFO, 
                  "(%P|%t)  Shared secret: %s\n", 
                  m_SharedSecretFile.data());
      }

   private:
      std::string m_SharedSecretFile;
};

class AAAD_UserEapMd5Method
{
   public:
      typedef enum {
	 SYSTEM,
         FLAT,
         NONE
      } PASSWD_TYPE;
   public:
      std::string &Secret() {
	 return m_Secret;
      }
      PASSWD_TYPE &PasswordType() {
	 return m_PasswdType;
      }
      static PASSWD_TYPE PasswordType(std::string &t) {
         static char *strType[] = {
	   "system", "flat", "none" // same order as enum
         };
         for (unsigned int i = 0; 
              i < sizeof(strType)/sizeof(char*);
              i ++) {
	    if (t == strType[i]) {
	       return PASSWD_TYPE(i);
	    }
	 }
         return NONE;
      }
      void Dump() {
         AAAD_LOG(LM_INFO, 
                  "(%P|%t) MD5 Passwd Typ: %d\n", 
                  (int)m_PasswdType);
      }

   private:
      std::string m_Secret;
      PASSWD_TYPE m_PasswdType;
};

class AAAD_UserEntry :
   public AAAD_MapElement
{
   public:
      std::string &Method() {
	 return m_Method;
      }
      AAAD_UserEapMd5Method &Md5() {
	 return m_Md5;
      }
      AAAD_UserEapArchieMethod &Archie() {
	 return m_Archie;
      }
      void Dump() {
         AAAD_LOG(LM_INFO, 
                  "(%P|%t) *** Match User: %s\n", 
                  m_Name.data());
         AAAD_LOG(LM_INFO, 
                  "(%P|%t)         Method: %s\n", 
                  m_Method.data());
         m_Md5.Dump();
         m_Archie.Dump();
      }

   private:
      std::string m_Method;
      AAAD_UserEapMd5Method m_Md5;
      AAAD_UserEapArchieMethod m_Archie;
};

typedef AAAD_Map AAAD_UserDb;
typedef ACE_Singleton<AAAD_UserDb, 
                      ACE_Recursive_Thread_Mutex> 
                      AAAD_UserDb_S;
#define AAAD_USER_DB()   (*(AAAD_UserDb_S::instance()))

class AAAD_UserEntryParser :
    public OD_Utl_XML_RegisteredElement
              <AAAD_UserDb, 
               OD_Utl_XML_ContentConvNull<AAAD_UserDb> > 
{
  public:
     AAAD_UserEntryParser(AAAD_UserDb &arg, char *name,
                          OD_Utl_XML_SaxParser &parser) :
         OD_Utl_XML_RegisteredElement
              <AAAD_UserDb,
               OD_Utl_XML_ContentConvNull<AAAD_UserDb> > 
                  (arg, name, parser),
                  m_pUser(NULL) {
     }        
     virtual bool startElement(ACEXML_Attributes *atts) {
     	 if (! OD_Utl_XML_Element::startElement(atts)) {
             return false;
     	 }
         ACE_NEW_NORETURN(m_pUser, AAAD_UserEntry);
         if (Name() == std::string("default_entry")) {
             m_pUser->Name() = "default";
         }
         return true;
     }
     virtual bool endElement() {
         m_arg.Register(std::auto_ptr<AAAD_MapElement>(m_pUser));
         m_pUser = NULL;
     	 return OD_Utl_XML_Element::endElement();
     }
     AAAD_UserEntry *Get() {
     	 return m_pUser;
     }
     
  private:
     AAAD_UserEntry *m_pUser;
};
               
class AAAD_NameMatchConv :
   public OD_Utl_XML_ContentConvNull<short>
{
  public:
     AAAD_NameMatchConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<short>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  short &arg) {
         if (m_element->Parent()->Name() == std::string("user_entry")) {
             AAAD_UserEntryParser *userElm = 
                 (AAAD_UserEntryParser*)m_element->Parent();
             userElm->Get()->Name() = ch;               
         }
         else {
             std::cout << "name match has an invalid parent !!!\n";
             throw;
         }
     }
};

class AAAD_EapMethodConv :
   public OD_Utl_XML_ContentConvNull<short>
{
  public:
     AAAD_EapMethodConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<short>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  short &arg) {
         if ((m_element->Parent()->Name() == std::string("user_entry")) ||
             (m_element->Parent()->Name() == std::string("default_entry"))) {
             AAAD_UserEntryParser *userElm = 
                 (AAAD_UserEntryParser*)m_element->Parent();
             userElm->Get()->Method() = ch;               
         }
         else {
             std::cout << "eap method has an invalid parent !!!\n";
             throw;
         }
     }
};

class AAAD_SharedSecretConv :
   public OD_Utl_XML_ContentConvNull<short>
{
  public:
     AAAD_SharedSecretConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<short>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  short &arg) {
         if ((m_element->Parent()->Name() == std::string("user_entry")) ||
             (m_element->Parent()->Name() == std::string("default_entry"))) {
             AAAD_UserEntryParser *userElm = 
                 (AAAD_UserEntryParser*)m_element->Parent();
             userElm->Get()->Archie().SharedSecretFile() = ch;               
         }
         else {
             std::cout << "shared secret has an invalid parent !!!\n";
             throw;
         }
     }
};

class AAAD_PasswordConv :
   public OD_Utl_XML_ContentConvNull<short>
{
  public:
     AAAD_PasswordConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<short>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  short &arg) {
         if ((m_element->Parent()->Name() == std::string("user_entry")) ||
             (m_element->Parent()->Name() == std::string("default_entry"))) {
             AAAD_UserEntryParser *userElm = 
                 (AAAD_UserEntryParser*)m_element->Parent();
             userElm->Get()->Md5().Secret() = ch;               
         }
         else {
             std::cout << "md5 password has an invalid parent !!!\n";
             throw;
         }
     }
};

class AAAD_PasswordTypeConv :
   public OD_Utl_XML_ContentConvNull<short>
{
  public:
     AAAD_PasswordTypeConv(OD_Utl_XML_Element *element) :
         OD_Utl_XML_ContentConvNull<short>(element) {
     }        
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  short &arg) {
         if ((m_element->Parent()->Name() == std::string("user_entry")) ||
             (m_element->Parent()->Name() == std::string("default_entry"))) {
             AAAD_UserEntryParser *userElm = 
                 (AAAD_UserEntryParser*)m_element->Parent();
             if (std::string(ch) == std::string("flat")) {
                 userElm->Get()->Md5().PasswordType() = AAAD_UserEapMd5Method::FLAT;
             }
             else if (std::string(ch) == std::string("system")) {
                 userElm->Get()->Md5().PasswordType() = AAAD_UserEapMd5Method::SYSTEM;
             }
             else if (std::string(ch) == std::string("none")) {
                 userElm->Get()->Md5().PasswordType() = AAAD_UserEapMd5Method::NONE;
             }
         }
         else {
             std::cout << "md5 password type has an invalid parent !!!\n";
             throw;
         }
     }
};

class AAAD_UserDbLoader
{
   public:
      AAAD_UserDbLoader(const char *name) {
         Open(name);
      }
   protected:
      int Open(const char *name) {
   
         AAAD_UserDb &db = AAAD_USER_DB();
         short unused;

         OD_Utl_XML_SaxParser parser;

         AAAD_UserEntryParser users(db, "user_entry", parser);
         AAAD_UserEntryParser default_user(db, "default_entry", parser);

         OD_Utl_XML_RegisteredElement<short,  AAAD_NameMatchConv> 
                                      user01(unused, "name_match", parser);
         OD_Utl_XML_RegisteredElement<short,  AAAD_EapMethodConv> 
                                      user02(unused, "eap_method", parser);
         OD_Utl_XML_RegisteredElement<short,  AAAD_SharedSecretConv> 
                                      user03(unused, "shared_secret_file", parser);
         OD_Utl_XML_RegisteredElement<short,  AAAD_PasswordConv> 
                                      user04(unused, "secret", parser);
         OD_Utl_XML_RegisteredElement<short,  AAAD_PasswordTypeConv> 
                                      user05(unused, "password_type", parser);

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

#endif // __AAAD_USER_DB_H__
