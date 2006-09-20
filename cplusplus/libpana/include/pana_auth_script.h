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


#ifndef __PANA_AUTH_SCRIPT_H__
#define __PANA_AUTH_SCRIPT_H__

#include <iostream>
#include <stdlib.h>
#include "pana_session.h"

class PANA_AuthScriptCtl
{
   public:
      PANA_AuthScriptCtl() {
      }
      PANA_AuthScriptCtl(pana_octetstring_t &script) :
         m_Script(script) { 
      }
      ~PANA_AuthScriptCtl() {
         ClearEp();
      }
      void Seed(PANA_SessionEventInterface::PANA_AuthorizationArgs &args) {
         m_Args = args;
         m_Args.m_Ep.Clear();
         if (args.m_Ep.IsSet()) {
            ClearEp();
            m_Args.m_Ep = new PANA_DeviceIdContainer;
            m_Args.m_Ep()->clone(*args.m_Ep());
         }
      }
      void Add() {
         SendCmd("add");
      }
      void Update() {
         SendCmd("update");
      }
      void Remove() {
         SendCmd("del");
      }
      std::string &CurrentScript() {
          return m_Script;
      }
      PANA_SessionEventInterface::PANA_AuthorizationArgs &CurrentArgs() {
          return m_Args;
      }
      static void Print(PANA_SessionEventInterface::PANA_AuthorizationArgs &args) {
         char buf[512];

         if (args.m_Pac.IsSet()) {        
            FormatToString(args.m_Pac(), buf, sizeof(buf));
            AAA_LOG((LM_INFO, "PaC Device Id: %s\n", buf));
         }

         if (args.m_Paa.IsSet()) {        
            FormatToString(args.m_Paa(), buf, sizeof(buf));
            AAA_LOG((LM_INFO, "PAA Device Id: %s\n", buf));
         }

         if (args.m_Key.IsSet()) {
            FormatToString(args.m_Key(), buf, sizeof(buf));
            AAA_LOG((LM_INFO, "AAA-Key: %s\n", buf));
         }

         if (args.m_KeyId.IsSet()) {
            printf("Key id: %d\n", args.m_KeyId());
         }

         if (args.m_Lifetime.IsSet()) {
            printf("Lifetime: %d\n", args.m_Lifetime());
         }

         if (args.m_ProtectionCapability.IsSet()) {
            printf("Protection Cap: %d\n", args.m_ProtectionCapability());
         }

         if (args.m_PMKKeyList.IsSet()) {
            PAMA_PMKKeyListIterator i;
            for (i = args.m_PMKKeyList().begin(); 
                 i != args.m_PMKKeyList().end(); 
                 i ++ ) {
                FormatToString((*i), buf, sizeof(buf));
                AAA_LOG((LM_INFO, "PMK-Key: %s\n", buf));
            }
         }

         if (args.m_PreferedISP.IsSet()) {
            AAA_LOG((LM_INFO, "Prefered ISP: %s\n", 
	              args.m_PreferedISP().m_Name.data()));
         }

         if (args.m_PreferedNAP.IsSet()) {
            AAA_LOG((LM_INFO, "Prefered NAP: %s\n", 
		      args.m_PreferedNAP().m_Name.data()));
         }

         if (args.m_Ep.IsSet()) {
            PANA_DeviceId *id;
            PANA_DeviceIdIterator i = args.m_Ep()->begin();
            for (;i != args.m_Ep()->end(); i++) {
                id = (*i);
                FormatToString(*id, buf, sizeof(buf));
	        AAA_LOG((LM_INFO, "EP Device id: %s\n", buf));
            }
         }
      }

