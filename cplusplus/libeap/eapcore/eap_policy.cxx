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
// $Id: eap_policy.cxx,v 1.12 2004/06/19 14:32:16 vfajardo Exp $

// eap_policy.cxx:  Class for EAP policy
// Written by Yoshihiro Ohba

#include "eap_policy.hxx"

void
EapContinuedPolicyElement::AddSuccessPolicyElement(PolicySwitch sw)
{
  if (sw == PolicyOnSuccess)
    nextOnSuccess = EapSuccessPolicyElement_S::instance();
  else
    nextOnFailure = EapSuccessPolicyElement_S::instance();
}

void
EapContinuedPolicyElement::AddFailurePolicyElement()
{
  nextOnFailure = EapSuccessPolicyElement_S::instance();
}

void
EapContinuedPolicyElement::AddContinuedPolicyElement
(EapPolicyElement *p, PolicySwitch sw)
{
  if (sw == PolicyOnSuccess)
    nextOnSuccess = p;
  else
    nextOnFailure = p;
}

EapPolicyElement*
EapPolicy::NextPolicyElement(EapContinuedPolicyElement::PolicySwitch sw)
{
  if (currentPolicyElement->policyType == EapPolicyElement::FinalSuccess)
    // This is a success PolicyElement.
    return 0;

  if (currentPolicyElement->policyType == EapPolicyElement::FinalFailure)
    // This is a failure PolicyElement.
    return 0;

  // This is a continued PolicyElement.
  EapContinuedPolicyElement *e = 
    (EapContinuedPolicyElement*)currentPolicyElement;

  return e->NextPolicyElement(sw);
}
	  
bool 
EapPolicy::Allow(EapType t)
{
  if (t == EapType(2)) // Notification MUST NOT be Nak'ed
    return true;
  if (!currentPolicyElement)
    return false;
  if (currentPolicyElement->policyType == EapPolicyElement::FinalSuccess)
    return true;
  if (currentPolicyElement->policyType == EapPolicyElement::FinalFailure)
    return false;

  EapContinuedPolicyElement *e = (EapContinuedPolicyElement*)currentPolicyElement;

  if (e && e->methodType == t)
    return true;
  else
    return false;
}

bool 
EapPolicy::IsSatisfied()
{
  if (!currentPolicyElement)
    return false;
  if (currentPolicyElement->policyType == EapPolicyElement::FinalSuccess)
    return true;
  if (currentPolicyElement->policyType == EapPolicyElement::FinalFailure)
    return false;
  else
    return false;
}

EapType 
EapPolicy::CurrentMethod() throw (PolicyError)
{ 
  if (!currentPolicyElement)
    throw NoCurrentMethod;
  if (currentPolicyElement->policyType == EapPolicyElement::FinalSuccess)
    throw NoCurrentMethod;
  if (currentPolicyElement->policyType == EapPolicyElement::FinalFailure)
    throw NoCurrentMethod;

  EapContinuedPolicyElement *e = 
    (EapContinuedPolicyElement*)currentPolicyElement;
  return e->methodType; 
}

void 
EapPolicy::Update(EapContinuedPolicyElement::PolicySwitch sw) 
{
  if (!currentPolicyElement)
    return;
  if (currentPolicyElement->policyType == EapPolicyElement::FinalSuccess)
    return;
  if (currentPolicyElement->policyType == EapPolicyElement::FinalFailure)
    return;
  EapContinuedPolicyElement *e = 
    (EapContinuedPolicyElement*)currentPolicyElement;
  if (sw == EapContinuedPolicyElement::PolicyOnSuccess)
    currentPolicyElement = e->nextOnSuccess;
  else
    currentPolicyElement = e->nextOnFailure;
}

void
EapPolicy::Update(EapTypeList &typeL)
{
  while (currentPolicyElement && currentPolicyElement->IsContinued())
    {
      Update(EapContinuedPolicyElement::PolicyOnFailure);
      EapContinuedPolicyElement* cpe = 
	(EapContinuedPolicyElement*)currentPolicyElement;

      // if type list is empty, just one-hop traversal of the failure
      // branch.
      if (typeL.empty())
	break;

      // otherwise, a matched type is found in the type list, stop at
      // the PolicyElement.
      if (typeL.Search(cpe->GetType()))
	break;
    }
}

void 
EapPolicy::MakeTypeList(EapTypeList &typeL, bool isVSE, EapPolicyElement *pe)
{
  // check if this is a continued PolicyElement.
  if (!pe || !pe->IsContinued())
    return;

  EapContinuedPolicyElement* cpe = (EapContinuedPolicyElement*)pe;

  EapType type = cpe->GetType();

  // If the nak'ed Request is of legacy type, and the list contains
  // vendor-specific type, the vendor-specific type is converted to
  // the legacy type 254.
  if (!isVSE && type.IsVSE())
    type = EapType(254);

  // Add the type of this PolicyElement, if the type is not in the list
  if (typeL.Search(type))
    return;
  
  typeL.push_back(type);

  // repeat the same thing on the success branch.
  MakeTypeList(typeL, 
	       cpe->NextPolicyElement
           (EapContinuedPolicyElement::PolicyOnSuccess) ? true : false);

  // repeat the same thing on the failure branch.
  MakeTypeList(typeL, 
	       cpe->NextPolicyElement
           (EapContinuedPolicyElement::PolicyOnFailure) ? true : false);
}

void 
EapPolicy::MakeTypeList(EapTypeList &typeL, bool isVSE)
{
  MakeTypeList(typeL, isVSE, currentPolicyElement);

  // if type list is empty, insert legacy type zero (0).
  if (typeL.empty())
    typeL.push_back(EapType(0));
}
