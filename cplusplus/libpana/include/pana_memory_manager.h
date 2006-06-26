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

#ifndef __PANA_MEMORY_MANAGER_H__
#define __PANA_MEMORY_MANAGER_H__

#include "ace/Atomic_Op.h"
#include "ace/Malloc_T.h"
#include "ace/Synch.h"
#include "ace/Message_Block.h"
#include "pana_exports.h"
#include "pana_defs.h"

class PANA_EXPORT PANA_MessageBuffer : public ACE_Message_Block
{
    public:
       PANA_MessageBuffer() :
           ACE_Message_Block(PANA_MAX_MESSAGE_SIZE,
                             MB_DATA, 0, 0,
                             AAAMemoryManager_S::instance()),
           m_RefCount(0) {
       }

        friend class PANA_MessagePoolManager;

    protected:
        ACE_Atomic_Op<ACE_Thread_Mutex, long> m_RefCount;
};

class PANA_EXPORT PANA_MessagePoolManager
{
    public:
       PANA_MessagePoolManager(int n_blocks = PANA_MIN_MESSAGE_COUNT) :
            m_Pool(NULL),  m_NumBlocks(n_blocks) {
            ACE_NEW_NORETURN(m_Pool, PANA_MessageBuffer[m_NumBlocks]);
       }
       PANA_MessageBuffer *malloc() {
            if (m_Pool) {
                PANA_MessageBuffer *buffer;
                for (int i = 0; i < m_NumBlocks; i++ ) {
                    buffer = &(m_Pool[i]);
                    if (buffer->m_RefCount.value() == 0) {
                        buffer->m_RefCount ++;
                        // reset read/write ptr
                        buffer->rd_ptr(buffer->base());
                        buffer->wr_ptr(buffer->base());
                        return (buffer);
                    }
                }
            }
            else {
                throw (PANA_Exception(PANA_Exception::NO_MEMORY, 
                       "Message pool not allocated"));
            }
            return (NULL);
       }
       void free(const PANA_MessageBuffer *buffer) {
            const_cast<PANA_MessageBuffer*>(buffer)->size(PANA_MAX_MESSAGE_SIZE);
           ((PANA_MessageBuffer*)buffer)->m_RefCount --;
       }

    private:
       PANA_MessageBuffer *m_Pool;
       int m_NumBlocks;
};

typedef ACE_Singleton<PANA_MessagePoolManager,
                      ACE_Recursive_Thread_Mutex> PANA_MessagePoolManager_S;
#define PANA_MESSAGE_POOL() PANA_MessagePoolManager_S::instance()

#endif /* __PANA_MEMORY_MANAGER_H__ */
