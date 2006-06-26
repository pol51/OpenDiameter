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

#ifndef __PANA_CLIENT_SHELL_H__
#define __PANA_CLIENT_SHELL_H__

#include <string>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <ace/Synch.h>
#include <ace/Singleton.h>
#include "pana_config_manager.h"
#include "pana_auth_script.h"

#define PANA_CLIENT_DEFAULT_TCOUNT  5

class PANAClientEvent
{
   public:
       virtual void Success(std::string &interfaceName) = 0;
       virtual PANA_CfgProviderInfo *IspSelection(const PANA_CfgProviderList &list) = 0;
       virtual void Failure() = 0;
       virtual void Disconnect() = 0;
};

class PANAClientArg
{
   public:
       std::string m_PanaCfgFile;
       std::string m_Username;
       std::string m_Password;
       std::string m_SharedSecret;
       std::string m_AuthScript;
       int m_ThreadCount;
       int m_Timeout;
       bool m_Md5Only;
};

class PANAClientHandle 
{
};

class PANAClient
{
   public:
       void RegisterEvent(PANAClientEvent &e) { 
           m_Event = &e;
       }
       PANAClientEvent &Event() {
           return *m_Event;
       }
       PANAClientArg &Arg() {
           return m_Arg;
       }

       void Start();
       void Stop();
       void UpdateAddress(ACE_INET_Addr &addr,
                          std::string &message);
       void SendNotification(std::string &message);
       void SendPing();

   private:
       PANAClientArg m_Arg;
       PANAClientEvent *m_Event;
       boost::shared_ptr<PANAClientHandle> m_Handle;
       ofstream m_LogStream;
};

typedef ACE_Singleton<PANAClient, ACE_Null_Mutex> PANAClient_S;
#define PANA_CLIENT  PANAClient_S::instance()

#endif // __PANA_CLIENT_H__