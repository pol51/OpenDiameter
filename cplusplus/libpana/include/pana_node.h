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

#ifndef __PANA_NODE_H__
#define __PANA_NODE_H__

#include "pana_exports.h"
#include "framework.h"

/* \brief PANA Node interface
 *
 * An instance of this class represents an instance of
 * a PANA protocol entity. Note that the role for this
 * entity is required at creation.
 *
 * The PANA core is an active object entity with an 
 * underlying thread pool. Provisions on starting,
 * pausing, resuming and stopping protocol functions
 * are based on internal thread pool execution states.
 *
 */
class PANA_EXPORT PANA_Node
{
   public:
       PANA_Node(AAA_Task &t, std::string &cfg_file) : 
           m_Task(t) {
           Start(cfg_file);
       }
       virtual ~PANA_Node() {
           Stop();
       }
       AAA_Task &Task() {
           return m_Task;
       }
       AAA_GroupedJob &Job() {
           return m_Task.Job();
       }

   protected:
       void Start(std::string &cfg_file);
       void Stop();

   private:
       AAA_Task &m_Task;
};

#endif /* __PANA_NODE_H__ */
