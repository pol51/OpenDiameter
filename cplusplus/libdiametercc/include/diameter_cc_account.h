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

/* 
   File: diameter_cc_account.h
   Author: Amrit Kaur (kaur_amrit@hotmail.com)
*/

#ifndef __CC_ACCOUNT_H__
#define __CC_ACCOUNT_H__

#include "framework.h"
#include "diameter_cc_parser.h"

/// The base class for Diameter CC Account information.
class DiameterCCAccount
{
public:
  DiameterCCAccount ()
  {}

  DiameterCCAccount(const subscriptionId_t& subscriptionid,
                    const requestedServiceUnit_t& balanceunits,
                    const CreditControlFailureHandlingEnum& creditControlFailureHandlingEnum)
    : subscriptionId(subscriptionid), 
      balanceUnits(balanceunits),
      ccFailureHandling(creditControlFailureHandlingEnum)
  {
  }

  DiameterCCAccount operator = (const DiameterCCAccount& b)
  {
    subscriptionId = b.subscriptionId;
    balanceUnits = b.balanceUnits;
    ccFailureHandling = b.ccFailureHandling;
    return *this;
  }

  subscriptionId_t& SubscriptionId() 
  { 
    return subscriptionId; 
  }

  requestedServiceUnit_t& BalanceUnits()
  { 
    return balanceUnits;
  }

  void BalanceUnits(const requestedServiceUnit_t& balanceunits)
  { 
    balanceUnits = balanceunits;
  }

  requestedServiceUnit_t& ReservedUnits()
  { 
    return reservedUnits;
  }

  void ReservedUnits(const requestedServiceUnit_t& reservedUnit)
  { 
    reservedUnits = reservedUnit;
  }

  CreditControlFailureHandlingEnum& CreditControlFailureHandling()
  { 
    return ccFailureHandling;
  }

  void CreditControlFailureHandling(const CreditControlFailureHandlingEnum& creditControlFailureHandlingEnum)
  { 
    ccFailureHandling = creditControlFailureHandlingEnum;
  }

protected:
  subscriptionId_t subscriptionId;
  
  requestedServiceUnit_t balanceUnits;
  
  requestedServiceUnit_t reservedUnits;
  
  CreditControlFailureHandlingEnum ccFailureHandling;
};

#endif
