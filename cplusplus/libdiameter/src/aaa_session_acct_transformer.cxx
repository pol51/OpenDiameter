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

/* Author   : Victor I. Fajardo
 * Synopsis : Base class for accouting support
 */

#include <sstream>
#include <string>
#include "aaa_log_facility.h"
#include "diameter_api.h"

class AAAXelement
{
   public:
      AAAXelement(char *name) :
          m_name(name) {
      }

      void SetText(const char *text) {
          m_value = text;
      }
      void SetText(ACE_UINT32 num) {
          m_value = num;
      }
      void SetText(ACE_UINT64 num) {
          m_value = "undefined";
      }
      void SetText(diameter_uri_t &uri) {
          m_value += uri.fqdn;
          m_value += ":";
          m_value += uri.port;

          switch (uri.transport) {
             case DIAMETER_TRANSPORT_PROTO_TCP: m_value += ";transport=tcp"; break;
             case DIAMETER_TRANSPORT_PROTO_SCTP: m_value += ";transport=sctp"; break;
             case DIAMETER_TRANSPORT_PROTO_UDP: m_value += ";transport=udp"; break;
          }

          switch (uri.protocol) {
             case DIAMETER_PROTO_DIAMETER: m_value += ";protocol=diameter"; break;
             case DIAMETER_PROTO_RADIUS: m_value += ";protocol=radius"; break;
             case DIAMETER_PROTO_TACACSPLUS: m_value += ";protocol=tacacsplus"; break;
          }

          switch (uri.scheme) {
             case DIAMETER_SCHEME_AAA: m_value += ";scheme=aaa"; break;
             case DIAMETER_SCHEME_AAAS: m_value += ";scheme=aaas"; break;
          }
      }
      void SetAttribute(const char *name, const char *value) {
          m_attributes += " ";
          m_attributes += name;
          m_attributes += "=";
          m_attributes += value;
      }
      void SetAttribute(const char *name, ACE_UINT32 num) {
          std::string value;
          value += num;
          SetAttribute(name, value.data());
      }
      void SetAttribute(const char *name, ACE_UINT64 num) {
          std::string value = "undefined";
          SetAttribute(name, value.data());
      }
      std::string Output() {
          std::string output;
          output += "<";
          output += m_name;
          output += m_attributes;
          output += ">";
          output += m_value;
          output += "</";
          output += m_name;
          output += ">";
          return output;
      }
      void Reset() {
          m_attributes = "";
          m_value = "";
      }

   private:
      std::string m_name;
      std::string m_attributes;
      std::string m_value;
};

class AAAXwriter
{
   public:
      AAAXwriter() { };

      void writeToString(DiameterMsg *msg, 
                         std::string &output);

   protected:
      AAAReturnCode Walk(AAAAvpContainerList &avplist,
                         std::string &output);
};

void AAAXwriter::writeToString(DiameterMsg *msg,
                                      std::string &output)
{
   output = "<Message>";

   AAAXelement version("version");
   version.SetText(ACE_UINT32(msg->hdr.ver));
   output += version.Output();

   AAAXelement flags("flags");
   flags.SetAttribute("request", ACE_UINT32(msg->hdr.flags.r));
   flags.SetAttribute("proxiable", ACE_UINT32(msg->hdr.flags.p));
   flags.SetAttribute("error", ACE_UINT32(msg->hdr.flags.e));
   flags.SetAttribute("retrans", ACE_UINT32(msg->hdr.flags.t));
   output += flags.Output();

   AAAXelement code("code");
   code.SetText(msg->hdr.code);
   output += flags.Output();

   AAAXelement appId("appId");
   appId.SetText(msg->hdr.appId);
   output += flags.Output();

   AAAXelement HopId("HopId");
   HopId.SetText(msg->hdr.hh);
   output += flags.Output();

   AAAXelement EndId("EndId");
   EndId.SetText(msg->hdr.ee);
   output += flags.Output();

   output += "<avp>";

   if (Walk(msg->acl, output) == AAA_ERR_SUCCESS) {
      output += "</avp>";
      output += "</Message>";
   }
   else {
      output = "";
   }
}

AAAReturnCode AAAXwriter::Walk(AAAAvpContainerList &avplist,
                               std::string &output)
{
   std::list<AAAAvpContainer*>::iterator i;
   for (i = avplist.begin(); i != avplist.end(); i ++) {   

      AAAAvpContainer *c = *i;
      AAAAvpContainerEntry *entry;            
      AAAXelement avp((char*)c->getAvpName());

      size_t y;
      for (y = 0; y < c->size(); y++) {
  	 entry = (*c)[y];
         switch (entry->dataType()) {
	    case AAA_AVP_ADDRESS_TYPE:
	       {
	           diameter_address_t &val = entry->dataRef
                         (Type2Type<diameter_address_t>());
	           avp.SetText(val.value.data());
               }
               break;
	    case AAA_AVP_STRING_TYPE:
	    case AAA_AVP_UTF8_STRING_TYPE:
	    case AAA_AVP_DIAMID_TYPE:
	       {
	           diameter_octetstring_t &val = entry->dataRef
                         (Type2Type<diameter_octetstring_t>());
	           avp.SetText(val.data());
               }
	       break;
	    case AAA_AVP_INTEGER32_TYPE:
	    case AAA_AVP_UINTEGER32_TYPE:
	    case AAA_AVP_ENUM_TYPE:
	       {
                  diameter_unsigned32_t &val = entry->dataRef
                        (Type2Type<diameter_unsigned32_t>());
	          avp.SetText(val);
               }
	       break;
	    case AAA_AVP_INTEGER64_TYPE:
	    case AAA_AVP_UINTEGER64_TYPE:
	       {
                  diameter_unsigned64_t &val = entry->dataRef
                        (Type2Type<diameter_unsigned64_t>());
	          avp.SetText(val);
               }
	       break;
  	    case AAA_AVP_GROUPED_TYPE:
	       {
                  output += "<";
                  output += c->getAvpName();
                  output += ">";
                  diameter_grouped_t &val = entry->dataRef
                        (Type2Type<diameter_grouped_t>());
	          Walk(val, output);
                  output += "</";
                  output += c->getAvpName();
                  output += ">";
               }
	       break;
	    case AAA_AVP_DIAMURI_TYPE:
	       {
                  diameter_uri_t &val = entry->dataRef
                        (Type2Type<diameter_uri_t>());
	          avp.SetText(val);
               }
	       break;
	    default:
	       break;
	 }
         output += avp.Output();
         avp.Reset();
      }   
   }

   return (AAA_ERR_SUCCESS);
}

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
AAAReturnCode AAAAccountingXMLRecTransformer::Convert(DiameterMsg *msg)
{
   AAAXwriter writer;

   std::string output;
   writer.writeToString(msg, output);

   if (output.length()) {
      record = reinterpret_cast<void*>(ACE_OS::strdup(output.data()));
      record_size = ACE_OS::strlen(output.data());
   }
   else {
      record = NULL;
      record_size = 0;
   }

   return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAAAccountingXMLRecTransformer::OutputRecord(DiameterMsg *originalMessage)
{
   AAA_LOG(LM_DEBUG, "(%P|%t) Server: Default output record handler\n");

   if (record) {

      AAA_LOG(LM_DEBUG, "(%P|%t) Server: Resetting record holder\n");

      ACE_OS::free(record);
      record = NULL;
      record_size = 0;
   }
   return (AAA_ERR_SUCCESS);
}

