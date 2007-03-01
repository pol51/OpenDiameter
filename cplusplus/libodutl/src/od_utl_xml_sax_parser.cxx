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

#include <string>
#include "framework.h"
#include "od_utl_xml_sax_parser.h"

class OD_UTL_XML_SAXHandler : 
  public ACEXML_DefaultHandler
{
  public:
     OD_UTL_XML_SAXHandler(const ACEXML_Char* name,
                           OD_Utl_XML_SaxParser &parser) :
        m_errorCount(0),
        m_fatalError(false),
        m_fileName(ACE::strnew (name)), 
        m_locator(NULL),
        m_parser(parser) {
     }
     virtual ~OD_UTL_XML_SAXHandler(void) {
        delete [] m_fileName;
     }

     // Methods inherit from ACEXML_ContentHandler.
     virtual void characters (const ACEXML_Char *ch,
                              int start,
                              int length ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         m_parser.characters(ch, start, length);
     }
     virtual void endDocument (ACEXML_ENV_SINGLE_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         m_parser.endDocument();
     }
     virtual void endElement (const ACEXML_Char *namespaceURI,
                              const ACEXML_Char *localName,
                              const ACEXML_Char *qName ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         m_parser.endElement(namespaceURI, localName, qName);
     }
     virtual void endPrefixMapping (const ACEXML_Char *prefix ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }  
     virtual void ignorableWhitespace (const ACEXML_Char *ch,
                                       int start,
                                       int length ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }
     virtual void processingInstruction (const ACEXML_Char *target,
                                         const ACEXML_Char *data ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }
     virtual void setDocumentLocator (ACEXML_Locator *locator) {
         m_locator = locator;
     }
     virtual void skippedEntity (const ACEXML_Char *name ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }
     virtual void startDocument (ACEXML_ENV_SINGLE_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         m_parser.startDocument();
     }
     virtual void startElement (const ACEXML_Char *namespaceURI,
                                const ACEXML_Char *localName,
                                const ACEXML_Char *qName,
                                ACEXML_Attributes *atts ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) { 
         m_parser.startElement(namespaceURI, localName, qName, atts);
     }
     virtual void startPrefixMapping (const ACEXML_Char *prefix,
                                      const ACEXML_Char *uri ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }

     // *** Methods inherit from ACEXML_DTDHandler.
     virtual void notationDecl (const ACEXML_Char *name,
                                const ACEXML_Char *publicId,
                                const ACEXML_Char *systemId ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }
     virtual void unparsedEntityDecl (const ACEXML_Char *name,
                                      const ACEXML_Char *publicId,
                                      const ACEXML_Char *systemId,
                                      const ACEXML_Char *notationName ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
     }

     // Methods inherit from ACEXML_EnitityResolver.
     virtual ACEXML_InputSource *resolveEntity (const ACEXML_Char *publicId,
                                                const ACEXML_Char *systemId ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         return (NULL);
     }

     // Methods inherit from ACEXML_ErrorHandler.
     virtual void error (ACEXML_SAXParseException &exception ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         AAA_LOG ((LM_INFO, "Error %s: line: %d col: %d \n",
                    (m_locator->getSystemId() == 0 ? m_fileName : m_locator->getSystemId()),
                     m_locator->getLineNumber(),
                     m_locator->getColumnNumber()));
         exception.print();
         m_errorCount ++;
     }
     virtual void fatalError (ACEXML_SAXParseException &exception ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         AAA_LOG ((LM_INFO, "Fatal error %s: line: %d col: %d \n",
                    (m_locator->getSystemId() == 0 ? m_fileName : m_locator->getSystemId()),
                     m_locator->getLineNumber(),
                     m_locator->getColumnNumber()));
         exception.print();
         m_fatalError = true;
     }
     virtual void warning (ACEXML_SAXParseException &exception ACEXML_ENV_ARG_DECL)
         ACE_THROW_SPEC ((ACEXML_SAXException)) {
         AAA_LOG ((LM_INFO, "Warning %s: line: %d col: %d \n",
                    (m_locator->getSystemId() == 0 ? m_fileName : m_locator->getSystemId()),
                     m_locator->getLineNumber(),
                     m_locator->getColumnNumber()));
         exception.print();
     }

     // local methods
     int ErrorCount() {
         return m_errorCount;
     }
     bool FatalError() {
         return m_fatalError;
     }
  private:
     int m_errorCount;
     bool m_fatalError;
     ACEXML_Char* m_fileName;
     ACEXML_Locator* m_locator;
     OD_Utl_XML_SaxParser &m_parser;
};

