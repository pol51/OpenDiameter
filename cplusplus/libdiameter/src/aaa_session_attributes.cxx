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

#include "aaa_data_defs.h"
#include "aaa_route_id_generator.h"
#include "aaa_session_attributes.h"

void DiameterSessionAttributes::MsgIdTxMessage(DiameterMsg &msg)
{
   if (msg.hdr.flags.r) {
       if (msg.hdr.hh == 0) {
           m_LastTxHopId = DIAMETER_HOPBYHOP_GEN()->Get();
           msg.hdr.hh = m_LastTxHopId;
       }
       else {
	   m_LastTxHopId = msg.hdr.hh;
       }
       if (msg.hdr.ee == 0) {
           m_LastTxEndId = DIAMETER_ENDTOEND_GEN()->Get();
           msg.hdr.ee = m_LastTxEndId;
       }
       else {
           m_LastTxEndId = msg.hdr.ee;
       }
   }
   else {
       msg.hdr.hh = m_LastRxHopId;
       msg.hdr.ee = m_LastRxEndId;
   }
}

bool DiameterSessionAttributes::MsgIdRxMessage(DiameterMsg &msg)
{
   if (msg.hdr.flags.r) {
       m_LastRxHopId = msg.hdr.hh;
       m_LastRxEndId = msg.hdr.ee;
       return (true);
   }
   return ((msg.hdr.hh == m_LastTxHopId) &&
           (msg.hdr.ee == m_LastTxEndId));
}

AAAReturnCode DiameterSessionId::Get(DiameterMsg &msg)
{
   DiameterUtf8AvpContainerWidget sidAvp(msg.acl);
   diameter_utf8string_t *sid = sidAvp.GetAvp(DIAMETER_AVPNAME_SESSIONID);
   try {
      if (sid == NULL) {
         throw (AAA_ERR_PARSING_FAILED);
      }
      ACE_UINT32 where1 = sid->find(";");
      if (where1 == std::string::npos) {
         throw (AAA_ERR_PARSING_FAILED);
      }
      DiameterId() = sid->substr(0, where1);

      ACE_UINT32 where2 = sid->find(";", ++ where1);
      if (where2 == std::string::npos) {
         throw (AAA_ERR_PARSING_FAILED);
      }
      std::string stringHigh = sid->substr(where1, where2-where1);
      High() = ACE_OS::strtoul(stringHigh.c_str(), NULL, 10);

      where1 = sid->find(";", ++ where2);
      if (where1 == std::string::npos) {
         std::string stringLow = sid->substr(where2);
         Low() = ACE_OS::atoi(stringLow.c_str());
      }
      else {
         std::string stringLow = sid->substr(where2, where1-where2);
         Low() = ACE_OS::atoi(stringLow.c_str());
         OptionalValue() = sid->substr(++ where1);
      }
      return (AAA_ERR_SUCCESS);
   }
   catch (AAAReturnCode rc) {
      return (rc);
   }
}

AAAReturnCode DiameterSessionId::Set(DiameterMsg &msg)
{
   DiameterUtf8AvpContainerWidget sidAvp(msg.acl);
   diameter_utf8string_t &sid = sidAvp.AddAvp(DIAMETER_AVPNAME_SESSIONID);

   char nums[64];
   sprintf(nums, ";%u;%u", High(), Low());
   sid = DiameterId() + nums;

   if (OptionalValue().length() > 0) {
       sid += ";";
       sid += OptionalValue();
   }
   return (AAA_ERR_SUCCESS);
}

void DiameterSessionId::Dump()
{
   std::string dump;
   Dump(dump);
   AAA_LOG((LM_INFO, "(%P|%t) Session id=%s\n", dump.c_str()));
}

void DiameterSessionId::Dump(std::string &dump)
{
   dump = DiameterId().c_str();
   dump += ';';
   dump += High();
   dump += ';';
   dump += Low();
   dump += ';';
   dump += OptionalValue().c_str();   
}
