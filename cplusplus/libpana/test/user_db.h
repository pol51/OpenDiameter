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

#ifndef __USER_DB_H__
#define __USER_DB_H__

#include <map>
#include <string>
#include "ace/OS.h"
#include "ace/Synch.h"
#include "ace/Singleton.h"

typedef struct {
    std::string m_Username;
    std::string m_Passphrase;
    ACE_UINT32 m_AuthType;
    // TBD: Add other user profiles here
} AAA_UserEntry;

typedef std::map<std::string, AAA_UserEntry*> AAA_UserEntryDb;    
typedef std::map<std::string, AAA_UserEntry*>::iterator AAA_UserEntryDbIterator;

class AAA_UserDb {
    public:
        int open(std::string &cfg_file);
        AAA_UserEntry *lookup(const std::string &uname);
        void close();
    
    private:
        AAA_UserEntryDb m_UserDb;
};

typedef ACE_Singleton<AAA_UserDb, ACE_Null_Mutex> AAA_UserDb_S;

/*!
 * Helper macros for singleton access
 */
#define USER_DB_OPEN(x)      AAA_UserDb_S::instance()->open((x))
#define USER_DB_LOOKUP(x)    AAA_UserDb_S::instance()->lookup((x))
#define USER_DB_CLOSE()      AAA_UserDb_S::instance()->close()

#endif /* __PANA_CORE_H__ */
