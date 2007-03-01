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

#ifndef __OD_UTL_SHA1_H__
#define __OD_UTL_SHA1_H__

#include "od_utl_exports.h"

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN
#endif // LITTLE_ENDIAN

class OD_UTL_EXPORT OD_Utl_Sha1
{
    public:
        /*!
         * Constructor 
         */
	    OD_Utl_Sha1();

        /*!
         * destructor
         */
	    virtual ~OD_Utl_Sha1();

        // Rotate x bits to the left
        #define ROL32(value, bits) (((value)<<(bits))|((value)>>(32-(bits))))

        #ifdef LITTLE_ENDIAN
	        #define SHABLK0(i) (m_block->l[i] = \
		        (ROL32(m_block->l[i],24) & 0xFF00FF00) | (ROL32(m_block->l[i],8) & 0x00FF00FF))
        #else
	        #define SHABLK0(i) (m_block->l[i])
        #endif

        #define SHABLK(i) (m_block->l[i&15] = ROL32(m_block->l[(i+13)&15] ^ m_block->l[(i+8)&15] \
	        ^ m_block->l[(i+2)&15] ^ m_block->l[i&15],1))

        // SHA-1 rounds
        #define R0(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK0(i)+0x5A827999+ROL32(v,5); w=ROL32(w,30); }
        #define R1(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK(i)+0x5A827999+ROL32(v,5); w=ROL32(w,30); }
        #define R2(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0x6ED9EBA1+ROL32(v,5); w=ROL32(w,30); }
        #define R3(v,w,x,y,z,i) { z+=(((w|x)&y)|(w&x))+SHABLK(i)+0x8F1BBCDC+ROL32(v,5); w=ROL32(w,30); }
        #define R4(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0xCA62C1D6+ROL32(v,5); w=ROL32(w,30); }

        typedef union {
	        unsigned char c[64];
	        unsigned long l[16];
        } SHA1_WORKSPACE_BLOCK;

        // Two different formats for ReportHash(...)
        enum {
	        REPORT_HEX = 0,
	        REPORT_DIGIT = 1
        };

	    unsigned long m_state[5];
	    unsigned long m_count[2];
	    unsigned char m_buffer[64];
	    unsigned char m_digest[20];

        /*!
         * Reset hasing
         */
	    void Reset();

	    /*!
         * Update the hash value
         *
         * \param data Data to add
         * \param len length of data
         */
	    void Update(unsigned char* data, unsigned int len);

	    /*!
         * Finalize hash and report
         */
	    void Final();

        /*!
         * Retrieves the message digest
         *
         * \param report message digest
         * \param type report type
         */
	    void ReportHash(char *report, unsigned char type = REPORT_HEX);

        /*!
         * gets the digest in binary format
         *
         * \param dest Destination of digest
         */
	    void GetHash(unsigned char *uDest);

    private:
        /*!
         * Private SHA-1 transformation
         */
        void Transform(unsigned long state[5], unsigned char buffer[64]);

        unsigned char m_workspace[64]; /**< workspace */

        SHA1_WORKSPACE_BLOCK* m_block; /**< SHA1 pointer to the byte array above */
};

#endif // __OD_UTL_SHA1_H__
