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
/* Lesser General License for more details.                               */
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
   diameter_mip4_aaas_server_session_internal_interface.hxx
   Server Session interface definition for Diameter MIP4 AAA Server Application
   This is used to provide a definite type (not a template), specifying the 
   server session, so it can specified as a parameter passed to the 
   AAA S client session
   Written by Miriam Tauil
   Created October 8th, 2004.
*/

#ifndef __MIP4_AAAS_SERVER_SESSION_INTERNAL_INTERFACE_H__
#define __MIP4_AAAS_SERVER_SESSION_INTERNAL_INTERFACE_H__

#include <string>
#include <list>
#include "ace/Synch.h"
#include "diameter_api.h"
#include "diameter_mip4_aaas_server_fsm.hxx"
#include "diameter_mip4_parser.hxx"


/// Diameter MIP4 AAA server Server Session Interface.
/// This interface is implemented by the DiameterMip4AaaSServerSession class.
/// It defines the functions called by the DiameterMip4AaaSClientSession.
/// This class is needed, to define a reference of the 
/// DiameterMip4AaaSServerSession class to be passed as a parameter to
/// the DiameterMip4AaaSClientSession. (the DiameterMip4AaaSServerSession is 
/// a template, this interface definition eliminates defining the 
/// DiameterMip4AaaSClientSession as a template too. )

//class DIAMETER_MIP4_AAAS_SERVER_INTERFACE_EXPORTS DiameterMip4AaaSServerSessionInterface
class DiameterMip4AaaSServerSessionInterface
{
 public:

  virtual HAR_Data& HAR()=0;
  virtual HAA_Data& HAA()=0;
  virtual void ServerSessionNotify (AAA_Event ev)=0;
  virtual ~DiameterMip4AaaSServerSessionInterface() { }
};
#endif
