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

#include <fstream>
#include "aaad_diameter_eap.h"
#include "aaad_user_db.h"

#if HAVE_SHADOW_H
#include <shadow.h>
#endif

EapAuthIdentityStateMachine::ProcessIdentityResult 
AAAD_EapAuthIdentityStateMachine::ProcessIdentity
         (std::string& identity) 
{
   AAAD_EapBackendAuthSwitchStateMachine &fsm = 
        (AAAD_EapBackendAuthSwitchStateMachine&)
            SwitchStateMachine();
   return fsm.ProcessIdentity(identity);
};

const char* 
AAAD_EapAuthMD5ChallengeStateMachine::GetSystemPasswd
(std::string &user)
{
#if HAVE_SHADOW_H
   struct spwd *spw = getspnam(user.data());
   if (spw == NULL) {
      AAAD_LOG(LM_INFO, 
               "(%P|%t) Invalid user [%s]\n",
               user.data());
      return NULL;
   }

   if (spw->sp_pwdp == '\0') {
      AAAD_LOG(LM_INFO, 
               "(%P|%t) No passwd set for [%s]\n",
               user.data());
      return ""; // allow no password
   }
#if 0
   char *epw = crypt(passwd, pw->pw_passwd);
   if (strcmp(epw, pw->pw_passwd)) {
      AAAD_LOG(LM_INFO, 
               "(%P|%t) Invalid passwd for [%s]\n",
               user.data());
      return NULL;
   }
#endif
   AAAD_LOG(LM_INFO, 
            "(%P|%t) User [%s] access granted\n",
            user.data());
   return spw->sp_pwdp;
#else
   AAAD_LOG(LM_INFO, 
            "(%P|%t) ERROR !!! No shadow passwd support\n");
   return false;
#endif
}

void AAAD_EapAuthMD5ChallengeStateMachine::InputPassphrase() 
{
   AAAD_EapBackendAuthSwitchStateMachine &fsm = 
        (AAAD_EapBackendAuthSwitchStateMachine&)
            SwitchStateMachine();

   AAAD_UserEntry *e = fsm.CurrentUserEntry();
   if (e == NULL) {
      AAAD_LOG(LM_INFO, 
             "(%P|%t) ERROR !!! User missing in md5 method\n");
      return;
   }

   int pos;
   const char *sysP;
   std::string user;
   std::string &pw = Passphrase();
   
   switch (e->Md5().PasswordType()) {
      case AAAD_UserEapMd5Method::SYSTEM:
         pos = fsm.CurrentIdentity().find('@');
         if (pos > 0) {
            user.assign(fsm.CurrentIdentity(), 0, pos);
         }
         else {
	    user = fsm.CurrentIdentity();
	 }
         sysP = GetSystemPasswd(user);
         if (sysP == NULL) {
            AAAD_LOG(LM_INFO, 
                "(%P|%t) ACCESS DENIED !!! \n");
            return;
         }
         pw = sysP;
         break;
      case AAAD_UserEapMd5Method::FLAT:
	 pw = e->Md5().Secret();
	 break;
      case AAAD_UserEapMd5Method::NONE:
      default:
	 break;
   }
}

std::string& AAAD_EapAuthArchieStateMachine::InputIdentity() 
{
   m_IdentityBuf = "";

   std::string buf("diameter_eap");
   AAAD_AppDiamEapData *d = (AAAD_AppDiamEapData*) 
              AAAD_APP_TBL().Lookup(buf);
   if (d) {
      return d->Application().LocalIdentity();
   }
   else {
      AAAD_LOG(LM_INFO, 
            "(%P|%t) ERROR !!! Missing in eap config during method\n");
   }
   return m_IdentityBuf;
}

std::string& AAAD_EapAuthArchieStateMachine::InputSharedSecret() 
{
   AAAD_EapBackendAuthSwitchStateMachine &fsm = 
        (AAAD_EapBackendAuthSwitchStateMachine&)
            SwitchStateMachine();

   m_SharedSecretBuf = "";
   try {
      AAAD_UserEntry *e = fsm.CurrentUserEntry();
      if (e == NULL) {
         AAAD_LOG(LM_INFO, 
               "(%P|%t) ERROR !!! User missing in archie method\n");
         throw (0);
      }

      std::string &fname = e->Archie().SharedSecretFile();

      struct stat attr;
      memset(&attr, 0, sizeof(attr));
      if (stat(fname.data(), &attr) < 0) {
         AAAD_LOG(LM_INFO, 
               "(%P|%t) ERROR !!! Failed to stat [%s]\n",
               fname.data());
         throw (0);
      }

      char *buf = new char[attr.st_size];
      memset(buf, 0x0, sizeof(buf));

      ifstream secretf;
      secretf.open(fname.data(), ios::binary | ios::in);
      if (secretf.is_open()) {
         secretf.read(buf, attr.st_size);
         m_SharedSecretBuf.assign(buf, attr.st_size);
      }
      else {
         AAAD_LOG(LM_INFO, 
               "(%P|%t) ERROR !!! Failed to open [%s]\n",
               fname.data());
         throw (0);
      }
      secretf.close();
   }
   catch (...) {
   }
   return m_SharedSecretBuf;
}

