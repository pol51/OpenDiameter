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

// diameter_mip4_ha_server_fsm.cxx
// Written by Miriam Tauil
// December 20, 2004

#include "diameter_mip4_ha_server_session.hxx"
#include "diameter_mip4_ha_server_fsm.hxx"
#include "diameter_mip4_parser.hxx"

class DiameterMip4HaServerAction 
  : public AAA_Action<DiameterMip4HaServerStateMachine>
{
  virtual void operator()(DiameterMip4HaServerStateMachine&)=0;
 protected:
  DiameterMip4HaServerAction() {}
  ~DiameterMip4HaServerAction() {}
};

/// State table used by DiameterMip4HaServerStateMachine.
class DiameterMip4HaServerStateTable_S 
  : public AAA_StateTable<DiameterMip4HaServerStateMachine>
{
  friend class 
  ACE_Singleton<DiameterMip4HaServerStateTable_S, ACE_Recursive_Thread_Mutex>;
 private:
  class AcProcessMipRegReq : public DiameterMip4HaServerAction 
  {
    void operator()(DiameterMip4HaServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
		   "[%N] Check HAR ....\n"));

      HAR_Data& har = sm.HAR();
      HAA_Data& haa = sm.HAA();
      haa.Clear();
      haa.AuthApplicationId.Set(Mip4ApplicationId);

      sm.SetAcctMultiSessionId( haa.AcctMultiSessionId );
      
      if (har.UserName.IsSet())
	{
	  haa.UserName.Set( har.UserName() );
	}

      if (har.MipHomeAgentAddress.IsSet())
	haa.MipHomeAgentAddress.Set( har.MipHomeAgentAddress() );

      if (har.MipMobileNodeAddress.IsSet())
	haa.MipMobileNodeAddress.Set( har.MipMobileNodeAddress());

      // return 1 -  mip reg request accepted
      // return 2 -  mip reg request rejected
      // return 3 -  invalid mip reg request 
      // else error
      int rc = sm.ProcessMipRegRequest( har.MipRegRequest() );
      if (rc ==1 )
	sm.Event(DiameterMip4HaServerStateMachine::EvSgRegRequestAccepted);
      else if (rc == 2 )
        sm.Event(DiameterMip4HaServerStateMachine::EvSgRegRequestRejected);
      else if (rc == 3 )
        sm.Event(DiameterMip4HaServerStateMachine::EvSgInvalidRegRequest);
      else //error
        sm.Event(DiameterMip4HaServerStateMachine::EvSgDisconnect);
    }
  };

  // respond with SUCCESS
  class  AcSendHAA_DueToRegRequestAccepted : public DiameterMip4HaServerAction 
  {
    void operator()(DiameterMip4HaServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "[%N] Send positive HAA.\n"));

      HAR_Data& har = sm.HAR();
      HAA_Data& haa = sm.HAA();

      // set Result-Code + other AVPs
      //populate the HAA fields uniquely for this success case

      sm.SetMipRegReply( haa.MipRegReply );
      haa.ResultCode.Set( AAA_SUCCESS);        

      if (MipFeatureVectorOperations::IsFeatureVectorSet( 
      		  har.MipFeatureVector(), FV_FA_HA_KEY_REQUESTED) )
       	sm.SetMipFaToHaSpi( haa.MipFaToHaSpi() );

      if (MipFeatureVectorOperations::IsFeatureVectorSet( 
      		  har.MipFeatureVector(), FV_MN_FA_KEY_REQUESTED) )
	sm.SetMipFaToMnSpi( haa.MipFaToMnSpi() );


      // These must have been set in the AMR in MN co-located mode
  
      if (har.MipMobileNodeAddress.IsSet())
	{
	  haa.MipMobileNodeAddress.Set( har.MipMobileNodeAddress() );
	}
      else
	sm.SetMipMnAddress( haa.MipMobileNodeAddress);

      sm.SendHAA();

      sm.Session().Update(AAASession::EVENT_AUTH_SUCCESS);
       //sm.Event( DiameterMip4HaServerStateMachine::EvSgHAASent);
    }
  };
    
  class AcSendHAA_DueToRegRequestRejected : public DiameterMip4HaServerAction 
  {
    void operator()(DiameterMip4HaServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "[%N] Sending HAA with Auth Rejected result-code.\n"));
      
      HAR_Data& har = sm.HAR();
      HAA_Data& haa = sm.HAA();
  
      //populate the HAA fields uniquely for this success case
      sm.SetMipRegReply( haa.MipRegReply );

      haa.ResultCode.Set( AAA_AUTHENTICATION_REJECTED);

      if ( ! sm.SetErrorMessage( haa.ErrorMessage ) )
	haa.ErrorMessage.Set("Mobile Node failed authentication\n");
      haa.ErrorReportingHost.Set ( har.DestinationHost()); // this HA host
 
      sm.SendHAA(); 

      // Update the session state.
      sm.Session().Update(AAASession::EVENT_AUTH_FAILED);
    }
  };

  class AcSendHAA_DueToInvalidRegReq : public DiameterMip4HaServerAction 
  {
    void operator()(DiameterMip4HaServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "[%N] Sending HAA with Auth Rejected result-code: Invalid Mip Registration Request.\n"));

      HAR_Data& har = sm.HAR();
      HAA_Data& haa = sm.HAA();

       //populate the HAA fields uniquely for this success case

      haa.ResultCode.Set( AAA_AUTHENTICATION_REJECTED);

      if ( ! sm.SetErrorMessage( haa.ErrorMessage ) )
	haa.ErrorMessage.Set("Invalid Mip Registration Request\n");
      haa.ErrorReportingHost.Set ( har.DestinationHost()); // this HA host
 
      sm.SendHAA(); 

      // Update the session state.
      sm.Session().Update(AAASession::EVENT_AUTH_FAILED);
    }
  };   
   
  enum state {
    StInitialize,
    StProcessMipRegReq,
    StAccepted,          //  accepted Mip Reg Req
    StRejected,          //  rejected Mip Reg Req
    StInvalidRegReq,     // 
    StSenttHAA,          // Should this state added to the table ??
    StTerminated
  };

  AcProcessMipRegReq acProcessMipRegReq; 
  AcSendHAA_DueToRegRequestAccepted acSendHAA_DueToRegRequestAccepted;
  AcSendHAA_DueToRegRequestRejected acSendHAA_DueToRegRequestRejected;
  AcSendHAA_DueToInvalidRegReq acSendHAA_DueToInvalidRegReq;

  // Defined as a leaf class
  DiameterMip4HaServerStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       DiameterMip4HaServerStateMachine::EvRxHAR,
		       StProcessMipRegReq, acProcessMipRegReq);

    AddStateTableEntry(StInitialize, 
		       DiameterMip4HaServerStateMachine::EvSgDisconnect, 
		       StTerminated);

    AddWildcardStateTableEntry(StInitialize, StTerminated);

    AddStateTableEntry(StProcessMipRegReq, 
		       DiameterMip4HaServerStateMachine::EvSgRegRequestAccepted, 
		       StAccepted,
		       acSendHAA_DueToRegRequestAccepted); 

    AddStateTableEntry(StProcessMipRegReq, 
		       DiameterMip4HaServerStateMachine::EvSgRegRequestRejected, 
		       StRejected, acSendHAA_DueToRegRequestRejected); 

    AddStateTableEntry(StProcessMipRegReq, 
		       DiameterMip4HaServerStateMachine::EvSgInvalidRegRequest, 
		       StInvalidRegReq, acSendHAA_DueToInvalidRegReq); 

    AddStateTableEntry(StAccepted,
		       DiameterMip4HaServerStateMachine::EvSgDisconnect,
		       StTerminated);

    AddStateTableEntry(StRejected,
		       DiameterMip4HaServerStateMachine::EvSgDisconnect,
		       StTerminated);

    AddWildcardStateTableEntry(StAccepted, StTerminated);
    AddWildcardStateTableEntry(StRejected, StTerminated);
    AddWildcardStateTableEntry(StInvalidRegReq, StTerminated);

    InitialState(StInitialize);
  }
  ~DiameterMip4HaServerStateTable_S() {}
};

