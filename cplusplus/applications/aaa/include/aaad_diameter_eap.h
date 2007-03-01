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

#ifndef __AAAD_DIAMETER_EAP_H__
#define __AAAD_DIAMETER_EAP_H__

#include "aaad_call_framework.h"
#include "eap_method_registrar.hxx"
#include "eap_identity.hxx"
#include "eap_md5.hxx"
#include "eap_archie.hxx"
#include "eap_archie_fsm.hxx"
#include "diameter_eap_server_session.hxx"
#include "aaad_user_db.h"

class AAAD_EapAuthIdentityStateMachine : 
    public EapAuthIdentityStateMachine
{
      friend class EapMethodStateMachineCreator
         <AAAD_EapAuthIdentityStateMachine>;

   public:
      AAAD_EapAuthIdentityStateMachine
        (EapSwitchStateMachine &s) :
         EapAuthIdentityStateMachine(s) {
      } 
      ProcessIdentityResult ProcessIdentity
         (std::string& identity);

   private:
      ~AAAD_EapAuthIdentityStateMachine() {
      } 
};

class AAAD_EapAuthMD5ChallengeStateMachine 
  : public EapAuthMD5ChallengeStateMachine
{
      friend class EapMethodStateMachineCreator
           <AAAD_EapAuthMD5ChallengeStateMachine>;
   public:
      AAAD_EapAuthMD5ChallengeStateMachine
        (EapSwitchStateMachine &s) :
         EapAuthMD5ChallengeStateMachine(s) {
      } 
      void InputPassphrase();
      const char *GetSystemPasswd(std::string &user);

   private:
      ~AAAD_EapAuthMD5ChallengeStateMachine() {
      } 
};

class AAAD_EapAuthArchieStateMachine : 
   public EapAuthArchieStateMachine
{
      friend class EapMethodStateMachineCreator
                      <AAAD_EapAuthArchieStateMachine>;

   public:
      AAAD_EapAuthArchieStateMachine(EapSwitchStateMachine &s) :
            EapAuthArchieStateMachine(s) {
      }
      std::string& InputIdentity();
      std::string& InputSharedSecret();

   private:
      ~AAAD_EapAuthArchieStateMachine() {
      } 
      std::string m_SharedSecretBuf;
      std::string m_IdentityBuf;
};

typedef AAA_JobHandle<AAA_GroupedJob> 
        AAAD_EapJobHandle;

class AAAD_EapBackendAuthSwitchStateMachine :
    public EapBackendAuthSwitchStateMachine
{
    public:

       AAAD_EapBackendAuthSwitchStateMachine
          (ACE_Reactor &r, AAAD_EapJobHandle& h) :
	  EapBackendAuthSwitchStateMachine(r, h),
	  m_UserEntry(NULL),
          m_IdentityMethod(EapContinuedPolicyElement(EapType(1))),
          m_Md5Method(EapContinuedPolicyElement(EapType(4))),
	  m_ArchieMethod(EapContinuedPolicyElement(EapType(ARCHIE_METHOD_TYPE))) {

          Policy().InitialPolicyElement
                (&m_IdentityMethod);

          NeedInitialRequestToSend(false);

       }

       void Send(AAAMessageBlock *b);

       void Success(AAAMessageBlock *b);
  
       void Success();

       void Failure(AAAMessageBlock *b);

       void Failure();

       void Abort();

       EapAuthIdentityStateMachine::ProcessIdentityResult 
       ProcessIdentity(std::string& identity);

       AAAD_UserEntry *&CurrentUserEntry() {
	  return m_UserEntry;
       }
       std::string &CurrentIdentity() {
	  return m_Identity;
       }

    private:
       AAAD_UserEntry *m_UserEntry;
       std::string m_Identity;
       EapContinuedPolicyElement m_IdentityMethod;
       EapContinuedPolicyElement m_Md5Method;
       EapContinuedPolicyElement m_ArchieMethod;
};

class AAAD_DiameterEapServerSession : 
    public DiameterEapServerSession
{
    public:
       AAAD_DiameterEapServerSession
         (AAAApplicationCore& appCore, 
          diameter_unsigned32_t appId=EapApplicationId) :
          DiameterEapServerSession(appCore, appId),
	  m_Initial(true),
          m_Handle(EapJobHandle
	       (AAA_GroupedJob::Create(appCore.GetTask().Job(), 
                                       this, "backend"))),
          m_Eap(boost::shared_ptr<AAAD_EapBackendAuthSwitchStateMachine>
	        (new AAAD_EapBackendAuthSwitchStateMachine
		 (*appCore.GetTask().reactor(), m_Handle))) {
          this->Start();
       }
       void Start() throw(AAA_Error) {
          DiameterEapServerSession::Start(); 
       }
       void Abort() {
          DiameterEapServerSession::Stop();
          Eap().Stop();
       }
       AAAD_EapBackendAuthSwitchStateMachine& Eap() { 
          return *m_Eap; 
       }
       void ForwardEapResponse(std::string &eapMsg);
       bool ValidateUserName(const diameter_utf8string_t &userName) {
	  // TBD: Validate diameter username avp
	  return true; 
       }

    private:
       bool m_Initial;
       EapJobHandle m_Handle;
       boost::shared_ptr<AAAD_EapBackendAuthSwitchStateMachine> m_Eap;
};

typedef AAAServerSessionClassFactory<AAAD_DiameterEapServerSession> 
AAAD_EapServerFactory;

class AAAD_AppDiameterEap :
   public AAAD_CallElement
{
   public:
      virtual int Start(AAAApplicationCore &core);
      virtual bool IsRunning();
      virtual void Stop();

   private:
      EapMethodRegistrar m_MethodRegistrar;

      EapMethodStateMachineCreator
         <AAAD_EapAuthMD5ChallengeStateMachine> 
           m_AuthMD5ChallengeCreator;

      EapMethodStateMachineCreator
         <AAAD_EapAuthIdentityStateMachine> 
           m_AuthIdentityCreator;

      EapMethodStateMachineCreator
         <AAAD_EapAuthArchieStateMachine> 
           m_AuthArchieCreator;

      std::auto_ptr<AAAD_EapServerFactory> m_AuthFactory;
};

#endif // __AAAD_DIAMETER_EAP_H__

