
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

/* Author   : Miriam Tauil
 * Synopsis : Home Agent Client Session application test 
 */


/******************************************************************************

This sample application demostrates the use of the Diameter MIPv4 library, 
implemented as part of the Open Diameter project.

This specific application shows the use of the Diameter MIPv4 Home Agent
 Client Session. It allows the use of different MIPv4 implementations. 
The MIPv4 implementation needs to follow a specific interface specified in
 the file : diameter_mip4_ha_client_interface.hxx .

The DiameterMip4HaClientSession is a C++ template that accepts as an argument 
a MipHaClientsession object. As mentioned above, the class of this object 
needs to follow the interface in:  diameter_mip4_ha_client_interface.hxx .

The following message flow (Scenario 1) is implemented in this application:
Mobile Node in Co-Located Mode

		      AAAS
		      ^ |
	          AMR | | AMA  
		      | |
		      | V
	MIP-Reg-Req	       
   MN ---------------> HA
      <--------------
        MIP-Reg-Reply


This sample application, for simplicity,demostrates only the HA CLIENT session.
(HA server session is demostrated in scenario 2, has_test. A real HA will 
include the functionality of the 2 sample applications: hac_test and has_test)
******************************************************************************/
/*
There are 2 ways to allocate the DiameterMip4HaClientSession:
=============================================================

1. Using the "new" operator. If this option is used the virtual function 
  Abort() needs to be implemented, and should contain the command:
  delete DiameterMip4HaClientSession. 
  The reason for the implementation of the Abort() function is to allow 
  memory deallocation consistently with the allocation method.

  For example:

 typedef DiameterMip4HaClientSession<XyzHaClientSession> DiameterMip4Session;

 class MyDiameterMip4Session::public DiameterMip4Session {

   public:
     MyDiameterMip4Session(AAAApplicationCore &appCore):
        DiameterMip4Session(appCore){}

     void Abort() {delete this}

 };


int main(int argc, char *argv[]) 
{
   ...
   // allocation
   DiameterMip4Session _diameterMip4Session = new MyDiameterMip4Session(appCoreRef);
   ...

   // deallocation
   _diameterMip4Session.Abort();

}

==============================================================

2. The other option will allocate the variable for the DiameterMip4Session from
the stack, which will be automatically deallocated when the main program exists.
In this case the virtual empty Abort() function in the DiameterMip4Session,
does not need to be called or implemented.

 For example:

 typedef DiameterMip4HaClientSession<XyzHaClientSession> DiameterMip4Session;

 // no need to defined an additional class inherited from DiameterMip4Session

int main(int argc, char *argv[]) 
{
   ...
   // allocation
   DiameterMip4Session _diameterMip4Session(appCoreRef);
   ...

   // automatic deallocation, 
   // or it is possible to call Abort(), implemented 
   // as calling the destructor: ~_dimeterMip4Session();

}

========================================================================
========================================================================
Additionally to the 2 option of memory allocation for the object
DiameterMip4Session, the application can allocate DiameterMip4Session objects 
  as they are required, and discarde them as the session teminates 

OR 
The DiameterMip4Session objects can be instantiated at the beginning of 
the main application and they can be used and reused as needed. This option
is illustrated in the second section in the code below:#ifdef MULTIPLE_SESSIONS.

The first option is illustrated for handling only one session in the section
#ifndef MULTIPLE_SESSIONS.

******************************************************************************/


#include "xyz_ha_client_session.hxx"
#include "../include/diameter_mip4_ha_client_session.hxx"

  typedef DiameterMip4HaClientSession<XyzHaClientSession> DiameterMip4Session;

int main(int argc, char *argv[]) 
{
 
   /*
    * The main test program is straightforward. It
    * processes all arguments passed to it then 
    * creates the appropriate number of sessions.
    * The sessions are being used as the MIP registrations are 
    * received. When the Diameter session terminates the State Machine 
    * sets the state to StInitialize, and the object is available again 
    * for a new session.
    * 
    * The number of sessions as well as the authorizing
    * host and realm is passed in as arguments. Also,
    * the configuration file is an argument.
    *
    * This application should act on triggering events: 
    * receiving MIP registration requests.  For simplicity, the content 
    * of the MIP registration requests are statically specified. 
    */

   if (argc != 5) {
      AAA_LOG((LM_DEBUG, "(%P|%t) Client: Usage: hac_test [AAA Server host] [AAA Server realm] [num session] [config file]\n"));
      return (1);
   }

   int howManySessions = atoi(argv[3]);
   if (howManySessions <= 0) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Client: Invalid number of sessions\n"));
      return (1);
   }

  AAA_LOG((LM_DEBUG, "(%P|%t) HA Client: Application starting\n"));

   AAA_Task myTask;
   try {
      myTask.Start(10);
   }
   catch (...) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Client: Cannot start task\n"));
      return (1);
   }

  AAAApplicationCore appCore;   
  if (appCore.Open(argv[4], myTask) != AAA_ERR_SUCCESS) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Client: Application core open config file error\n"));
      return (1);
   }
  
   AAAApplicationCore &appCoreRef= appCore;

   // Primitive wait. Waiting for the client to
   // establish connection with the peers
   while (appCore.GetNumActivePeerConnections() == 0);

#define MULTIPLE_SESSIONS

#ifndef MULTIPLE_SESSIONS
   // this needs to be done for ' howManySessions'
    DiameterMip4Session _diameterMip4Session( appCoreRef);   
    _diameterMip4Session.SetDestinationHost( argv[1]);
    _diameterMip4Session.SetDestinationRealm( argv[2]);

    _diameterMip4Session.Start();

   diameter_octetstring_t _mipRegReq = "123456678788999009009-0-900-90908989809898980980978676766567564cffvtfghgfvbhbnhgjff";
   _diameterMip4Session.RxMipRegReq( _mipRegReq);

   while (1)
     {
     ACE_OS::sleep(100);
     }

#endif


#ifdef MULTIPLE_SESSIONS

   DiameterMip4Session *_diameterMip4SessionPtr[howManySessions];

   for (int i=0; i<howManySessions; i++)
   {
     (_diameterMip4SessionPtr[i]) = new DiameterMip4Session( appCoreRef);   
     (*(_diameterMip4SessionPtr[i])).SetDestinationHost( argv[1]);
     (*(_diameterMip4SessionPtr[i])).SetDestinationRealm( argv[2]);
   }

   // For each Mip registration request received assigned a free
   // DiameterMip4Session and pass it the content of the Registration Request
   int newMipRegReq = 1;
   DiameterMip4Session *nextSession = 0;

   while ( newMipRegReq )
     {
       for ( int i=0; i<howManySessions ; i++)  // loop through the array
	 {
	   if ( (*(_diameterMip4SessionPtr[i])).State() == 
		(*(_diameterMip4SessionPtr[i])).InitialState()  )
	     {
	       nextSession = _diameterMip4SessionPtr[i]; 
	       break;
	     }
	 }

       if (nextSession == 0 )
	 {
	   // error handling for no session available to handle new MIP Reg Req
	   // ...
	 }
       else
	 {
	   diameter_octetstring_t _mipRegReq = "AAAAAAABBBBBBBBCCCCCCCCC";
	   (* nextSession).RxMipRegReq( _mipRegReq);
	 }
     }

   // should be in exit function
   for (int i=0; i<howManySessions; i++)
   {
     delete (_diameterMip4SessionPtr[i]);
   }
#endif // MULTIPLE_SESSIONS
     return (0);
}



