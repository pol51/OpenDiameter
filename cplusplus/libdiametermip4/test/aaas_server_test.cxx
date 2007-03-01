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
 * Synopsis : AAA Server application test 
 */

#include "diameter_specific_aaas_server_interface.hxx"


int main(int argc, char *argv[])
{
   /*
    * This sample server test program is straight forward.
    * After creating an instance of the application core
    * and passing it the configuration file, it registers
    * a class factory for handling requests with application
    * id Mip4ApplicationId. The the configuration filename is passed in as
    * an argument.
    *
    * The functionality that is specific to the AAA server is 
    * specified in "diameter_specific_aaas_server_interface.hxx". 
    * This file includes a very thin implementation, mainly providing fixed 
    * values to different paramenters. The implementor can use this example to
    * create an interface to a real AAA server.

    * This program will create the AAA Server server session (accepting an AMR)
    * if the Mobile Node is NOT in co-located mode, it will create a client 
    * session (sending and HAR to the Home Agent).
    
    * Providing the AAA Server functionality with the interface specified in 
    * "diameter_specific_aaas_server_interface.hxx" or 
    * "diameter_mip4_aaas_server_interface.hxx" 
    * this sample application can act as a AAA server Diameter-MIP application.
    * 
    * With the following implementation, every time the a message with
      Mip4applicationId arrives, 
       if it belongs to an existing session => the msg is passed to the session
       if its SessionId is new to the server => it will create a new session 
          obj, and the message will be passed to the session message handler.

     The HandleDisconnect method of the session object needs to deallocate 
     itself, since the openDiameter library only removes the session from its
     internal database,but it is the responsability of the application creating
     the session to deallocate it.

    * Another option to implement the server is to create the maximum number
    of sessions that this server will handle simultanously , and when a session
    terminates the session will be flaged as an unused session, which will 
    be reused when the server receives a msg related to a new session.

    This can be implemented by overwriting the "create" method in the 
    AAAServerSessionClassFactory. The create method is going to create a new 
    server session object only if there is no unused session object and 
    number of session objects did not exceed the maximum limit.

    In this case the HandleDisconnect method of the session object needs 
    flag the session object as unused, so the "create" function can make use 
    of it when needed. 
    The exit routine will need to deallocate all the session objects
    previously allocated.
    
    * On exit, the factory should be removed
    * from the application core and the application cores
    * destructor will perform internal cleanup.
    *
    */

   if (argc != 2) {
      AAA_LOG((LM_DEBUG, "(%P|%t) Server: Usage: aaas_server_test [config file]\n"));
      return (1);
   }

   AAA_LOG((LM_DEBUG, "(%P|%t) Server: Application starting\n"));
   AAA_Task myTask;
   try {
      myTask.Start(10);
   }
   catch (...) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Server: Cannot start task\n"));
      return (1);
   }
   AAAApplicationCore myCore;
   if (myCore.Open(argv[1], myTask) != AAA_ERR_SUCCESS) {
      ACE_ERROR((LM_ERROR, "(%P|%t) Server: Application core open error\n"));
      return (1);
   }


   /* 
    * application id is for this sample
    */

   typedef DiameterMip4AaaSServerSession<SpecificMip4AaaSServer>  DiameterMip4SpecificServer;

    AAAServerSessionClassFactory<DiameterMip4SpecificServer> authFactory( myCore, Mip4ApplicationId );

   /*
    * Register the factory to our application core.
    * All request with application id Mip4ApplicationId will be
    * handled here.
    */
   myCore.RegisterServerSessionFactory(&authFactory);

   /*
    * This will block indefinetly.
    * It should be improved providing an exit routine.
    */

   while (1)
     {
       ACE_OS::sleep (10);
     }

   // removing the session factory will go in the exit routine. 
   myCore.RemoveServerSessionFactory(&authFactory);

   AAA_LOG((LM_DEBUG, "(%P|%t) Server: Exiting\n"));
   
   return (0);
}






