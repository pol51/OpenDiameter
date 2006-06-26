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

#if defined(WIN32)
#include <memory>
#endif
#include <iostream>

#include "od_utl_xml_parser.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/SAXParseException.hpp>

int OD_Utl_XMLElementParser::populate(DOMNode *n, 
                                      DOMNode **found)
{
   DOMNode *sibling = n;
   while (sibling != NULL) {

      if (sibling->getNodeType() == DOMNode::ELEMENT_NODE) {

         char *c_str = XMLString::transcode(sibling->getNodeName());

         if (XMLString::compareString(c_str, 
                                      m_Name.c_str()) == 0) {

             XMLString::release(&c_str);
          
            if (found) {
	       *found = sibling;
	    }

 	    return svc(sibling);
         }

         XMLString::release(&c_str);
      }
      sibling = sibling->getNextSibling();
   }
   return (-1);
}

char *OD_Utl_XMLElementParser::getTextContent(DOMNode *n)
{
   DOMNode *child = n->getFirstChild();
   if (child && child->getNodeType() == DOMNode::TEXT_NODE) {
    
      const XMLCh *xmlCh = child->getNodeValue();
      if (xmlCh) {
         return XMLString::transcode(xmlCh);
      }
   }
   return (NULL);
}

char *OD_Utl_XMLElementParser::getAttribute(DOMNode *n, 
                                            const char *name)
{
   XMLCh *tmp = XMLString::transcode(name);
   DOMElement *e = (DOMElement*)n;
   const XMLCh *xmlCh = e->getAttribute(tmp);
   XMLString::release(&tmp);
   return XMLString::transcode(xmlCh);
}

int OD_Utl_XMLTreeParser::open(std::string &filename, 
                               OD_Utl_XMLElementParser &root)
{
   int rcode = (-1);

   try {
      XMLPlatformUtils::Initialize();

      std::auto_ptr<XercesDOMParser> parser(new XercesDOMParser);
      std::auto_ptr<OD_Utl_XMLDOMTreeErrorReporter> errReporter
                    (new OD_Utl_XMLDOMTreeErrorReporter);

      parser->setValidationScheme(XercesDOMParser::Val_Always);
      parser->setDoNamespaces(true);
      parser->setDoSchema(true);
      parser->setErrorHandler(errReporter.get());

      try {
         parser->parse(filename.data());

         if (parser->getErrorCount() == 0) {
            DOMNode *doc = parser->getDocument();
            rcode = root.populate(doc->getFirstChild());
         }
      }
      catch (const DOMException& e) {
          ACE_UNUSED_ARG(e);
      }
      catch (const XMLException& e) {
          ACE_UNUSED_ARG(e);
      }
   }
   catch (const XMLException& toCatch) {
      ACE_UNUSED_ARG(toCatch);
   }

   close();
   return (rcode);
}

void OD_Utl_XMLTreeParser::close()
{
   XMLPlatformUtils::Terminate();
}

void OD_Utl_XMLDOMTreeErrorReporter::warning
          (const SAXParseException& toCatch)
{
    DOMStrX strx(toCatch.getMessage());
    std::cout << "Warning at line=" << toCatch.getLineNumber();
    std::cout << ", column=" << toCatch.getColumnNumber();
    std::cout << ", msg=" << strx.localRef() << std::endl;;
}

void OD_Utl_XMLDOMTreeErrorReporter::error
          (const SAXParseException& toCatch)
{
    DOMStrX strx(toCatch.getMessage());
    fSawErrors = true;
    std::cout << "Error at line=" << toCatch.getLineNumber();
    std::cout << ", column=" << toCatch.getColumnNumber();
    std::cout << ", msg=" << strx.localRef() << std::endl;;
}

void OD_Utl_XMLDOMTreeErrorReporter::fatalError
          (const SAXParseException& toCatch)
{
    DOMStrX strx(toCatch.getMessage());
    fSawErrors = true;
    std::cout << "Fatal Error at line=" << toCatch.getLineNumber();
    std::cout << ", column=" << toCatch.getColumnNumber();
    std::cout << ", msg=" << strx.localRef() << std::endl;;
}





