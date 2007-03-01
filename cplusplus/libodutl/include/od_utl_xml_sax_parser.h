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

#ifndef __OD_UTL_XML_SAX_PARSER_H__
#define __OD_UTL_XML_SAX_PARSER_H__

#include <iostream>
#include <list>
#include <map>
#include "od_utl_exports.h"
#include "ACEXML/common/FileCharStream.h"
#include "ACEXML/parser/parser/Parser.h"
#include "ACEXML/common/DefaultHandler.h"

class OD_UTL_EXPORT OD_Utl_XML_SaxException
{
   public:
      OD_Utl_XML_SaxException() :
         m_Code(0) {
      }
      OD_Utl_XML_SaxException(char *desc, 
                              ACE_UINT32 code = 0) :
         m_Code(code),
         m_description(desc) {
      }
      OD_Utl_XML_SaxException(std::string &desc, 
                              ACE_UINT32 code = 0) :
         m_Code(code),
         m_description(desc) {
      }
      std::string &Description() {
         return m_description;
      }
      ACE_UINT32 &Code() {
         return m_Code;
      }
      void Print() {
         std::cout << "SAX Parsing exception: " 
                   << m_description 
                   << std::endl;
      }

   protected:
      ACE_UINT32 m_Code;
      std::string m_description;
};

class OD_Utl_XML_Element;
typedef std::list<OD_Utl_XML_Element*> OD_Utl_XML_ElementStack;

class OD_UTL_EXPORT OD_Utl_XML_Element
{
  public:
     OD_Utl_XML_Element(char *name, 
                        OD_Utl_XML_ElementStack &stack) :
        m_inProcess(false), 
        m_name(name),
        m_callStack(stack),
        m_parent(NULL),
        m_numInstance(0) {
     }
     virtual ~OD_Utl_XML_Element() {
     }
     std::string &Name() {
        return m_name;
     }
     int NumInstance() {
     	return m_numInstance;
     }
     virtual bool startElement(ACEXML_Attributes *atts) {
        if (m_inProcess) {
        	    std::string err = "Error: element ";
        	    err += m_name;
        	    err += "already in process";
            throw OD_Utl_XML_SaxException(err);
        }
        m_numInstance ++;
        m_inProcess = true;
        if (! m_callStack.empty()) {
            m_parent = m_callStack.front();
        }
        m_callStack.push_front(this);
        return true;
     }
     virtual bool characters(const ACEXML_Char *ch,
                             int start,
                             int length ACEXML_ENV_ARG_DECL) {
        if (! m_inProcess) {
        	    std::string err = "Error: element ";
        	    err += m_name;
        	    err += "not in process";
            throw OD_Utl_XML_SaxException(err);
        }
        return true;
     }
     virtual bool endElement() {
        if (! m_inProcess) {
        	    std::string err = "Error: element ";
        	    err += m_name;
        	    err += "not in process";
            throw OD_Utl_XML_SaxException(err);
        }
        m_inProcess = false;
        m_callStack.pop_front();
        m_parent = NULL;
        return true;
     }
     OD_Utl_XML_ElementStack &CallStack() {
        return m_callStack;
     }
     OD_Utl_XML_Element *Parent() {
        return m_parent;
     }

  private:
     bool m_inProcess;
     std::string m_name;
     OD_Utl_XML_ElementStack &m_callStack;
     OD_Utl_XML_Element *m_parent;
     int m_numInstance;
};

typedef std::map<std::string, OD_Utl_XML_Element*> OD_Utl_XML_ElementMap;
typedef std::pair<std::string, OD_Utl_XML_Element*> OD_Utl_XML_ElementPair;

class OD_UTL_EXPORT OD_Utl_XML_SaxParser
{
   public:
      OD_Utl_XML_SaxParser(int numPasses = 1) :
          m_numPasses(numPasses),
          m_currentElement(NULL) {
      }
      virtual ~OD_Utl_XML_SaxParser() {
      }
      OD_Utl_XML_ElementStack &callStack() {
      	  return m_callStack;
      }
      OD_Utl_XML_ElementMap &elementMap() {
      	  return m_elementMap;
      }
      virtual void Load(char* xmlFile);

