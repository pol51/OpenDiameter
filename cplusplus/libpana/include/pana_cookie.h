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

#ifndef __PANA_COOKIE_H__
#define __PANA_COOKIE_H__

#include "ace/Synch.h"
#include "ace/Singleton.h"
#include <string>

class PANA_Cookie
{
    public:
        typedef enum {
            default_table_size_ = 5,
        };

    public:
        /*!
         * Constructor 
         */
        PANA_Cookie();

        /*!
         * destructor
         */
        virtual ~PANA_Cookie() { }

        /*!
         * Generates a PANA cookie given a device ID
         *
         * \param id Device ID
         * \param digest Generated digest
         */
        bool Generate(std::string &id, std::string &digest);

        /*!
         * Verifies a cookie
         *
         * \param id Device ID
         * \param digest Generated digest
         */
        bool Verify(std::string &id, std::string &digest);

        /*!
         * Adjust index
         */
        void Cycle();

    protected:
        /*!
         * Generates a PANA cookie given a device ID
         *
         * \param ndx secret index
         * \param id Device ID
         * \param digest Generated digest
         */
        bool Generate(ACE_UINT32 ndx, std::string &id, std::string &digest);

    private:

        ACE_UINT32 s_index_; /**< current secret version */

        std::string s_table_[default_table_size_]; /**< version table */
};

/*! 
 * Singleton declaration of the session database
 */
typedef ACE_Singleton<PANA_Cookie, ACE_Null_Mutex> PANA_Cookie_S;

/*!
 * Helper macros for singleton access
 */
#define PANA_COOKIE_GENERATE(x, y) PANA_Cookie_S::instance()->Generate((x), (y))
#define PANA_COOKIE_VERIFY(x, y)   PANA_Cookie_S::instance()->Verify((x), (y))
#define PANA_COOKIE_CYCLE()        PANA_Cookie_S::instance()->Cycle()

#endif // __PANA_COOKIE_H__

