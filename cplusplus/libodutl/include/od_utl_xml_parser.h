
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

#ifndef __OD_UTL_XMLPARSER_H__
#define __OD_UTL_XMLPARSER_H__

#include <string>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include "ace/OS.h"
#include "od_utl_exports.h"

#if defined(XERCES_HAS_CPP_NAMESPACE)
using namespace xercesc;
#endif

class OD_UTL_EXPORT OD_Utl_XMLDOMTreeErrorReporter  : public ErrorHandler
{
    public:
        OD_Utl_XMLDOMTreeErrorReporter() : 
           fSawErrors(false) { 
        }
        ~OD_Utl_XMLDOMTreeErrorReporter() { 
        }
        void warning(const SAXParseException& toCatch);
        void error(const SAXParseException& toCatch);
        void fatalError(const SAXParseException& toCatch);
        void resetErrors() { 
           /* no-op */ 
        }
        inline bool getSawErrors() const { 
           return fSawErrors; 
        }

    private:
        bool fSawErrors;
};

class OD_UTL_EXPORT OD_Utl_XMLElementParser {
   public:
      OD_Utl_XMLElementParser(const char *name) :
         m_Name(name) {
      }
      OD_Utl_XMLElementParser(std::string &name) :
         m_Name(name) {
      }
      virtual ~OD_Utl_XMLElementParser() { 
      }

      virtual int svc(DOMNode *n) = 0;

      std::string &name() { 
         return m_Name; 
      }

      virtual int populate(DOMNode *n, 
                           DOMNode **found = NULL);

      char *getTextContent(DOMNode *n);

      char *getAttribute(DOMNode *n, 
                         const char *name);

   protected:
      std::string m_Name;
};

template<class T>
class OD_UTL_EXPORT OD_Utl_XMLElementParserWithData : 
   public OD_Utl_XMLElementParser 
{
   public:
      OD_Utl_XMLElementParserWithData
          (std::string &name, T &data) :
           OD_Utl_XMLElementParser(name),
         m_Payload(data) { 
      }
      OD_Utl_XMLElementParserWithData
          (const char *name, T &data) :
           OD_Utl_XMLElementParser(name),
         m_Payload(data) { 
      }
   protected:
      T &m_Payload;
};

class OD_UTL_EXPORT OD_Utl_XMLTreeParser {
   public:
      int open(const char *fname,
               OD_Utl_XMLElementParser &root) {
         std::string name(fname);
         return open(name, root);
      }
      int open(std::string &filename, 
               OD_Utl_XMLElementParser &root);
      void close();
};

class OD_UTL_EXPORT OD_Utl_XMLDataString :
   public OD_Utl_XMLElementParserWithData<std::string>
{
   public:
      OD_Utl_XMLDataString(const char *name, 
                           std::string &data) : 
         OD_Utl_XMLElementParserWithData<std::string>
                           (name, data) {
      }
      OD_Utl_XMLDataString(std::string &name, 
                           std::string &data) : 
         OD_Utl_XMLElementParserWithData<std::string>
                           (name, data) {
      }
      virtual int svc(DOMNode *n) {
         char *c_str = getTextContent(n);
         if (c_str) {
             m_Payload.assign(c_str);
             XMLString::release(&c_str);
             return (0);
         }
         return (-1);
      }
};

class OD_UTL_EXPORT OD_Utl_XMLDataUInt32 : 
   public OD_Utl_XMLElementParserWithData<ACE_UINT32>
{
   public:
      OD_Utl_XMLDataUInt32(const char *name, 
                           ACE_UINT32 &data) : 
         OD_Utl_XMLElementParserWithData<ACE_UINT32>
                           (name, data) {
      }
      OD_Utl_XMLDataUInt32(std::string &name, 
                           ACE_UINT32 &data) : 
         OD_Utl_XMLElementParserWithData<ACE_UINT32>
                           (name, data) {
      }
      virtual int svc(DOMNode *n) {
         char *c_str = getTextContent(n);
         if (c_str) {
             m_Payload = ACE_OS::atoi(c_str);
             XMLString::release(&c_str);
             return (0);
         }
         return (-1);
      }
};

class OD_UTL_EXPORT DOMStrX {
    public:
       DOMStrX(const XMLCh * const toTranscode) { 
          m_LocalRef = XMLString::transcode(toTranscode); 
       }
       ~DOMStrX() { 
          XMLString::release(&m_LocalRef); 
       }
       char *localRef() { 
          return m_LocalRef; 
       }

    private:
       char *m_LocalRef;
};

#endif // __OD_UTL_XMLPARSER_H__
