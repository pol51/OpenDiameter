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

#ifndef __AAA_MSG_TO_XML_H__
#define __AAA_MSG_TO_XML_H__

#include <sstream>
#include <string>
#include "diameter_parser.h"

class AAA_PARSER_EXPORT_ONLY AAAXmlElement
{
   public:
      AAAXmlElement(char *name) :
          m_name(name) {
      }
      void SetText(const char *text);
      void SetText(ACE_UINT32 num);
      void SetText(ACE_UINT64 num);
      void SetText(diameter_uri_t &uri);
      void SetAttribute(const char *name, const char *value);
      void SetAttribute(const char *name, ACE_UINT32 num);
      void SetAttribute(const char *name, ACE_UINT64 num);
      std::string Output();
      void Reset();

   private:
      std::string m_name;
      std::string m_attributes;
      std::string m_value;
};

class AAA_PARSER_EXPORT_ONLY AAAXmlWriter
{
   public:
      AAAXmlWriter() { };

      void writeToString(DiameterMsg *msg, 
                         std::string &output);

   protected:
      AAAReturnCode Walk(AAAAvpContainerList &avplist,
                         std::string &output);
};

/*!
 *  <Message>
 *     <version>value</version>
 *     <flags request="value" proxiable="value" error="value" retrans="value"></flags>
 *     <code>value</code>
 *     <appId>value</appId>
 *     <HopId>value</HopId>
 *     <EndId>value</EndId>
 *     <avp>
 *        <"avpname">value</avp>
 *          .
 *          .
 *        <"avpname">
 *           <"avpname">value</"avpname">
 *           <"avpname">value</"avpname">
 *               .
 *               .
 *           <"avpname">
 *              <"avpname">value</"avpname">
 *                 .
 *                 .
 *              </"avpname">
 *        </"avpname">
 *     </avp>
 *  </Message>
 */

class AAA_PARSER_EXPORT_ONLY AAADiameterMsgToXML
{
    public:
        static void Convert(DiameterMsg *msg);
};

#endif // __AAA_MSG_TO_XML_H__
