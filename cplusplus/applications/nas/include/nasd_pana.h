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

#ifndef __NASD_PANA_H__
#define __NASD_PANA_H__

#include "nasd_call_framework.h"
#include "pana_node.h"
#include "pana_paa_factory.h"
#include "nasd_eap_passthrough.h"
#include "pana_pac_ep_key.h"
#include "eap_fast.hxx"

#define GTC_METHOD_TYPE 6

class NASD_PanaSessionFactory:public PANA_PaaSessionFactory {
 public:
	NASD_PanaSessionFactory(PANA_Node & n):PANA_PaaSessionFactory(n) {
	} PANA_PaaSession *Create();
};

class NASD_PanaInitializer:public NASD_CnInitCallback {
 public:
	virtual bool Initialize(AAA_Task & t);
	virtual bool UnInitialize();

 private:
	 std::auto_ptr < PANA_Node > m_Node;
	 std::auto_ptr < NASD_PanaSessionFactory > m_Factory;
};

#endif				// __NASD_PANA_H__
