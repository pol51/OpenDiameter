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

// diameter_mip4_aaas_server_fsm.cxx
// Written by Miriam Tauil
// October 8th, 2004

#include "diameter_mip4_aaas_server_session.hxx"
#include "diameter_mip4_aaas_server_fsm.hxx"
#include "diameter_mip4_parser.hxx"

class DiameterMip4AaaSServerAction 
  : public AAA_Action<DiameterMip4AaaSServerStateMachine>
{
  virtual void operator()(DiameterMip4AaaSServerStateMachine&)=0;
 protected:
  DiameterMip4AaaSServerAction() {}
  ~DiameterMip4AaaSServerAction() {}
};

/// State table used by DiameterMip4AaaSServerStateMachine.
class DiameterMip4AaaSServerStateTable_S 
  : public AAA_StateTable<DiameterMip4AaaSServerStateMachine>
{
  friend class 
  ACE_Singleton<DiameterMip4AaaSServerStateTable_S, ACE_Recursive_Thread_Mutex>;
 private:
  class AcCheckAMR : public DiameterMip4AaaSServerAction 
  {
    void operator()(DiameterMip4AaaSServerStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, 
		   "[%N] Check AMR authentication.\n");
      
      int rc = sm.AuthenticateUser( sm.AMR().UserName(), 
		    sm.AMR().MipMobileNodeAddress(),
		    sm.AMR().MipMnAaaAuth().MipMnAaaSpi(),
		    sm.AMR().MipMnAaaAuth().MipAuthInputDataLength(),
		    sm.AMR().MipMnAaaAuth().MipAuthenticatorLength(),
		    sm.AMR().MipMnAaaAuth().MipAuthenticatorOffset(),
		    sm.AMR().MipRegRequest() );

#ifdef PRINT_MSG_CONTENT
  printf("AMR:\nAuthorization Lifetime: %d\nMip-Reg-Request: %s\nUser Name: %s\nDestination Realm: %s\nDestination Host: %s\n\n",
	 sm.AMR().AuthorizationLifetime(), 
	 sm.AMR().MipRegRequest().data(), 
	 sm.AMR().UserName().data(),
	 sm.AMR().DestinationRealm().data(),
	 sm.AMR().DestinationHost().data() );  
#endif

      if (rc ==1 )
	if (MipFeatureVectorOperations::IsFeatureVectorSet( 
	   sm.AMR().MipFeatureVector(), FV_CO_LOCATED_MN_BIT ))
	  sm.Event(DiameterMip4AaaSServerStateMachine::EvSgValidAMR);
        else
	  sm.Event(DiameterMip4AaaSServerStateMachine::EvSgSendHAR);
      else
	sm.Event(DiameterMip4AaaSServerStateMachine::EvSgInvalidAMR);
    }
  };

  // This case relates to MN in co-located mode:  reply to HA request
  class AcSendAMA_DueToValidAMR : public DiameterMip4AaaSServerAction 
  {
    void operator()(DiameterMip4AaaSServerStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] Send positive AMA.\n");
      // set Result-Code + other AVPs

      AMR_Data& amr = sm.AMR();
      AMA_Data& ama = sm.AMA();
      ama.Clear();
      ama.AuthApplicationId = Mip4ApplicationId;
      
      if (amr.AcctMultiSessionId.IsSet())
	{
	  ama.AcctMultiSessionId.Set( amr.AcctMultiSessionId() );
	}
      if (amr.UserName.IsSet())
	{
	  ama.UserName.Set( amr.UserName() );
	}
      if (amr.MipFeatureVector.IsSet())
	{
	  ama.MipFeatureVector.Set( amr.MipFeatureVector() );
	}
      // AAAS set value to STATE_MAINTAINED (0) or NO_STATE_MAINTAINED (1)
      // value is set automatically by the library according value in the config file.
      //sm.SetAuthState( ama.AuthSessionState );

      // ama.ReAuthRequestType - probably not needed Mip app

      //populate the AMA fields uniquely for this success case
      // other fields from AMR to AMA will be populated in SendAMA()

      ama.ResultCode = AAA_SUCCESS ;  
      
      if ( !sm.SetAuthorizationLifetime( ama.AuthorizationLifetime ) )
	ama.AuthorizationLifetime.Set( amr.AuthorizationLifetime() );
      
      if (MipFeatureVectorOperations::IsFeatureVectorSet( 
      		  amr.MipFeatureVector(), FV_MN_HA_KEY_REQUESTED) )
	{	
	  if ( sm.SetMnHaNonce( ama.MipMnToHaMsa().MipNonce()) )//MnHaNonce->Mn
	    {
	      sm.SetAlgorithmType( ama.MipMnToHaMsa().MipAlgorithmType);
	      sm.SetReplayMode( ama.MipMnToHaMsa().MipReplayMode);
	      //ama.MipMnToHaMsa.MipMnAaaSpi-this needs to be set by MN -> HA 
	      // info is not needed in MipMnToHaMsa
	    }
	  if ( sm.SetHaMnKey( ama.MipHaToMnMsa().MipSessionKey ) )//MnHaKey-> HA
	    {
	      sm.SetAlgorithmType( ama.MipHaToMnMsa().MipAlgorithmType);
	      sm.SetReplayMode( ama.MipHaToMnMsa().MipReplayMode);
	    }

	  sm.SetMipMsaLifetime( ama.MipMsaLifetime);
      }
      // These 2 must have been set in the AMR in MN co-located mode
      if (amr.MipHomeAgentAddress.IsSet())
	{
	  ama.MipHomeAgentAddress.Set( amr.MipHomeAgentAddress() );
	}
      if (amr.MipMobileNodeAddress.IsSet())
	{
	  ama.MipMobileNodeAddress.Set( amr.MipMobileNodeAddress() );
	}

      sm.SendAMA();

      sm.Session().Update(AAASession::EVENT_AUTH_SUCCESS);
       //sm.Event( DiameterMip4AaaSServerStateMachine::EvSgAMASent);
    }
  };
    // reply to FA/HA request
  class AcSendAMA_DueToInvalidAMR : public DiameterMip4AaaSServerAction 
  {
    void operator()(DiameterMip4AaaSServerStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] Sending AMA with Auth Rejected result-code.\n");
      
      AMR_Data& amr = sm.AMR();
      AMA_Data& ama = sm.AMA();
      ama.Clear();
      ama.AuthApplicationId = Mip4ApplicationId;

      if (amr.AcctMultiSessionId.IsSet())
	{
	  ama.AcctMultiSessionId.Set( amr.AcctMultiSessionId() );
	}
      if (amr.UserName.IsSet())
	{
	  ama.UserName.Set( amr.UserName() );
	}
      if (amr.MipFeatureVector.IsSet())
	{
	  ama.MipFeatureVector.Set( amr.MipFeatureVector() );
	}

      // AAAS set value to STATE_MAINTAINED (0) or NO_STATE_MAINTAINED (1)
      sm.SetAuthState( ama.AuthSessionState );

      // ama.ReAuthRequestType - probably not needed Mip app
      ama.ResultCode = AAA_AUTHENTICATION_REJECTED;

      if ( ! sm.SetErrorMessage( ama.ErrorMessage() ) )
	ama.ErrorMessage.Set("Mobile Node failed authentication\n");
      ama.ErrorReportingHost.Set ( amr.DestinationHost()); // this aaas host
 
      sm.SendAMA(); 

      // Update the session state.
      sm.Session().Update(AAASession::EVENT_AUTH_FAILED);
    }
  };

  class AcSendHAR : public DiameterMip4AaaSServerAction 
  {
    void operator()(DiameterMip4AaaSServerStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] Constructing and Sending HAR.\n");

      // populate HAR attributes and trigger the clientSession to send HAR
      sm.SendHAR();  

      // there is no appropiate event to update the session state
    }
  };


  class AcSendAMA_DueToHAAFailure : public DiameterMip4AaaSServerAction 
  {
    void operator()(DiameterMip4AaaSServerStateMachine& sm)
    {

      AAA_LOG(LM_DEBUG, 
		   "[%N] Sending AMA due to HAA authorization failure.\n");

      AMR_Data& amr = sm.AMR();
      AMA_Data& ama = sm.AMA();
      ama.Clear();

      ama.AuthApplicationId = Mip4ApplicationId;

      if (amr.AcctMultiSessionId.IsSet())
	{
	  ama.AcctMultiSessionId.Set( amr.AcctMultiSessionId() );
	}
      if (amr.UserName.IsSet())
	{
	  ama.UserName.Set( amr.UserName() );
	}
      if (amr.MipFeatureVector.IsSet())
	{
	  ama.MipFeatureVector.Set( amr.MipFeatureVector() );
	}


      // AAAS set value to STATE_MAINTAINED (0) or NO_STATE_MAINTAINED (1)
      sm.SetAuthState( ama.AuthSessionState );

      // ama.ReAuthRequestType - probably not needed Mip app

      if ( ! sm.SetErrorMessage( ama.ErrorMessage() ) )
	ama.ErrorMessage.Set("Mobile Node failed authorization\n");
      ama.ErrorReportingHost.Set ( amr.DestinationHost()); // this aaas host


      ama.ResultCode = AAA_AUTHORIZATION_REJECTED;
      sm.SendAMA(); 
      sm.Session().Update(AAASession::EVENT_AUTH_FAILED);
    }
  };

  class AcSendAMA_DueToHaaNotReceived : public DiameterMip4AaaSServerAction 
  {
    void operator()(DiameterMip4AaaSServerStateMachine& sm)
    {

      AAA_LOG(LM_DEBUG, 
		   "[%N] Sending AMA due to HAA authorization failure.\n");

      AMR_Data& amr = sm.AMR();
      AMA_Data& ama = sm.AMA();
      ama.Clear();

      ama.AuthApplicationId = Mip4ApplicationId;

      if (amr.AcctMultiSessionId.IsSet())
	{
	  ama.AcctMultiSessionId.Set( amr.AcctMultiSessionId() );
	}
      if (amr.UserName.IsSet())
	{
	  ama.UserName.Set( amr.UserName() );
	}
      if (amr.MipFeatureVector.IsSet())
	{
	  ama.MipFeatureVector.Set( amr.MipFeatureVector() );
	}

      // AAAS set value to STATE_MAINTAINED (0) or NO_STATE_MAINTAINED (1)
      sm.SetAuthState( ama.AuthSessionState );

      // ama.ReAuthRequestType - probably not needed Mip app

      if ( ! sm.SetErrorMessage( ama.ErrorMessage() ) )
	ama.ErrorMessage.Set("HAA not received\n");
      ama.ErrorReportingHost.Set ( amr.DestinationHost()); // this aaas host

      ama.ResultCode =AAA_UNABLE_TO_DELIVER;
      sm.SendAMA(); 
      sm.Session().Update(AAASession::EVENT_AUTH_FAILED);
    }
  };
    // reply to FA request
  class AcSendAMA_DueToHAASuccess : public DiameterMip4AaaSServerAction 
  {
    void operator()(DiameterMip4AaaSServerStateMachine& sm)
    {

      AAA_LOG(LM_DEBUG, "[%N] Sending AMA due to successful HAA.\n");

      AMR_Data& amr = sm.AMR();
      HAA_Data& haa = sm.HAA();
      AMA_Data& ama = sm.AMA();

      ama.Clear();
      ama.AuthApplicationId = Mip4ApplicationId;
      
      if (haa.AcctMultiSessionId.IsSet())
	{
	  ama.AcctMultiSessionId.Set( haa.AcctMultiSessionId() );
	}
      if (amr.UserName.IsSet())
	{
	  ama.UserName.Set( amr.UserName() );
	}
      
      if ( !sm.SetAuthorizationLifetime( ama.AuthorizationLifetime ) )
	ama.AuthorizationLifetime.Set( amr.AuthorizationLifetime() );

      if (amr.MipFeatureVector.IsSet())
	{
	  ama.MipFeatureVector.Set( amr.MipFeatureVector() );
	}

      // AAAS set value to STATE_MAINTAINED (0) or NO_STATE_MAINTAINED (1)
      sm.SetAuthState( ama.AuthSessionState );

      // ama.ReAuthRequestType - probably not needed Mip app

      //MipRegReply from HAR
      if (haa.MipRegReply.IsSet())
	{
	  ama.MipRegReply.Set( haa.MipRegReply() );
	}

      //MIP-MN-To-FA-MSA
      //MIP-FA-to-mn-MSA
      if ( MipFeatureVectorOperations::IsFeatureVectorSet( 
      			   amr.MipFeatureVector(), FV_MN_FA_KEY_REQUESTED) )

      {
	if ( sm.SetMnFaNonce(ama.MipMnToFaMsa().MipNonce()) )//MnFaNonce-> Mn
	  {
	    sm.SetAlgorithmType( ama.MipMnToFaMsa().MipAlgorithmType);
	    //ama.MipMnToFaMsa.MipMnAaaSpi-this needs to be set by MN for FA 
	    // info is not needed in MipMnToFaMsa HERE
	  }
		
	if ( sm.SetMnFaKey( ama.MipFaToMnMsa().MipSessionKey() ) )//MnHaKey->FA
	  {
	    sm.SetAlgorithmType( ama.MipFaToMnMsa().MipAlgorithmType);
	    ama.MipFaToMnMsa().MipFaToMnSpi.Set( haa.MipFaToMnSpi() );	
	  }
      }

      //MIP-FA-to-HA-MSA
      if ( MipFeatureVectorOperations::IsFeatureVectorSet( 
      			   amr.MipFeatureVector(), FV_FA_HA_KEY_REQUESTED) )
      {
	if ( sm.SetFaHaKey( ama.MipFaToHaMsa().MipSessionKey() ) )//MnHaKey->HA
	  {
	    sm.SetAlgorithmType( ama.MipFaToHaMsa().MipAlgorithmType);
	    ama.MipFaToHaMsa().MipFaToHaSpi.Set( haa.MipFaToHaSpi() );
	  }
      }

      sm.SetMipMsaLifetime( ama.MipMsaLifetime);

      // from HAA:  MIP-HA-Address + MIP-MN-Address
      ama.MipHomeAgentAddress.Set( haa.MipHomeAgentAddress() );

      ama.MipMobileNodeAddress.Set( haa.MipMobileNodeAddress() );
       

      ama.ResultCode = AAA_SUCCESS;
      sm.SendAMA(); 
      sm.Session().Update(AAASession::EVENT_AUTH_SUCCESS);
    }
  };

  enum state {
    StInitialize,
    StCheckAMR,
    StAccepted,          // ValidAMR & no need for HAA, Valid HAA
    StRejected,          //InvalidAMR, Invalid HAA
    StWaitHAA,
    StTerminated
  };

  AcCheckAMR acCheckAMR; 
  AcSendAMA_DueToValidAMR acSendAMA_DueToValidAMR;
  AcSendAMA_DueToInvalidAMR acSendAMA_DueToInvalidAMR;
  AcSendHAR acSendHAR;
  AcSendAMA_DueToHAASuccess acSendAMA_DueToHAASuccess;
  AcSendAMA_DueToHAAFailure acSendAMA_DueToHAAFailure;
  AcSendAMA_DueToHaaNotReceived acSendAMA_DueToHaaNotReceived;
  

  // Defined as a leaf class
  DiameterMip4AaaSServerStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       DiameterMip4AaaSServerStateMachine::EvRxAMR,
		       StCheckAMR, acCheckAMR);
    AddStateTableEntry(StInitialize, 
		       DiameterMip4AaaSServerStateMachine::EvSgDisconnect, 
		       StTerminated);
    AddWildcardStateTableEntry(StInitialize, StTerminated);

    AddStateTableEntry(StCheckAMR, 
		       DiameterMip4AaaSServerStateMachine::EvSgValidAMR, 
		       StAccepted,
		       acSendAMA_DueToValidAMR); 
    AddStateTableEntry(StCheckAMR, 
		       DiameterMip4AaaSServerStateMachine::EvSgInvalidAMR, 
		       StRejected, acSendAMA_DueToInvalidAMR); 
    AddStateTableEntry(StCheckAMR, 
		       DiameterMip4AaaSServerStateMachine::EvSgSendHAR, 
		       StWaitHAA, acSendHAR); 

    AddStateTableEntry(StWaitHAA,
		       DiameterMip4AaaSServerStateMachine::EvSgValidHAA, 
		       StAccepted, acSendAMA_DueToHAASuccess);
    AddStateTableEntry(StWaitHAA,
		       DiameterMip4AaaSServerStateMachine::EvSgInvalidHAA,
		       StRejected, acSendAMA_DueToHAAFailure);
    AddStateTableEntry(StWaitHAA,
                      DiameterMip4AaaSServerStateMachine::EvSgHaaNotReceived,
                      StRejected, acSendAMA_DueToHaaNotReceived);
     AddStateTableEntry(StWaitHAA,
                      DiameterMip4AaaSServerStateMachine::EvSgSessionTimeout,
                      StRejected, acSendAMA_DueToHaaNotReceived);

    AddWildcardStateTableEntry(StWaitHAA, StTerminated);


    AddStateTableEntry(StAccepted,
		       DiameterMip4AaaSServerStateMachine::EvSgDisconnect,
		       StTerminated);


    AddWildcardStateTableEntry(StAccepted, StTerminated);

    AddWildcardStateTableEntry(StRejected, StRejected);

    AddWildcardStateTableEntry(StTerminated, StTerminated);

    InitialState(StInitialize);
  }
  ~DiameterMip4AaaSServerStateTable_S() {}
};

