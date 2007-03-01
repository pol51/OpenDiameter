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
/* $Id: eap_method_registrar.hxx,v 1.16 2004/06/17 21:13:36 yohba Exp $ */
/* 
   eap_method_registrar.hxx:  class for EAP state machine registration

   Written by Yoshihiro Ohba
   
*/

/*! \page methodRegistrar Method Registrar

A method registrar is used for finding a function (referred to as a
<em>creator</em>) to create a method state machine instance corresponding 
to a given pair if EAP type and role.

To this end, a method registrar maintains a table that stores a list
of EapMethodTableEntry class entries each consists of:

- EAP type name string (e.g., "Identity")
- Eap type (e.g., EapType(1))
- Eap role (i.e., Peer or Authenticator)
- A pointer to creator function.

Each EAP type (or EAP authentication method) must be registared before
the corresponding creator function is called by sessions.  For EAP
types that are built in the library, registrations are automatically
done inside the library when the process that linked the library
starts.  For EAP types that are defined by applications, registrations
are explicitly done by application.  The registration is done via
EapMethodTable::instance()->add() with specifying a record to
register.


*/
#ifndef __EAP_METHOD_REGISTRAR_HXX__
#define __EAP_METHOD_REGISTRAR_HXX__

#include <string>
#include <list>
#include <ace/Singleton.h>
#include <ace/Reactor.h>
#include <boost/function/function1.hpp>
#include "eap.hxx"
#include "eap_fsm.hxx"

/// Functor for which return type is a pointer to state machine and
/// argument is a pointer to session.
typedef boost::function1<EapMethodStateMachine*, EapSwitchStateMachine&>
EapMethodStateMachineFunctor;

/// The EapStateMachineCreator template class is used by
/// EapMethodTable to register a particular constractor for a method
/// state machine class.
template <class T> class EapMethodStateMachineCreator
{
public:
  EapMethodStateMachine* operator()(EapSwitchStateMachine &s)
  {
    return new T(s);
  };
};

/// This class instance is prepared for each method and each role.
class EAP_EXPORTS EapMethodTableEntry
{
  friend class EapMethodTable_S;
  friend class EapSwitchStateMachine;
public:
  EapMethodTableEntry(std::string name, EapType type, EapRole role, 
		      EapMethodStateMachineFunctor creator)
  {
    this->name = name;
    this->type = type;
    this->role = role;
    this->creator = creator;
  }
private:
  std::string  name;
  EapType type;
  EapRole role;
  EapMethodStateMachineFunctor creator;
};

/// A singleton for storing a list of EapMethodTableEntry instances.
class EAP_EXPORTS EapMethodTable_S : public std::list<EapMethodTableEntry*> 
{
  friend class ACE_Singleton<EapMethodTable_S, ACE_Recursive_Thread_Mutex>;
public:

  /// This function is used when searching a method table entry.
  EapMethodTableEntry *Find(EapType type, EapRole role)
  {
    iterator i;
    for (i=begin(); i!=end(); i++)
      {
	EapMethodTableEntry* e = (*i);
	if (e->type == type && e->role == role) { return e; }
      }
    return NULL;
  }

  /// This function is used when adding a method table entry.
  inline void add(EapMethodTableEntry *e) { push_back(e); }

private:
  EapMethodTable_S() { registerDefaultMethods(); };
  ~EapMethodTable_S() {};
  void registerDefaultMethods();
};

typedef ACE_Singleton<EapMethodTable_S, ACE_Recursive_Thread_Mutex>
EapMethodTable;

/// Example usage:
///      
/// EapMethodRegistrar registrar;
/// registrar.registarMethod
///   (std::string("MyMethod"), EapType(100), Peer, myPeerMethodCreator)
class EAP_EXPORTS EapMethodRegistrar
{
public:
  void registerMethod(std::string name, EapType type, EapRole role, 
		      EapMethodStateMachineFunctor creator);
};
#endif  // __EAP_METHOD_REGISTRAR_HXX__
