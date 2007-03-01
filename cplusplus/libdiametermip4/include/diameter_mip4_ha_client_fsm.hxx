/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* OpenDiameter: Open-source software for the Diameter protocol           */
/*                                                                        */
/* Copyright (C) 2002 -2007  Open Diameter Project 		          */   
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
/* 
   diameter_mip4_ha_client_fsm.hxx
   Client Statemachine definition for Diameter MIP HA Application 
   Written by Miriam Tauil
   Created June 1, 2004.
*/

#ifndef __MIP4_HA_CLIENT_FSM_H__
#define __MIP4_HA_CLIENT_FSM_H__

#include "framework.h"
#include "diameter_mip4_parser.hxx"
#include "diameter_api.h"


/*!
 * Windows specific export declarations
 */
#ifdef WIN32
   #if defined(DIAMETER_MIP4_HA_CLIENT_EXPORT)
       #define DIAMETER_MIP4_HA_CLIENT_EXPORTS __declspec(dllexport)
   #else
       #define DIAMETER_MIP4_HA_CLIENT_EXPORTS __declspec(dllimport)
   #endif
#else
   #define DIAMETER_MIP4_HA_CLIENT_EXPORTS
   #define DIAMETER_MIP4_HA_CLIENT_EXPORTS
#endif

typedef AAA_JobHandle<AAA_GroupedJob> DiameterJobHandle;