void OD_Utl_XML_SaxParser::startDocument(ACEXML_ENV_SINGLE_ARG_DECL)
{
}

void OD_Utl_XML_SaxParser::endDocument(ACEXML_ENV_SINGLE_ARG_DECL)
{
}

 void OD_Utl_XML_SaxParser::startElement(const ACEXML_Char *namespaceURI,
                                         const ACEXML_Char *localName,
                                         const ACEXML_Char *qName,
                                         ACEXML_Attributes *atts ACEXML_ENV_ARG_DECL)
{
   OD_Utl_XML_ElementMap::iterator i = 
       m_elementMap.find(std::string(qName));
   if (i != m_elementMap.end()) {
       i->second->startElement(atts);
       m_currentElement = i->second;
   }
}

void OD_Utl_XML_SaxParser::characters(const ACEXML_Char *ch,
                                      int start,
                                      int length ACEXML_ENV_ARG_DECL)
{
   if (m_currentElement) {
       m_currentElement->characters(ch, start, length);
   }
}
                                      
void OD_Utl_XML_SaxParser::endElement(const ACEXML_Char *namespaceURI,
                                       const ACEXML_Char *localName,
                                       const ACEXML_Char *qName ACEXML_ENV_ARG_DECL)
{
   OD_Utl_XML_ElementMap::iterator i = 
       m_elementMap.find(std::string(qName));
   if (i != m_elementMap.end()) {
       i->second->endElement();
   }
   m_currentElement = NULL;
}

void OD_Utl_XML_SaxParser::Load(char* xmlFile)
{
   ACEXML_FileCharStream *fstm = NULL;
   if (xmlFile == NULL) {
       throw OD_Utl_XML_SaxException("No dictionary file specified");
   }

   fstm = new ACEXML_FileCharStream;
   if (fstm == NULL) {
       throw OD_Utl_XML_SaxException("Allocation failure");
   }
   
   if (fstm->open (xmlFile) != 0) {
       std::string err = "Failed to open XML file: ";
       err += xmlFile;
       throw OD_Utl_XML_SaxException(err);
   }

   auto_ptr<ACEXML_DefaultHandler> handler
       (new OD_UTL_XML_SAXHandler(xmlFile, (*this)));
   if (handler.get() == NULL) {
       throw OD_Utl_XML_SaxException("Allocation failure");
   }

   ACEXML_Parser parser;
   ACEXML_InputSource input(fstm);

   parser.setContentHandler(handler.get());
   parser.setDTDHandler(handler.get());
   parser.setErrorHandler(handler.get());
   parser.setEntityResolver(handler.get());
    
   try {
      for (unsigned int passes = 0; passes < m_numPasses; passes ++) {
          parser.parse(&input ACEXML_ENV_ARG_NOT_USED);
      }
   }
   catch (ACEXML_SAXException &ex) {
       ex.print();
       throw OD_Utl_XML_SaxException("Parsing failure");
   }

   OD_UTL_XML_SAXHandler *h = static_cast<OD_UTL_XML_SAXHandler*>(handler.get());
   if (h->FatalError() || (h->ErrorCount() > 0)) {
       throw OD_Utl_XML_SaxException("Parsing failure");
   }       
}
