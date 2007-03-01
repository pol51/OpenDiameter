/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* OpenDiameter: Open-source software for the Diameter protocol           */
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
/* 
   mip4_diameter_ha_client_interface.hxx
   Definition of the Diameter interface to the MIP 4 Application, for HA 
   Client session.

   This virtual interface is defined to resolve the cross reference need 
   of the 2 parts (diameter & MIP) of the software/classes.
 
   Written by Miriam Tauil
   Created Sep 3, 2004.
*/

#ifndef __MIP4_DIAMETER_HA_CLIENT_INTERFACE_H__
#define __MIP4_DIAMETER_HA_CLIENT_INTERFACE_H__

#include "diameter_api.h"

class Mip4DiameterHaClientInterface {
public:
  //Ideally  RxMipRegReq() should have been a pure virtual class since it
  // needs an implementation in DiameterMip4HaClientSession, but we could not
  // solve a compiler error complaining about 
  // Mip4DiameterHaClientInterface::RxMipRegReq(diameter_octetstring_t&) being 
  // virtual and not implemnted.
  // It might be a combination of a template class being inherited from
  // a pure virtual class w\ the implementation provided in the template class.
  // MT 9/7/2004

  virtual void RxMipRegReq( diameter_octetstring_t &mipRegReq){} //=0;
  virtual ~Mip4DiameterHaClientInterface(){}

};

#endif //__MIP4_DIAMETER_HA_CLIENT_INTERFACE_H__