typedef 
ACE_Singleton<DiameterMip4AaaSServerStateTable_S, ACE_Recursive_Thread_Mutex> 
DiameterMip4AaaSServerStateTable;

DiameterMip4AaaSServerStateMachine::DiameterMip4AaaSServerStateMachine
(AAAServerSession& s, DiameterJobHandle &h)
  : AAA_StateMachine<DiameterMip4AaaSServerStateMachine>
  (*this, *DiameterMip4AaaSServerStateTable::instance(), "MIP4_AAAS_SERVER"),
    session(s),
    handle(h) 
{
  AAA_StateMachine<DiameterMip4AaaSServerStateMachine>::Start();
}

void 
DiameterMip4AaaSServerStateMachine::SendAMA()
{
  AAAMessage msg;

  AMA_Data amaData= AMA();
  
  AMA_Parser parser;
  parser.setAppData(&amaData);
  parser.setRawData(&msg);

  try {
    parser.parseAppToRaw();
  }
  catch (DiameterParserError) {
    AAA_LOG(LM_ERROR, "[%N] Parsing error.\n");
    return;
  }
  AAAServerSession &session = Session();
  
  AAAMessageControl msgControl( &session );  
  if (msgControl.Send(msg) != AAA_ERR_SUCCESS) {
    AAA_LOG(LM_ERROR, "Failed sending message.\n");
  }
  else {
    AAA_LOG(LM_DEBUG, "Sent AMA Message.\n");
  }
}