/// State machine for Diameter MIP HA clients.  It has many member
/// functions for enforcement of attributes (i.e., EnforceXYZ) and 
/// for setting attributes (i.e., SetXYX).  Arguments of enforcement
/// functions are not enveloped with AAA_ScholorAttribute or
/// DiameterVectorAttribute since they are intended to be unmodified.
class DIAMETER_MIP4_HA_CLIENT_EXPORTS DiameterMip4HaClientStateMachine 
  : public AAA_StateMachine<DiameterMip4HaClientStateMachine>,
    public AAA_EventQueueJob
{
  friend class DiameterJobMultiplexor;

 public:
  /// Constructor.
  DiameterMip4HaClientStateMachine( AAAClientSession& s,
				    DiameterJobHandle &h); 

  ~DiameterMip4HaClientStateMachine() 
  {
    handle.Job().Remove(this); 
  }
  
  // expose value of protected variable from  DiameterMip4HaClientStateMachine
  
  AAA_State State()
  {
    return AAA_StateMachine<DiameterMip4HaClientStateMachine>::state; 
  }
  
  static AAA_State InitialState()
  {
    return StInitialize;
  }
  
  enum {
    EvRxMipRegReq,
    EvRxAMA,
    EvSgSuccess,
    EvSgFailure,
    EvSgSessionTimeout,
    EvSgAuthLifetimeTimeout,
    EvSgAuthGracePeriodTimeout,
    EvSgTimeout,
    EvSgDisconnect,
    EvSgReset
  };
  
  // This is a copy from diameter_mip4_ha_client_fsm.cxx
  // DiameterMip4ClientStateTable_S, since I couldn't expose StInitialize
  // to be retuned to the application as a sign for a free session object.
  enum state {
    StInitialize //,
    //StWaitAMA,
    //StCheckResultCode,
    //StAccepted,
    //StRejected,
    //StTerminated
  };

  /// Store an event and notify the session.
  inline void Notify(AAA_Event ev) {
    // Enqueue the event.
    AAA_EventQueueJob::Enqueue(ev);

    // Notify the session.
    handle.Job().Schedule(this);
  }

  //inline DiameterMip4HaClientSession& Session() { return session; }
  inline AAAClientSession& Session() { return session; }

  /// This is used for constructing and sending a
  /// Diameter-AMR-Request.
  void SendAMR();

  inline AAA_JobData& JobData() { return *handle.Job().Data(); }

  template <class T> inline T& JobData(Type2Type<T>) 
  { return (T&)*handle.Job().Data(); }

  // Insert-AVP member functions.

  /// This function is used for setting User-Name AVP contents.
 
  virtual void
 SetUserName(DiameterScholarAttribute<diameter_utf8string_t> &userName)=0;
 
  /// This function is used for setting Destination-Realm AVP
  /// contents.  
  virtual void SetDestinationRealm
  (DiameterScholarAttribute<diameter_utf8string_t> &destinationRealm){}//=0; 

  // OriginHost & OriginRealm --> this will be populated from the config file

  virtual void SetMipMnAaaAuth
  (DiameterScholarAttribute<mip_mn_aaa_auth_info_t> &mipMnAaaAuth)=0; 

  //optional AVPs

  /// This function is used for setting Destination-Host AVP
  /// contents.  
  virtual void SetDestinationHost
  (DiameterScholarAttribute<diameter_utf8string_t> &destinationHost)
  {
  }

  virtual void SetMipMobileNodeAddress
  (DiameterScholarAttribute<diameter_address_t> &mipMobileNodeAddress)
  {
  }

  virtual void SetMipHomeAgentAddress
  (DiameterScholarAttribute<diameter_address_t> &mipHomeAgentAddress)
  {
  }

  // SetMipFeatureVector for HA Client => MN co-located mode
  virtual void SetMipFeatureVector 
  (DiameterScholarAttribute<diameter_unsigned32_t> &mipFeatureVector)
  {
  
#define FV_CO_LOCATED_MN            256
#define FV_MN_HA_KEY_REQUESTED      16

    diameter_unsigned32_t _mipFeatureVector;
    _mipFeatureVector = 0;
    _mipFeatureVector |= FV_CO_LOCATED_MN;
    _mipFeatureVector |= (IsMnHaKeyRequested() |  FV_MN_HA_KEY_REQUESTED );  
    mipFeatureVector.Set(_mipFeatureVector);
  }

  // return 1 if MN_HA keye was requested in MIP reg req
  // return 0 - otherwise
  virtual int IsMnHaKeyRequested() =0;

  //DiameterScholarAttribute<mip_originating_foreign_aaa_info_t> MipOriginatingForeignAaa;

  
  virtual void
  SetAuthorizationLifetime
  (DiameterScholarAttribute<diameter_unsigned32_t> &authorizationLifetime)
  {
  }

  // fn not needed 
  //  virtual void  SetAuthSessionState
  //(DiameterScholarAttribute<diameter_enumerated_t> &authSessionState) {  }

  virtual void SetMipHomeAgentHost
  (DiameterScholarAttribute<mip_home_agent_host_info_t> &mipHomeAgentHost)
  {
  }


  /// This function is used for setting Auth-Request-Type AVP
  /// contents.  
  virtual void SetAuthRequestType
  (DiameterScholarAttribute<diameter_enumerated_t> &authRequestType)
  {
    authRequestType = AUTH_REQUEST_TYPE_AUTHORIZE_AUTHENTICATE;
  }
 

  // Enforcement member functions.  

  /// This function is used for enforcing Authorization-Lifetime AVP
  /// contents.
  virtual void EnforceAuthorizationLifetime
  (const diameter_unsigned32_t &authorizationLifetime)
  {
  }

  /// This function is used for enforcing Auth-Session-State AVP
  /// contents.
  virtual void EnforceAuthSessionState
  (const diameter_enumerated_t &authSessionState)
  {
  }

  virtual void EnforceReAuthRequestType
  (const diameter_enumerated_t &reAuthReqType)
  {
  }

  virtual void EnforceMipMnToFaMsa 
  (const mip_mn_to_fa_msa_info_t &mipMnToFaMsa)=0;
  
  virtual void EnforceMipMnToHaMsa 
  ( const mip_mn_to_ha_msa_info_t &mipMnToHaMsa)=0;

  virtual void EnforceMipFaToMnMsa 
  ( const mip_fa_to_mn_msa_info_t &mipFaToMnMsa)=0;

  virtual void EnforceMipFaToHaMsa 
  ( const mip_fa_to_ha_msa_info_t &mipFaToHaMsa)=0;

  virtual void EnforceMipHaToMnMsa 
  ( const mip_ha_to_mn_msa_info_t &mipHaToMnMsa)=0;

  virtual void EnforceMipMsaLifetime 
  ( const diameter_unsigned32_t &mipMsaLifetime)=0;

  virtual void EnforceErrorMessage
  ( const diameter_utf8string_t &errorMessage)=0;

  virtual void SetMipCandidateHomeAgentHost
  (DiameterScholarAttribute<diameter_identity_t> &mipCandidateHomeAgentHost)
  {
  }

  virtual void SetMipHaToFaSpi
  (DiameterScholarAttribute<diameter_unsigned32_t> &mipHaToFaSpi)
  {
  }
  
  virtual AMR_Data& AMR()=0; //implemented in DiameterMip4HaClientSession class
  virtual AMA_Data& AMA()=0; //implemented in DiameterMip4HaClientSession class

  /// This virtual function is called when a AMA response is received 
  /// As part of the MIP HA application
  /// (not the diameter part), the MIP answer needs to be sent. 
  //implemented in DiameterMip4HaClientSession class
  virtual void SendMipRegReply( diameter_unsigned32_t &amaResultCode)=0;

  //implemented in DiameterMip4HaClientSession class, to reuse the session
  // object. Needs to be called in the state machine.
  virtual AAAReturnCode Reset()=0;

 protected:
 private:
  /// Inherited from AAA_EventQueueJob.
  int Schedule(AAA_Job *job, size_t=1) { return (-1); }

  /// Inherited from AAA_EventQueueJob.
  inline int Serve()
  {
    if (!AAA_EventQueueJob::ExistBacklog())
      {
	AAA_LOG((LM_ERROR, "%N: no backlog to serve."));
	return 0;
      }

    // Obtain the event to execute.
    AAA_Event ev = 0;
    AAA_EventQueueJob::Dequeue(ev);

    bool existBacklog = AAA_EventQueueJob::ExistBacklog();

    // Execute it.
    Event(ev);
    return existBacklog ? 1 : 0;
  }

  AAAClientSession &session;

  /// Job handle.
  DiameterJobHandle handle;

};

#endif //  __MIP4_HA_CLIENT_FSM_H__