EapAuthIdentityStateMachine::ProcessIdentityResult 
AAAD_EapBackendAuthSwitchStateMachine::ProcessIdentity
(std::string& identity) 
{
   m_UserEntry = (AAAD_UserEntry*)AAAD_USER_DB().Match(identity);
   if (m_UserEntry == NULL) {
      std::string buf("default");
      m_UserEntry = (AAAD_UserEntry*)AAAD_USER_DB().Lookup(buf);
   }
   if (m_UserEntry == NULL) {
      AAAD_LOG(LM_INFO, 
           "(%P|%t) ERROR !!! No matching entry in user db \n");
      return EapAuthIdentityStateMachine::Failure;
   }

   AAAD_LOG(LM_INFO, "(%P|%t) *** Match User [%s]: %s\n", 
            identity.data(), m_UserEntry->Name().data());

   if (m_UserEntry->Method() == "md5") {
      m_IdentityMethod.AddContinuedPolicyElement
         (&m_Md5Method, 
           EapContinuedPolicyElement::PolicyOnSuccess);
   }
   else if (m_UserEntry->Method() == "archie") {
      m_IdentityMethod.AddContinuedPolicyElement
         (&m_ArchieMethod, 
           EapContinuedPolicyElement::PolicyOnSuccess);
   }
   else {
      AAAD_LOG(LM_INFO, 
            "(%P|%t) ERROR !!! No configure eap method for user \n");
      m_UserEntry = NULL;
      return EapAuthIdentityStateMachine::Failure;
   }      

   m_Identity = identity;
   return EapAuthIdentityStateMachine::Success;
}

void 
AAAD_EapBackendAuthSwitchStateMachine::Send(AAAMessageBlock *b)
{
   std::string eapMsg(b->base(), b->length());
   JobData(Type2Type<AAAD_DiameterEapServerSession>()).
             SignalContinue(eapMsg);
}

void 
AAAD_EapBackendAuthSwitchStateMachine::Success(AAAMessageBlock *b)
{
   std::string eapMsg(b->base(), b->length());
   JobData(Type2Type<AAAD_DiameterEapServerSession>()).
             SignalSuccess(eapMsg);
   Stop();
}

void 
AAAD_EapBackendAuthSwitchStateMachine::Success()
{
   std::string eapMsg("");
   JobData(Type2Type<AAAD_DiameterEapServerSession>()).
             SignalSuccess(eapMsg);
   Stop();
}

void 
AAAD_EapBackendAuthSwitchStateMachine::Failure(AAAMessageBlock *b)
{
   std::string eapMsg(b->base(), b->length());
   JobData(Type2Type<AAAD_DiameterEapServerSession>()).
             SignalFailure(eapMsg);
   Stop();
}

void 
AAAD_EapBackendAuthSwitchStateMachine::Failure()
{
   std::string eapMsg("");
   JobData(Type2Type<AAAD_DiameterEapServerSession>()).
             SignalFailure(eapMsg);
   Stop();
}

void 
AAAD_EapBackendAuthSwitchStateMachine::Abort()
{
   Stop();
}

void 
AAAD_DiameterEapServerSession::ForwardEapResponse(std::string &eapMsg)
{
   AAAMessageBlock *msg;
   if (eapMsg.length() > 0) {
      msg = AAAMessageBlock::Acquire((ACE_UINT32)eapMsg.length());
      msg->copy((char*)eapMsg.data(), eapMsg.length());
      if (m_Initial) {
         m_Initial=false;
	 Eap().Start(msg);
      }
      else {
         Eap().Receive(msg);
      }
      msg->release();
   }
}

int AAAD_AppDiameterEap::Start(AAAApplicationCore &core)
{
   std::string str("diameter_eap");
   AAAD_AppDiamEapData *d = (AAAD_AppDiamEapData*)
       AAAD_APP_TBL().Lookup(str);
   if (d && d->Enabled()) {
       AAAD_UserDbLoader userDbLoader
              (d->Application().UserDbFile().data());

       AAAD_LOG(LM_INFO, "(%P|%t) **** User database ****\n");
       AAAD_USER_DB().Dump();
   }
   else {
       AAAD_LOG(LM_INFO, 
          "(%P|%t) No configuration entry for diameter eap !!!\n");
       return (-1);
   }

   m_AuthFactory = std::auto_ptr<AAAD_EapServerFactory>
         (new AAAD_EapServerFactory(core, EapApplicationId));

   core.RegisterServerSessionFactory(m_AuthFactory.get());

   m_MethodRegistrar.registerMethod
      (std::string("Identity"), EapType(1), 
       Authenticator, m_AuthIdentityCreator);

   m_MethodRegistrar.registerMethod
      (std::string("MD5-Challenge"), EapType(4), 
       Authenticator, m_AuthMD5ChallengeCreator);

   m_MethodRegistrar.registerMethod
      (std::string("Archie"), EapType(ARCHIE_METHOD_TYPE), 
       Authenticator, m_AuthArchieCreator);

   return (0);
}

bool AAAD_AppDiameterEap::IsRunning()
{
   return (true);
}

void AAAD_AppDiameterEap::Stop()
{
}
