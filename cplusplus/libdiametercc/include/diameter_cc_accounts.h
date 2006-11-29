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

/* $Id: */
/* 
   diameter_cc_accounts.h
   Accounts Diameter Credit Control Application
*/

#ifndef __CC_ACCOUNTS_H__
#define __CC_ACCOUNTS_H__

#include <map>
#include "framework.h"
#include "diameter_cc_parser.h"
#include "diameter_cc_account.h"

/// The base class for Diameter CC Account information.
class DiameterCCAccounts
{
public:
  DiameterCCAccounts ()
  {}

  ~DiameterCCAccounts ()
  {
    accounts.clear();
  }
  
  bool accountExists(const subscriptionId_t& subscriptionId)
  {
    std::map<subscriptionId_t, DiameterCCAccount>::iterator i = accounts.find(subscriptionId);
    if (i != accounts.end())
      {
        return true;
      }
    return false;
  }

  DiameterCCAccount getAccount(const subscriptionId_t& subscriptionId)
  {
    DiameterCCAccount account;
    std::map<subscriptionId_t, DiameterCCAccount>::iterator i = accounts.find(subscriptionId);
    if (i != accounts.end())
      {
        account = i->second;
      }
    return account;
  }

  int setAccount(const subscriptionId_t& subscriptionId,
                 const DiameterCCAccount& diameterCCAccount)
  {
    std::map<subscriptionId_t, DiameterCCAccount>::iterator i = accounts.find(subscriptionId);
    if (i != accounts.end())
      {
        i->second = diameterCCAccount;
      }
    return true;
  }
  
  int addAccount( const subscriptionId_t& subscriptionId,
                  const DiameterCCAccount& account)
  {
    int insert = 0;
    accounts.insert(std::pair<subscriptionId_t,DiameterCCAccount>
                    (subscriptionId, account));
    return insert;
  }

  int removeAccount(const subscriptionId_t& subscriptionId)
  {
    int  remove =0;
    accounts.erase(subscriptionId);
    return remove;
  }

private:
  // Collection of DiameterCCAccounts
  std::map<subscriptionId_t, DiameterCCAccount> accounts;
};

#endif