typedef 
ACE_Singleton<DiameterMip4HaServerStateTable_S, ACE_Recursive_Thread_Mutex> 
DiameterMip4HaServerStateTable;

DiameterMip4HaServerStateMachine::DiameterMip4HaServerStateMachine
(AAAServerSession& s, DiameterJobHandle &h)
  : AAA_StateMachine<DiameterMip4HaServerStateMachine>
  (*this, *DiameterMip4HaServerStateTable::instance(), "MIP4_HA_SERVER"),
    session(s),    handle(h) 
{
  AAA_StateMachine<DiameterMip4HaServerStateMachine>::Start();
}

 DiameterMip4HaServerStateMachine::~DiameterMip4HaServerStateMachine() 
 {
    handle.Job().Remove(this); 
 }


void 
DiameterMip4HaServerStateMachine::SendHAA()
{
  DiameterMsg msg;

  HAA_Data &haaData= HAA();
  
  HAA_Parser parser;
  parser.setAppData(&haaData);
  parser.setRawData(&msg);

  #ifdef PRINT_MSG_CONTENT // debug info
    printf("HAA:\nResult-Code: %d\nMip-Reg-Reply:  %s\nUser-Name:%s\n \n", 
	   haaData.ResultCode(), 
	   haaData.MipRegReply().data(), 
	   haaData.UserName().data());
 
  #endif
  try {
    parser.parseAppToRaw();
  }
  catch (DiameterParserError) {
    AAA_LOG((LM_ERROR, "[%N] Parsing error.\n"));
    return;
  }
  AAAServerSession &session = Session();
  
  AAAMessageControl msgControl( &session );  
  if (msgControl.Send(msg) != AAA_ERR_SUCCESS) {
    AAA_LOG((LM_ERROR, "Failed sending message.\n"));
  }
  else {
    AAA_LOG((LM_DEBUG, "Sent HAA Message.\n"));
  }
}





