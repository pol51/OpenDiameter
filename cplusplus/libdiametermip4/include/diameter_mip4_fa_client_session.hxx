/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* OpenDiameter: Open-source software for the Diameter protocol           */
/*                                                                        */
/* Copyright (C) 2004  Open Diameter Project.                             */
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
   diameter_mip4_fa_client_session.hxx
   Fa Client Session definition for Diameter MIP 4 Application 
   Written by Miriam Tauil
   Created January 19, 2005.
*/

#ifndef __MIP4_FA_CLIENT_SESSION_H__
#define __MIP4_FA_CLIENT_SESSION_H__


#include <list>
#include "ace/Synch.h"
#include "diameter_api.h"
#include "diameter_mip4_fa_client_fsm.hxx"
#include "diameter_mip4_parser.hxx"
#include "mip4_diameter_fa_client_interface.hxx"


/// Diameter MIP FA Client session.  This class is defined as multiple
/// inheritance,  from AAAClientSession (defined in Diameter API),
/// DiameterMip4FaClientStateMachine and the abstract interface
/// Mip4DiameterFaClientInterface
template<class SpecificFaClientSession > 
class DiameterMip4FaClientSession : 
  public AAAClientSession, public DiameterMip4FaClientStateMachine, 
  public Mip4DiameterFaClientInterface
{
 public:

  class DIAMETER_MIP4_FA_CLIENT_EXPORTS AMA_Handler : 
    public AAASessionMessageHandler
  {
  public:
    AMA_Handler(AAAApplicationCore &appCore,
		DiameterMip4FaClientSession<SpecificFaClientSession>  &s) 
      : AAASessionMessageHandler(appCore, MipAmrCommandCode),
	session(s)
    {}

  private:  
    DiameterMip4FaClientSession<SpecificFaClientSession> &session;

  
    AAAReturnCode HandleMessage (AAAMessage &msg)
    {
    // Header flag check.
      if (msg.hdr.flags.r)
	{
	  AAA_LOG(LM_ERROR, "[%N] Received AMR instead of AMA.\n");
	  return AAA_ERR_UNKNOWN_CMD;
	}

      // Parse the received message.
      AMA_Parser parser;
      parser.setAppData(&session.amaData);
      parser.setRawData(&msg);

      try {
	parser.parseRawToApp();
      }
      catch ( DiameterParserError ) 
	{
	  AAA_LOG(LM_ERROR, "[%N] Payload error.\n");
	  return AAA_ERR_PARSING_ERROR;
	}

      session.Notify(DiameterMip4FaClientStateMachine::EvRxAMA);
      return AAA_ERR_SUCCESS;
    }

  };

  /// Constuctor.
  DiameterMip4FaClientSession(AAAApplicationCore &appCore): 
    AAAClientSession(appCore, Mip4ApplicationId),  
    DiameterMip4FaClientStateMachine(*this, appCore.GetTask().JobHandle()),
    specificFaClientSession(*new SpecificFaClientSession(*this)) 
  {

    answerHandler = (new AMA_Handler(appCore, *this));
    
    // Register the AMA message handler
    if (RegisterMessageHandler( answerHandler) != AAA_ERR_SUCCESS)
    {
      AAA_LOG(LM_ERROR, "[%N] AMA_Handler registration failed.\n");
      throw -1; 
    }

    // test if SpecificFaClientSession is an FaClientSession
#ifdef THIS_SHOULD_WORK_BUT_IT_DOESNT
    try {
     FaClientSession &faClientSession = 
       dynamic_cast<FaClientSession&>(specificFaClientSession);
     assert (faClientSession != NULL);

     }
     catch (bad_cast) {    
       DIAMETER_LOG(LM_ERROR, "[%N] AMA_Handler registration failed.\n");
       throw -1;
     }    
#endif
}

  /// Destructor.
  virtual ~DiameterMip4FaClientSession() 
  {
    delete (&specificFaClientSession);
    delete answerHandler;
  }


  // this is a virtual fn in the parent interface, that must be implemented 
  // here. 
  void RxMipRegReq( diameter_octetstring_t &mipRegReq) 
  {
     amrData.MipRegRequest.Set( mipRegReq); 
     Notify(  DiameterMip4FaClientStateMachine::EvRxMipRegReq); 
  }

  DiameterMip4FaClientSession* Self() { return this; }

  /// Reimplemented from AAAClientSession. This is invoked during
  /// incomming message events. The msg argument is pre-allocated by
  /// the library and is valid only within the context of this
  /// method. It is the responsibility of the derived class to
  /// override this function and capture the events if it is
  /// interested in it.
  AAAReturnCode HandleMessage(AAAMessage &msg)
  {
    AAA_LOG(LM_ERROR, "[%N] Unknown command.\n");
    return AAA_ERR_UNKNOWN_CMD;
  }

  /// Reimplemented from AAAClientSession. This is invoked during
  /// session disconnect event. Disconnection occurs when a session is
  /// terminated or the peer connection is disconnection and unable to
  /// recover. It is the responsibility of the derived class to
  /// override this function and capture this events if it is
  /// interested in it.
  AAAReturnCode HandleDisconnect()
  { 
    AAA_LOG(LM_ERROR, "[%N] Session termination event received.\n");
    Notify(DiameterMip4FaClientStateMachine::EvSgDisconnect);
    return AAA_ERR_SUCCESS; 
  }
  /// Reimplemented from AAAClientSession.
  AAAReturnCode HandleSessionTimeout()
  { 
    AAA_LOG(LM_ERROR, "[%N] Session timeout received.\n");
    Notify(DiameterMip4FaClientStateMachine::EvSgSessionTimeout);
    return AAA_ERR_SUCCESS; 
  }
                                     
  /// Reimplemented from AAAClientSession.
  AAAReturnCode HandleAuthLifetimeTimeout()
  { 
    AAA_LOG(LM_ERROR, "[%N] Timeout received.\n");
    Notify(DiameterMip4FaClientStateMachine::EvSgAuthLifetimeTimeout);
    return AAA_ERR_SUCCESS; 
  }

  /// Reimplemented from AAAClientSession.
  AAAReturnCode HandleAuthGracePeriodTimeout()
  { 
    AAA_LOG(LM_ERROR, "[%N] Timeout received.\n");
    Notify(DiameterMip4FaClientStateMachine::EvSgAuthGracePeriodTimeout);
    return AAA_ERR_SUCCESS; 
  }

  /// Reimplemented from AAAClientSession. This is invoked during
  /// session timeout event. Timeout occurs when a session is idle
  /// (not re- authorization) for a specified amount of time. It is
  /// the responsibility of the derived class to override this
  /// function and capture this events if it is interested in it.
  AAAReturnCode HandleTimeout()
  { 
    AAA_LOG(LM_ERROR, "[%N] Session timeout received.\n");
    Notify(DiameterMip4FaClientStateMachine::EvSgTimeout);
    return AAA_ERR_SUCCESS; 
  }
  /// Reimplemented from AAAClientSession. This is invoked during
  /// session abort event. Abort occurs when the server decides to
  /// terminate the client session by sending an ASR. It is the
  /// responsibility of the derived class to override this function
  /// and capture this events if it is interested in it.
  AAAReturnCode HandleAbort() { return AAA_ERR_SUCCESS; }

  void Start() throw (AAA_Error)
  {
    DiameterMip4FaClientStateMachine::Start();
    AAAClientSession::Start();
  }

  AAAReturnCode Reset() throw (AAA_Error)
  {    
    AAAClientSession::Start();
    amrData.Clear(); 
    amaData.Clear();
    //sets the session state to StInitialize
    DiameterMip4FaClientStateMachine::Restart();
   
    specificFaClientSession.Reset();

    return AAA_ERR_SUCCESS;
  }

  // The implementation of this function in a child class is optional.
  // The function's purpose is to provide an interface for memory deallocation
  // of the DiameterMip4FaClientSession, when it is allocated using the "new"
  // operator. More documentation in the sample application file.
  virtual void Abort(){}

 void SetUserName(AAA_ScholarAttribute<diameter_utf8string_t> &userName)
  {
    diameter_utf8string_t _userName;
    specificFaClientSession.SetUserName( _userName);
    userName.Set( _userName);
  }

  /// This function is used for setting Destination-Realm AVP
  /// contents.  
  virtual void SetDestinationRealm
  (AAA_ScholarAttribute<diameter_utf8string_t> &destinationRealm)
  {
    diameter_utf8string_t _destinationRealm;
    specificFaClientSession.SetDestinationRealm( _destinationRealm);
    destinationRealm.Set( _destinationRealm);
  }
  // OriginHost & OriginRealm --> this will be populated from the config file
  //  MipRegRequest will be set by the fn RxMipregReq()

  void SetMipMnAaaAuth
    (AAA_ScholarAttribute<mip_mn_aaa_auth_info_t> &mipMnAaaAuth)
  {

    AAA_ScholarAttribute<mip_mn_aaa_auth_info_t> _mipMnAaaAuth; 
    
    specificFaClientSession.SetMipMnAaaSpi( 
			 &( _mipMnAaaAuth().MipMnAaaSpi() ));
    specificFaClientSession.SetMipAuthInputDataLength( 
    			&(_mipMnAaaAuth().MipAuthInputDataLength() ));
    specificFaClientSession.SetMipAuthenticatorLength( 
			&( _mipMnAaaAuth().MipAuthenticatorLength() ));
    specificFaClientSession.SetMipAuthenticatorOffset( 
			&( _mipMnAaaAuth().MipAuthenticatorOffset() ));


    mipMnAaaAuth.Set(  _mipMnAaaAuth());

    mipMnAaaAuth().MipMnAaaSpi = _mipMnAaaAuth().MipMnAaaSpi();

    mipMnAaaAuth().MipAuthInputDataLength =
				     _mipMnAaaAuth().MipAuthInputDataLength();
    mipMnAaaAuth().MipAuthenticatorLength =
	    		             _mipMnAaaAuth().MipAuthenticatorLength();
    mipMnAaaAuth().MipAuthenticatorOffset =
				     _mipMnAaaAuth().MipAuthenticatorOffset();

  }

  //optional AVPs

  /// This function is used for setting Destination-Host AVP
  /// contents.  
  void SetDestinationHost//( diameter_utf8string_t  _destinationHost)
  (AAA_ScholarAttribute<diameter_utf8string_t> &destinationHost)
  {
    diameter_utf8string_t _destinationHost;
    specificFaClientSession.SetDestinationHost( _destinationHost);
    destinationHost.Set(_destinationHost);
  }

  void SetMipMobileNodeAddress
  (AAA_ScholarAttribute<diameter_address_t> &mipMobileNodeAddress)
  {
    diameter_address_t _mipMobileNodeAddress;
    specificFaClientSession.SetMipMobileNodeAddress(_mipMobileNodeAddress);
    mipMobileNodeAddress.Set(_mipMobileNodeAddress);
  }

  void SetMipHomeAgentAddress
  (AAA_ScholarAttribute<diameter_address_t> &mipHomeAgentAddress)
  {
    diameter_address_t _mipHomeAgentAddress;
    specificFaClientSession.SetMipHomeAgentAddress(_mipHomeAgentAddress);
    mipHomeAgentAddress.Set(_mipHomeAgentAddress);
  }

  int IsMnHaKeyRequested()
  {
    return ( specificFaClientSession.IsMnHaKeyRequested() );
  }

  int IsMnFaKeyRequested() 
  {
    return ( specificFaClientSession.IsMnFaKeyRequested() );
  }
  int IsFaHaKeyRequested() 
  {
    return ( specificFaClientSession.IsFaHaKeyRequested() );
  }
  int IsMnHomeAddrRequested()
  {
    return ( specificFaClientSession.IsMnHomeAddrRequested() );
  }
  int IsMnHomeAgentRequested()
  {
    return ( specificFaClientSession.IsMnHomeAgentRequested() );
  }

  void SetAuthorizationLifetime
  (AAA_ScholarAttribute<diameter_unsigned32_t> &authorizationLifetime)
  {
    diameter_unsigned32_t _authorizationLifetime;
    if ( specificFaClientSession.SetAuthorizationLifetime(
					  &_authorizationLifetime) == 0 )
	authorizationLifetime.Set(0);
    else
        authorizationLifetime.Set(_authorizationLifetime);
  }

// fn not needed=> static info take from configuration file
// void SetAuthSessionState
//  (AAA_ScholarAttribute<diameter_enumerated_t> &authSessionState) {}

  //int 
  void SetMipHomeAgentHost
  (AAA_ScholarAttribute<mip_home_agent_host_info_t> &mipHomeAgentHost)
  {
    //mip_home_agent_host_info_t 
    diameter_identity_t _mipHomeAgentHost;
    if (specificFaClientSession.SetMipHomeAgentHost(_mipHomeAgentHost)==1)
      {
	// convert ip address from string to unsigned32 ???
	//mipHomeAgentHost.DestinationHost.Set(_mipHomeAgentHost);
      //return 1;
      }
    //else
    //return 0;
  }

  virtual int SetMipFaChallenge
  (AAA_ScholarAttribute<diameter_octetstring_t> &mipFaChallenge)
  {
    diameter_octetstring_t _mipFaChallenge;

    int rc = specificFaClientSession.SetMipFaChallenge(  _mipFaChallenge);
    mipFaChallenge.Set(_mipFaChallenge);
    return rc;
  }

  virtual void SetMipCandidateHomeAgentHost
  (AAA_ScholarAttribute<diameter_identity_t> &mipCandidateHomeAgentHost)
  {}

  virtual int SetMipHaToFaSpi
  (AAA_ScholarAttribute<diameter_unsigned32_t> &mipHaToFaSpi)
  {
    diameter_unsigned32_t _mipHaToFaSpi;
    if ( specificFaClientSession.SetMipHaToFaSpi( &_mipHaToFaSpi) == 0)
      {
	mipHaToFaSpi.Set(0);
	return 0;
      }
    else
      {
      mipHaToFaSpi.Set( _mipHaToFaSpi);
      return 1;
      }
  }

  void EnforceAuthorizationLifetime
                     (const diameter_unsigned32_t &authorizationLifetime)
  {
   specificFaClientSession.EnforceAuthorizationLifetime(authorizationLifetime);
  }

  void EnforceAuthSessionState( const diameter_enumerated_t &authSessionState)
  {
    specificFaClientSession.EnforceAuthSessionState( authSessionState);
  }

  void EnforceReAuthRequestType( const diameter_enumerated_t &reAuthReqType)
  {
    specificFaClientSession.EnforceReAuthRequestType( reAuthReqType);
  }

  void EnforceMipMnToFaMsa(const mip_mn_to_fa_msa_info_t &mipMnToFaMsa)
  {
    specificFaClientSession.EnforceMipMnToFaMsa( mipMnToFaMsa);
  }
  
  void EnforceMipMnToHaMsa( const mip_mn_to_ha_msa_info_t &mipMnToHaMsa)
  {
    specificFaClientSession.EnforceMipMnToHaMsa( mipMnToHaMsa);
  }

  void EnforceMipFaToMnMsa( const mip_fa_to_mn_msa_info_t &mipFaToMnMsa)
  {
    specificFaClientSession.EnforceMipFaToMnMsa( mipFaToMnMsa);
  }

  void EnforceMipFaToHaMsa( const mip_fa_to_ha_msa_info_t &mipFaToHaMsa)
  {
    specificFaClientSession.EnforceMipFaToHaMsa( mipFaToHaMsa);
  }

  void EnforceMipHaToMnMsa( const mip_ha_to_mn_msa_info_t &mipHaToMnMsa)
  {
    specificFaClientSession.EnforceMipHaToMnMsa( mipHaToMnMsa);
  }

  void EnforceMipMsaLifetime( const diameter_unsigned32_t &mipMsaLifetime)
  {
    specificFaClientSession.EnforceMipMsaLifetime( mipMsaLifetime);
  }

  void EnforceErrorMessage( const diameter_utf8string_t &errorMessage)
  {
    specificFaClientSession.EnforceErrorMessage( errorMessage);
  }

  void EnforceMipFilterRule ( 
      const  AAA_VectorAttribute<diameter_ipfilter_rule_t> &mipFilterRule)
     //const diameter_ipfilter_rule_t &mipFilterRule)
  {
    specificFaClientSession.EnforceMipFilterRule( mipFilterRule);
  }

  void SendMipRegReply(diameter_unsigned32_t &amaResultCode)
  {
    specificFaClientSession.SendMipRegReply( amaResultCode);
  }


  void SendMipRegReply( diameter_unsigned32_t &amaResultCode,
				diameter_octetstring_t &mipRegReply)
  {
    specificFaClientSession.SendMipRegReply( amaResultCode, mipRegReply);
  }


  AMR_Data& AMR() { return amrData; }
  AMA_Data& AMA() { return amaData; }

  SpecificFaClientSession &specificFaClientSession;

 protected:
 private:
  
  AMR_Data amrData;
  AMA_Data amaData;
  AMA_Handler *answerHandler;
 
};


#endif  // __MIP4_FA_CLIENT_SESSION_H__
