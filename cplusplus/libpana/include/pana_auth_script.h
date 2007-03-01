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
      }
      void Seed(PANA_SessionEventInterface::PANA_AuthorizationArgs &args) {
         m_Args = args;
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

         if (args.m_PacAddress.IsSet()) {
            FormatToString(args.m_PacAddress(), buf, sizeof(buf));
            AAA_LOG((LM_INFO, "PaC Address: %s\n", buf));
         }

         if (args.m_PaaAddress.IsSet()) {
            FormatToString(args.m_PaaAddress(), buf, sizeof(buf));
            AAA_LOG((LM_INFO, "PAA Address: %s\n", buf));
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
      }

   protected:
      void SendCmd(const char *cmd) {
         char buf[512];
         ACE_INET_Addr addr;
         std::string sysCmd;

#if defined(WIN32)
         sysCmd = "\"" + m_Script + "\" ";
#else
         sysCmd = m_Script + " ";
#endif
         sysCmd += cmd;
         sysCmd += " ";

         if (m_Args.m_PacAddress.IsSet()) {
            FormatToString(m_Args.m_PacAddress(), buf, sizeof(buf));
            sysCmd += buf;
            sysCmd += " ";
         }

         if (m_Args.m_PaaAddress.IsSet()) {
            FormatToString(m_Args.m_PaaAddress(), buf, sizeof(buf));
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
      static void FormatToString(ACE_INET_Addr &addr,
                                 char *buf,
                                 size_t bsize) {
         addr.addr_to_string(buf, bsize);
      }

   private:
      PANA_SessionEventInterface::PANA_AuthorizationArgs m_Args;
      pana_octetstring_t m_Script;
};

#endif // __PANA_AUTH_SCRIPT_H__