      virtual void characters(const ACEXML_Char *ch,
                              int start,
                              int length ACEXML_ENV_ARG_DECL);
      virtual void startDocument(ACEXML_ENV_SINGLE_ARG_DECL);
      virtual void endDocument(ACEXML_ENV_SINGLE_ARG_DECL);
      virtual void startElement(const ACEXML_Char *namespaceURI,
                                const ACEXML_Char *localName,
                                const ACEXML_Char *qName,
                                ACEXML_Attributes *atts ACEXML_ENV_ARG_DECL);
      virtual void endElement(const ACEXML_Char *namespaceURI,
                              const ACEXML_Char *localName,
                              const ACEXML_Char *qName ACEXML_ENV_ARG_DECL);
    
   private:
      ACE_UINT32 m_numPasses;
      OD_Utl_XML_Element *m_currentElement;
      OD_Utl_XML_ElementMap m_elementMap;
      OD_Utl_XML_ElementStack m_callStack;
};

template<class T, class C>
class OD_UTL_EXPORT OD_Utl_XML_RegisteredElement :
  public OD_Utl_XML_Element
{
  public:
     OD_Utl_XML_RegisteredElement(T &arg,
                                  char *name, 
                                  OD_Utl_XML_SaxParser &parser) :
        OD_Utl_XML_Element(name, parser.callStack()),
        m_arg(arg) {
        parser.elementMap().insert(OD_Utl_XML_ElementPair(this->Name(), this));
     }     
     T &Arg() {
     	return m_arg;
     }
     
  protected:
     virtual bool characters(const ACEXML_Char *ch,
                             int start,
                             int length ACEXML_ENV_ARG_DECL) {
        if (! OD_Utl_XML_Element::characters(ch, start, length)) {
        	    return false;
        }
        C converter(this);
        converter.content(ch, start, length, m_arg);
        return true;
     }
     
  protected:
     T &m_arg;
};

template<class T>
class OD_UTL_EXPORT OD_Utl_XML_ContentConv
{
  public:
     OD_Utl_XML_ContentConv(OD_Utl_XML_Element *element = 0) :
         m_element(element) {
     }
     virtual ~OD_Utl_XML_ContentConv() {
     }
     virtual void content(const ACEXML_Char *ch,
                          int start,
                          int length,
                          T &arg) {
     }
  protected:
     OD_Utl_XML_Element *m_element;
};

template<class T>
class OD_UTL_EXPORT OD_Utl_XML_ContentConvNull :
  public OD_Utl_XML_ContentConv<T>
{
  public:
     OD_Utl_XML_ContentConvNull(OD_Utl_XML_Element *element = 0) :
         OD_Utl_XML_ContentConv<T>(element) {
     }
     virtual void content(const ACEXML_Char *ch,
                          int start,
                          int length,
                          T &arg) {
     }
};

class OD_UTL_EXPORT OD_Utl_XML_ContentConvUInt32 :
  public OD_Utl_XML_ContentConv<ACE_UINT32>
{
  public:
     OD_Utl_XML_ContentConvUInt32(OD_Utl_XML_Element *element = 0) :
         OD_Utl_XML_ContentConv<ACE_UINT32>(element) {
     }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  ACE_UINT32 &arg) {
        arg = ACE_OS::atoi(ch);
     }
};

class OD_UTL_EXPORT OD_Utl_XML_ContentConvString :
  public OD_Utl_XML_ContentConv<std::string>
{
  public:
     OD_Utl_XML_ContentConvString(OD_Utl_XML_Element *element = 0) :
         OD_Utl_XML_ContentConv<std::string>(element) {
    }
     void content(const ACEXML_Char *ch,
                  int start,
                  int length,
                  std::string &arg) {
        arg = ch;
     }
};

typedef OD_Utl_XML_RegisteredElement
           <ACE_UINT32, OD_Utl_XML_ContentConvUInt32> 
               OD_Utl_XML_UInt32Element;
typedef OD_Utl_XML_RegisteredElement
           <std::string, OD_Utl_XML_ContentConvString> 
               OD_Utl_XML_StringElement;

#endif // __OD_UTL_XML_SAX_PARSER_H__