   protected:
      void ClearEp() {
         if (m_Args.m_Ep.IsSet()) {
             delete m_Args.m_Ep();
             m_Args.m_Ep.Clear();
         }
      }
      void SendCmd(const char *cmd) {
         char buf[512];
         ACE_INET_Addr addr;
         PANA_DeviceId *id;
         std::string sysCmd;

#if defined(WIN32)
         sysCmd = "\"" + m_Script + "\" ";
#else
         sysCmd = m_Script + " ";
#endif
         sysCmd += cmd;
         sysCmd += " ";

         if (m_Args.m_Pac.IsSet()) {        
            FormatToString(m_Args.m_Pac(), buf, sizeof(buf));
            sysCmd += buf;
            sysCmd += " ";
         }

         if (m_Args.m_Paa.IsSet()) {        
            FormatToString(m_Args.m_Paa(), buf, sizeof(buf));
            sysCmd += buf;
            sysCmd += " ";
         }

         if (m_Args.m_Key.IsSet()) {
            FormatToString(m_Args.m_Key(), buf, sizeof(buf));
            sysCmd += buf;
            sysCmd += " ";
         }

         if (m_Args.m_KeyId.IsSet()) {
            sprintf(buf, "%d", m_Args.m_KeyId());
            sysCmd += buf;
            sysCmd += " ";
         }

         if (m_Args.m_Lifetime.IsSet()) {
            sprintf(buf, "%d", m_Args.m_Lifetime());
            sysCmd += buf;
            sysCmd += " ";
         }

         if (m_Args.m_ProtectionCapability.IsSet()) {
            sprintf(buf, "%d", m_Args.m_ProtectionCapability());
            sysCmd += buf;
            sysCmd += " ";
         }

         if (m_Args.m_PMKKeyList.IsSet()) {
            PAMA_PMKKeyListIterator i;
            for (i = m_Args.m_PMKKeyList().begin(); 
                 i != m_Args.m_PMKKeyList().end(); 
                 i ++ ) {
                FormatToString((*i), buf, sizeof(buf));
                sysCmd += buf;
                sysCmd += " ";
            }
         }

         if (m_Args.m_PreferedISP.IsSet()) {
            sysCmd += m_Args.m_PreferedISP().m_Name;
            sysCmd += ":";
            sysCmd += m_Args.m_PreferedISP().m_Id;
            sysCmd += " ";
         }

         if (m_Args.m_PreferedNAP.IsSet()) {
            sysCmd += m_Args.m_PreferedNAP().m_Name;
            sysCmd += ":";
            sysCmd += m_Args.m_PreferedNAP().m_Id;
            sysCmd += " ";
         }

         if (m_Args.m_Ep.IsSet()) {
            PANA_DeviceIdIterator i = m_Args.m_Ep()->begin();
            for (;i != m_Args.m_Ep()->end(); i++) {
                id = (*i);
                FormatToString(*id, buf, sizeof(buf));
                sysCmd += buf;
                sysCmd += " ";
            }
         }

         if (m_Script.length() > 0) {
            system(sysCmd.data());
         }
	 else {
            AAA_LOG((LM_INFO, "%s is invalid !\n"));
	 }
      }
   protected:
      static void FormatToString(pana_octetstring_t &data,
                          char *buf,
                          size_t bsize) {
         ACE_OS::memset(buf, 0, bsize);
         char *tbuf = new char[data.size()+1];
         for (size_t i=0; i<data.size();i++) {
             sprintf(tbuf, "%02X", (unsigned char)data.data()[i]);
             if ((i*2) < bsize) {
                 ACE_OS::strcat(buf, tbuf);
                 continue;
             }
             break;
         }
         delete tbuf;
      }
      static void FormatToString(PANA_DeviceId &id, 
                          char *buf,
                          size_t bsize) {
         switch (id.type) {
            case AAA_ADDR_FAMILY_IPV4:
            case AAA_ADDR_FAMILY_IPV6: {
                  ACE_INET_Addr addr;
                  PANA_DeviceIdConverter::FormatToAddr(id, addr);
                  addr.addr_to_string(buf, bsize);
               }
               break;
            case AAA_ADDR_FAMILY_802:
            default: 
               FormatToString(id.value, buf, bsize);  
               break;
         }
      }

   private:
      PANA_SessionEventInterface::PANA_AuthorizationArgs m_Args;
      pana_octetstring_t m_Script;
};

#endif // __PANA_AUTH_SCRIPT_H__
