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
// $Id: eap_policy.hxx,v 1.11 2004/06/17 21:13:36 yohba Exp $

// eap_policy.hxx:  header file for EAP policy
// Written by Yoshihiro Ohba

/*! \page authpolicy Authentication Policy 

    This API supports a generic and flexible way of supporting
    authentication policies, which is particulary useful in an
    environmment in which multiple authentication methods are used.

    An authentication policy is represented as a binary tree in which
    each node corresponds to an authentication method.  If a node is
    not a leaf node, it has two branches each corresponds to success
    or failure operation.  A node in a policy tree is refered to as
    policy element (PE).  An example of authentication policy
    representation is shown below.

\verbatim

                             s        s
                   --> A(1) --> B(2) --> S1
                       |        |     f        s
                       |        +------> C(3) --> S2
                       |                 |     f
                       |                 +------> F
                       |     f        s        s
                       +------> D(2) --> E(3) --> S3
                                |     f  |     f
                                |        +------> S4
                                |
                                +------> F


\endverbatim

    In the above example, a policy tree is composed of five PEs,
    namely A, B, C, D, E, S1, S2, S3, S4 and F.  Branches tagged with
    's' and 'f' of each PE are traversed when the result of the
    corresponding authentication type is successful and failure,
    respectively.  PEs S1, S2, S3, S4 and F are leaf nodes
    corresponding to final success and final failure, respectively.
    Each final success leaf can be bound to a distinct authorization
    type.  Each PE is bound to a single authentication type
    represented by the number in round brackets.

    The above policy tree constitutes an ordered authentication
    policy.  For example, PE B is never examined before PE A.  PE B is
    never examined if PE A fails.  An unordered authentication policy
    can be rearrenged to an ordered authentication policy.  The
    ordered policy in the above example is an rearrengement of an
    unordered policy which succeeds if any two authentication types in
    the set of authentication types 1, 2 and 3 succeeds.

    Both Authenticator and Peer can have a distinct authentication
    policy tree.  During the EAP conversation, the two peering policy
    trees are traversed based on Request/Response message exchange in
    which the Authenticator proposes a specific authentication type
    and the Peer replies with a Response with the proposed
    authentication type if the proposed type is supported by the Peer,
    otherwise it replies with a Nak with a list of supported
    authentication types in the order of preference.  In the latter
    case, the Nak'ed PE must be considered failure in the
    Authenticator policy, and the list contained in the Nak must be
    used for further traversing the failure branch(es) of the policy
    subtree until the traversal encounters a PE that matches one of
    the authentication type(s) in the list contained in the Nak or a
    final success/failure leaf.

 */

#ifndef __EAP_POLICY_HXX__
#define __EAP_POLICY_HXX__

#include <ace/Singleton.h>
#include "eap.hxx"

/// This class is used for a single policy element (PE).  
/// See \ref authpolicy "Authentication Policy" for detailed
/// description about authentication policy.
class EAP_EXPORTS EapPolicyElement
{
  friend class EapPolicy;
public:
  enum PolicyType {
    FinalSuccess,  /// Indicates final success.
    FinalFailure,  /// Indicates final failure.
    Continued      /// Indicates there are other PolicyElement to try.
  };

  /// This function returns true when the PE is of type Continued.
  inline bool IsContinued() 
  { 
    if (policyType==Continued) return true;
    return false;
  }
protected:
  /// This class instance cannot be directly created.  Use constractor
  /// of derived classes.
  EapPolicyElement(PolicyType t) : policyType(t) {}
  PolicyType policyType;
};

/// A class for a policy element representing final success
/// authentication.  This class is a singleton.
class EAP_EXPORTS EapSuccessPolicyElement : public EapPolicyElement
{
public:
  EapSuccessPolicyElement() : EapPolicyElement(FinalSuccess) {}
};

/// A class for a policy element representing final failure
/// authentication.  This class is a singleton.
class EAP_EXPORTS EapFailurePolicyElement : public EapPolicyElement
{
public:
  EapFailurePolicyElement() : EapPolicyElement(FinalFailure) {}
};

/// A singleton for EapSuccessPolicyElement.
typedef ACE_Singleton<EapSuccessPolicyElement, ACE_Recursive_Thread_Mutex> 
EapSuccessPolicyElement_S;

/// A singleton for EapFailurePolicyElement.
typedef ACE_Singleton<EapFailurePolicyElement, ACE_Recursive_Thread_Mutex> 
EapFailurePolicyElement_S;

