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

#ifndef __AAA_MEMORY_MANAGER_H__
#define __AAA_MEMORY_MANAGER_H__

#include "diameter_parser_api.h"

/*!
 * AAA Message Manager
 *
 * This class provides very efficient memory management
 * for fixed sized message chunks. Internal implementations
 * MUST use this manager to allocate raw message buffers
 * to store AAA formatted packets during reception and
 * transmission. An instances of this object manages n 
 * number of pre-allocated chunks that it lends it's users.
 * The number of chunks is configurable at compile-time 
 * with the macro AAA_MIN_MESSAGE_COUNT. Care MUST be 
 * taken in choosing a number large enough so as not to 
 * starve the application requiring a large number of 
 * chuncks but small enough so as not to over allocate 
 * and have chunks that may never be used.
 *
 * It is important to note that a call to alloc() on this
 * object will return a fixed size chunks and that the
 * formatted AAA message MUST not exceed that size. The
 * size of these chunks is configurable at compile-time 
 * only. Care MUST be taken in choosing a size that will 
 * be larger than all possible packet size that AAA will 
 * send or recieve. If adjustments need to be made, adjust 
 * the AAA_MAX_MESSAGE_SIZE macro in defs.h
 */
class AAAMessageManager
{
    public:
        /*!
         * Definitions for default block count
         */
        typedef enum {
	   AAA_MIN_MESSAGE_COUNT = 512
        };

    public:
        /*!
         * constructor
         *
         * \param n_blocks Number of blocks to manage
         */
        AAAMessageManager(int n_blocks = AAA_MIN_MESSAGE_COUNT);

	/*!
	 * destructor
	 */
	~AAAMessageManager();

        /*!
         * Access function to message singleton
         */
        static AAAMessageManager *instance() { return &AAAMessageManager::allocator_; }

        /*!
         * Allocates an un-used message buffer 
         */
        AAAMessageBlock *malloc();

        /*!
         * Returns a message buffer to the free list
         */
        void free(AAAMessageBlock *buffer);

    private:
        static AAAMessageManager allocator_; /*<< Singleton instance of the allocator */

        AAAMessageBlock **pool_; /*<< Message pool */

        int num_blocks_; /*<< number of message buffers in the pool */
};

#endif /* __AAA_MEMORY_MANAGER_H__ */
