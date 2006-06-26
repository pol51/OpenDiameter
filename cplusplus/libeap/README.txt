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
# $Id: README.txt,v 1.9 2004/06/17 21:13:34 yohba Exp $ 

EAP State Machine: 
    writen by Yoshihiro Ohba (yohba@tari.toshiba.com)


1. Introduction
---------------

This source code is trying to implement the EAP State Machine
specified in draft-vollbrecht-eap-state-04.{txt,ps}.  test.cxx
provides a sample code.  Manurals on this source code can be generated
by using doxygen.

License terms are described in "COPYING" file.


2. Compile Environment
----------------------

This code can be compiled on Linux OS.

To compile, run make on eapcore directory.  A test program named
"test" will be generated on successful compilation.


3. Required library
-------------------

3.1 ACE library (version 5.2.4 or higher)
 
You can get the source code of the ACE library from:
                                                                                
http://www.cs.wustl.edu/%7Eschmidt/ACE.html
                                                                                
Acknowledgment:
    The members of the Open Diameter project thank the ACE
    project for developping the ACE library for providing
    OS independency and useful design patterns.
                                                                                
3.2. g++
                                                                                
    We have successfully compiled the Open Diameter libraries
    with the following g++ versions:

    3.3.1.

    Other versions of g++ may also work.


4. Documentation
----------------

If you have doxygen, then the documentation of the code can be created
in the following way:

cd docs;
doxygen


5. To-Do list
-------------

o The following processing is yet to be handled in Peer switch state
  machine:

  - altAccept, altReject processing
  - eapKeyData processing

o Implementation of verious EAP authentication methods.