/// This class represents an non-leaf policy element which contains a
/// single authentication type as well as two branches each taken when
/// the authentication type succeeds and fails, respectively.
/// See \ref authpolicy "Authentication Policy" for detailed
/// description about authentication policy.
class EAP_EXPORTS EapContinuedPolicyElement : public EapPolicyElement
{
  friend class EapPolicy;
public:
  /// By default, a success leaf policy element is added to the
  /// success branch, and a failure leaf policy element is added to
  /// the failure branch, of this policy element.  So, when a single
  /// EAP method is used, there is nothing to do for applications to
  /// explicitly set the branches.
  EapContinuedPolicyElement(EapType t) : 
    EapPolicyElement(Continued), 
    nextOnSuccess(EapSuccessPolicyElement_S::instance()), 
    nextOnFailure(EapFailurePolicyElement_S::instance()), methodType(t) {}
  /// Used for adding a policy node.
  enum PolicySwitch {
    PolicyOnSuccess,
    PolicyOnFailure
  };
  EapPolicyElement *NextPolicyElement(PolicySwitch sw) 
  { 
    if (sw == PolicyOnSuccess)
      return nextOnSuccess;
    else
      return nextOnFailure;
  }
  /// Add a success leaf policy element 
  /// It is added to either success branch or failure branch.
  void AddSuccessPolicyElement(PolicySwitch sw);
  /// Add a failure leaf policy element 
  /// It is added to failure branch only.
  void AddFailurePolicyElement();
  /// Add a continued policy element.
  /// It is added to either success branch or failure branch.
  void AddContinuedPolicyElement(EapPolicyElement *p, PolicySwitch sw);
  bool IsContinued() { return true; }
  EapType GetType() { return methodType; }
private:
  EapPolicyElement *nextOnSuccess;
  EapPolicyElement *nextOnFailure;
  EapType methodType;
};

/// This class represents an EAP policy.  
/// See \ref authpolicy "Authentication Policy" for detailed
/// description about authentication policy.
class EAP_EXPORTS EapPolicy
{
public:
  enum PolicyError {
    NoCurrentMethod // used by GetCurrentMethod
  };
  EapPolicy() : initialPolicyElement(0), currentPolicyElement(0) {}
  ~EapPolicy() {}

  /// This function is used for getting the next 
  EapPolicyElement* NextPolicyElement
  (EapContinuedPolicyElement::PolicySwitch sw);

  /// This function is used for getting the current PolicyElement.
  inline EapPolicyElement* CurrentPolicyElement() 
  { return currentPolicyElement; }

  /// This function is used for setting the current PolicyElement.
  void CurrentPolicyElement(EapPolicyElement *e) { currentPolicyElement = e; }

  /// This function is used for getting the initial PolicyElement.
  inline EapPolicyElement* InitialPolicyElement() 
  { return initialPolicyElement; }

  /// This function is used for setting the initial PolicyElement.
  void InitialPolicyElement(EapPolicyElement *e) 
  { initialPolicyElement = e; }

  /// This function is used for getting the current method.
  EapType CurrentMethod() throw (PolicyError);

  /// This function is used for traversing one policy hop on the
  /// sucess or failure branch.
  void Update(EapContinuedPolicyElement::PolicySwitch sw);
  
  /// This function is used for traversing the failure policy path
  /// until encountering a type contained in the type list contained
  /// in Nak message.
  void Update(EapTypeList &typeL);

  /// This function is used by peers to check whether received request
  /// type is supported or not.
  bool Allow(EapType t);

  /// This function is used by peers and authenticators to check the
  /// current policy indicates the final success or not.
  bool IsSatisfied();

  /// This function is used by Peers to generate a type list to be
  /// included in Nak.  The type list is created based on the sub-tree
  /// rooted at the current PolicyElement.  The second argument
  /// indicates whether the Request to be Nak'ed is vendor-specific
  /// Request or legacy one.
  void MakeTypeList(EapTypeList &typeL, bool isVSE);

  /// This function is used by a backend authenticator to check
  /// whether "pick-up-init" is supported for a given EAP type.
  bool SupportPickUp(EapType type)
  {
    if (type == EapType(1))
      return true;
    return false;
  }

private:
  /// Actual definition of MakeTypeList();
  void MakeTypeList(EapTypeList &typeL, bool isVSE, EapPolicyElement *pe);

  /// Indicates the initial PolicyElement;
  EapPolicyElement *initialPolicyElement;

  /// Indicates the current PolicyElement;
  EapPolicyElement *currentPolicyElement;
};

#endif // __EAP_POLICY_HXX__
