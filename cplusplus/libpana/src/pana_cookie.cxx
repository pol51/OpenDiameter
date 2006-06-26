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

#include "ace/System_Time.h"
#include "pana_cookie.h"
#include "od_utl_sha1.h"

#define PANA_COOKIE_DEBUG 0

PANA_Cookie::PANA_Cookie()
{
    char sbuf[32];
    ACE_UINT32 ltime;
    ACE_System_Time::get_local_system_time(ltime);

    /* --- seed and generate secret table --- */
    ACE_OS::srand(ltime + ACE_OS::rand());
    for (int i = 0; i < default_table_size_; i ++) {
        ACE_OS::sprintf(sbuf, "%0.4x", ACE_OS::rand());
        s_table_[i] = sbuf;
    }

    s_index_ = 0; // initial index
}

bool PANA_Cookie::Generate(std::string &id, std::string &digest)
{
    return Generate(s_index_, id, digest);
}

bool PANA_Cookie::Generate(ACE_UINT32 ndx, std::string &id, std::string &digest)
{
    unsigned char sbuf[32];
    ACE_OS::memset(sbuf, 0x0, sizeof(sbuf));
    ACE_OS::memcpy(sbuf, &ndx, sizeof(ACE_UINT32));

    OD_Utl_Sha1 sha1;
    sha1.Update((unsigned char*)id.data(), id.size());
    sha1.Update((unsigned char*)s_table_[ndx].data(), sizeof(ACE_UINT32));
    sha1.Final();

    sha1.GetHash(sbuf + sizeof(ACE_UINT32));
    digest.assign((char*)sbuf, 20 + 2 * sizeof(ACE_UINT32)); // sha1 hash + index

#if PANA_COOKIE_DEBUG
    printf("Id : ", ndx);
    for (int i = 0; i < id.size(); i++) {
       printf("%02X ", ((unsigned char*)id.data())[i]);
    }
    printf("\n");
    printf("Generated Cookie [%d]: ", ndx);
    for (int i = 0; i < digest.size(); i++) {
       printf("%02X ", ((unsigned char*)digest.data())[i]);
    }
    printf("\n");
#endif

    return (true);
}

bool PANA_Cookie::Verify(std::string &id, std::string &digest)
{
#if PANA_COOKIE_DEBUG
    printf("Verifying: ");
    for (int i = 0; i < digest.size(); i++) {
       printf("%02X ", ((unsigned char*)digest.data())[i]);
    }
    printf("\n");
#endif

    std::string testDigest;
    for (int i = 0; i < default_table_size_; i ++) {
        Generate(i, id, testDigest);
        // 24 byte sha1 hash value comparison
        if (ACE_OS::memcmp(testDigest.data(), digest.data(), 20 + 2 * sizeof(ACE_UINT32)) == 0) {
            return (true);
        }
    }
    return (false);
}

void PANA_Cookie::Cycle()
{
    s_index_ ++;
    if (s_index_ > default_table_size_) {
        s_index_ = 0;
    }
}
