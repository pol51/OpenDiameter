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

/* $Id: */
/* 
   diameter_mip4_aaas_server_fsm.hxx
   AAA Server Server session Statemachine definition for Diameter MIP4 App. 
   Written by Miriam Tauil
   Created October 1, 2004.
*/

#ifndef __AAAS_SERVER_FSM_H__
#define __AAAS_SERVER_FSM_H__

#include "framework.h"
#include "diameter_api.h"
#include "diameter_mip4_parser.hxx"

#ifdef WIN32
   #if defined(DIAMETER_MIP4_AAAS_SERVER_EXPORT)
       #define DIAMETER_MIP4_AAAS_SERVER_EXPORTS __declspec(dllexport)
   #else
       #define DIAMETER_MIP4_AAAS_SERVER_EXPORTS __declspec(dllimport)
   #endif
#else
   #define DIAMETER_MIP4_AAAS_SERVER_EXPORTS
#endif

typedef AAA_JobHandle<AAA_GroupedJob> DiameterJobHandle;



/// State machine for Diameter MIP4 AAA server.  

class DIAMETER_MIP4_AAAS_SERVER_EXPORTS DiameterMip4AaaSServerStateMachine 
  : public AAA_StateMachine<DiameterMip4AaaSServerStateMachine>,
    public AAA_EventQueueJob
{
  friend class DiameterJobMultiplexor;

 public:
  /// Constructor.
  DiameterMip4AaaSServerStateMachine(AAAServerSession& s,
				DiameterJobHandle &h);

  virtual ~DiameterMip4AaaSServerStateMachine() 
  {
    handle.Job().Remove(this); 
  }

  enum {
    EvRxAMR,
    EvSgValidAMR,
    EvSgInvalidAMR,
    EvSgSendHAR,
    EvRxHAA,
    EvSgValidHAA,
    EvSgInvalidHAA,
    EvSgHaaNotReceived,
    EvSgAMASent,  //is it needed?
    EvSgSessionTimeout,
    EvSgAuthLifetimeTimeout,
    EvSgAuthGracePeriodTimeout,
    EvSgTimeout,
    EvSgDisconnect
  };

  /// Store an event and notify the session.
  inline void Notify(AAA_Event ev) throw (int) {
    // Enqueue the event.
    if (AAA_EventQueueJob::Enqueue(ev) <= 0)
      Abort();

    if (handle.Job().Schedule(this) < 0)
      Abort();
  }

  /// This is used for obtaining the reference to the server session
  /// object.
  inline AAAServerSession& Session() { return session; }

  /// This is used for aborting the state machine.  Usually called
  /// when Notify() fails.
  virtual void Abort()
  {
    AAA_LOG((LM_DEBUG, "Diameter AAA server session aborted.\n"));
    //Abort();  WHAT SHOULD BE DONE HERE ??
  }

  /// This is used for constructing and sending response to AMA
  void SendAMA();
  /// This is used for constructing HAR and Notifying the Client Session 
  // to send the HAR
  void SendHAR();

  /* The following virtual functions provide the state machine with information
     needed in the state machine that is provided by the specific
     implementation of the AAA Server.
     This are implemented in the AAA Server Session class by calling the 
     specificAaaServer functions.
  */

  virtual int SetMnHaNonce(diameter_octetstring_t &mnHaNonce)=0;//MnHaNonce for Mn
  virtual int SetHaMnKey( DiameterScholarAttribute<diameter_octetstring_t> &mipSessionKey)=0;//MnHaKey for HA


  virtual int SetAlgorithmType( DiameterScholarAttribute<diameter_unsigned32_t> & mipAlgorithmType)=0;
  virtual int SetReplayMode(DiameterScholarAttribute<diameter_unsigned32_t> &mipReplayMode)=0;
 
  virtual int SetAuthorizationLifetime(
	       DiameterScholarAttribute<diameter_unsigned32_t>&authLifetime)=0;

  virtual void SetAuthState( DiameterScholarAttribute<diameter_enumerated_t> &state)=0;

  virtual int SetMipMsaLifetime(
	       DiameterScholarAttribute<diameter_unsigned32_t>&mipMsaLifetime)=0;

  virtual int SetErrorMessage(diameter_utf8string_t &errorMessage)=0;

  /// Authorization function.  This function can be called from an AAAH
  /// server, so that authorization can be performed before completing
  /// authentication. 
  virtual bool AuthenticateUser(std::string UserName, 
			     diameter_address_t MipMobileNodeAddress,
			     diameter_unsigned32_t MipMnAaaSpi,
			     diameter_unsigned32_t MipAuthInputDataLength,
			     diameter_unsigned32_t MipAuthenticatorLength,
			     diameter_unsigned32_t MipAuthenticatorOffset,
			     std::string MipRegRequest )=0;

  // mnFaNonce => MIP-Mn-to-Fa-Msa
  virtual int SetMnFaNonce(diameter_octetstring_t &mnFaNonce)=0;  
  // sent to the Fa in AMA in Mip-Fa-To-Mn-Msa 
  virtual int SetMnFaKey(diameter_octetstring_t &mipSessionKey)=0;
  // sent to the Fa in AMA in Mip-Fa-To-Ha-Msa & to Ha in HAR
  virtual int SetFaHaKey(diameter_octetstring_t &mipSessionKey)=0;

  // AAA Server will allocate HA if no HA has been requested
  virtual int SetAaaSAllocatedHomeAgentHost(DiameterScholarAttribute<diameter_identity_t> &hostname)=0; //diameter_identity_t &hostname)=0;

  virtual void NotifyClientSession( int event)=0;

  inline AAA_JobData& JobData() { return *handle.Job().Data(); }

  template <class T> inline T& JobData(Type2Type<T>) 
  { return (T&)*handle.Job().Data(); }


  // implemented in aaas_server_session.hxx
  virtual AMR_Data& AMR()=0;
  virtual AMA_Data& AMA()=0;

  virtual HAR_Data& HAR()=0;
  virtual HAA_Data& HAA()=0;

 protected:

 private:
  /// Inherited from AAA_EventQueueJob.  Not used.
  int Schedule(AAA_Job*, size_t=1) { return (-1); }

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

  AAAServerSession& session;  
  /// Job handle.
  DiameterJobHandle handle;

};

#endif