void 
DiameterMip4AaaSServerStateMachine::SendHAR()
{
  AAAMessage msg;
 
  AMR_Data &amrData= AMR();
  HAR_Data &harData= HAR();
  harData.AuthApplicationId = Mip4ApplicationId;

  //populate fields from AMR to HAR
  
  if (  ! SetAuthorizationLifetime( harData.AuthorizationLifetime )) 
    harData.AuthorizationLifetime.Set( amrData.AuthorizationLifetime());
  SetAuthState( harData.AuthSessionState); 

  harData.MipRegRequest.Set(amrData.MipRegRequest());
  // OriginHost. OriginRealm - set by msgControl class

  if ( amrData.UserName.IsSet() )
    harData.UserName.Set( amrData.UserName() ); //if present is copied 

  harData.DestinationRealm.Set( amrData.DestinationRealm());// HA in the same domain
    
  harData.MipFeatureVector.Set( amrData.MipFeatureVector());

  // Optional AVPs

  // Desination Host: requested HA or AAAS assigned HA: 
  //if ( amrData.MipHomeAgentAddress().IsSet() )
      // convert from Address -> Host
      // otherwise => AAAS needs to allocate HA in the realm
  SetAaaSAllocatedHomeAgentHost( harData.DestinationHost );

  //check what keys are requested in amrData.MipFeatureVector, and request
  // these keys from the AAAS -> assign the values 

  //MipMnToFaMsa.
  if ( MipFeatureVectorOperations::IsFeatureVectorSet( amrData.MipFeatureVector, FV_MN_FA_KEY_REQUESTED))
    {
      SetAlgorithmType( harData.MipMnToFaMsa().MipAlgorithmType);
      SetMnFaNonce( harData.MipMnToFaMsa().MipNonce() );
      // Is the following field goes only on the HAA optionaly
      harData.MipMnToFaMsa().MipMnAaaSpi.Set( amrData.MipMnAaaAuth().MipMnAaaSpi() );
    }

  //MipMnToHaMsa + MipHaToMnMsa 
  if ( MipFeatureVectorOperations::IsFeatureVectorSet( amrData.MipFeatureVector, FV_MN_HA_KEY_REQUESTED))
    {
      harData.MipMnToHaMsa().MipMnAaaSpi.Set( amrData.MipHaToFaSpi() );
      SetMnHaNonce ( harData.MipMnToHaMsa().MipNonce() );
      SetAlgorithmType( harData.MipMnToHaMsa().MipAlgorithmType);
      SetReplayMode( harData.MipMnToHaMsa().MipReplayMode);

      SetAlgorithmType( harData.MipHaToMnMsa().MipAlgorithmType);
      SetReplayMode( harData.MipHaToMnMsa().MipReplayMode);
      SetHaMnKey( harData.MipHaToMnMsa().MipSessionKey );
    }

  //MipHaToFaMsa.
  if ( MipFeatureVectorOperations::IsFeatureVectorSet( 
       amrData.MipFeatureVector, FV_FA_HA_KEY_REQUESTED))
    {
      SetAlgorithmType( harData.MipHaToFaMsa().MipAlgorithmType);
      harData.MipHaToFaMsa().MipHaToFaSpi.Set( amrData.MipHaToFaSpi() );
      SetFaHaKey( harData.MipHaToFaMsa().MipSessionKey() );
    }

  // Set by AAA Server
  SetMipMsaLifetime( harData.MipMsaLifetime );

  if ( amrData.MipOriginatingForeignAaa.IsSet() )
    harData.MipOriginatingForeignAaa.Set( amrData.MipOriginatingForeignAaa());

  if ( amrData.MipMobileNodeAddress.IsSet() )
    harData.MipMobileNodeAddress.Set( amrData.MipMobileNodeAddress() );
  if ( amrData.MipHomeAgentAddress.IsSet() )
    harData.MipHomeAgentAddress.Set( amrData.MipHomeAgentAddress() );

  // harData.MipFilterRule.


  // printed in the aaas client session
  //  printf("HAR:\nAuthorization Lifetime: %d\nMip-Reg-Request: %s\nUser Name: %s\nDestination Realm: %s\nDestination Host: %s\n",
  //	 harData.AuthorizationLifetime(), 
  //	 harData.MipRegRequest().data(), 
  //	 harData.UserName().data(),
  //	 harData.DestinationRealm().data(),
  //	 harData.DestinationHost().data() );  

  // trigger SendHAR event in the AAAS client session
  NotifyClientSession( DiameterMip4AaaSClientStateMachine::EvSendHAR);

}



