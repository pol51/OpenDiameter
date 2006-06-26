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

#ifndef __PANA_SERIAL_NUM_H__
#define __PANA_SERIAL_NUM_H__

#include "ace/OS.h"
#include "ace/System_Time.h"

/*!
   Serial number implementation as defined in RFC1982

 2.	Serial Number Arithmetic

   Serial numbers are formed from non-negative integers	from a finite
   subset of the range of all integer values.  The lowest integer in
   every subset	used for this purpose is zero, the maximum is always one
   less	than a power of	two.

   When	considered as serial numbers however no	value has any particular
   significance, there is no minimum or	maximum	serial number, every
   value has a successor and predecessor.

   To define a serial number to	be used	in this	way, the size of the
   serial number space must	be given.  This	value, called "SERIAL_BITS",
   gives the power of two which	results	in one larger than the largest
   integer corresponding to	a serial number	value.	This also specifies
   the number of bits required to hold every possible value	of a serial
   number of the defined type.	The	operations permitted upon serial
   numbers are defined in the following	section.

3. Operations upon the serial number

   Only two operations are defined upon serial numbers, addition of a
   positive integer of limited range, and comparison with another serial
   number.

 */
class PANA_SerialNumber
{
    private:
        typedef enum {
            SERIAL_BITS = 32,
            // Don't want to deal with size issues so we
            // shorten this to (2 ^ (32 -1 ))
            SERIAL_VALUE = 0xFFFFFFFF, // (2 ^ SERIAL_BITS) - 1
            SERIAL_VALUE_LESS_ONE = 0x80000000, // 2 ^ (SERIAL_BITS -1)
        };

    public:
        PANA_SerialNumber(ACE_UINT32 seed = 0) : 
           m_SerialNum(seed) { 
           if (m_SerialNum == 0) {
               Reset();
           }
        }

      
        /*
           Serial numbers may be incremented by	the	addition of	a positive
           integer n, where	n is taken from	the	range of integers
           [0 .. (2^(SERIAL_BITS - 1) -	1)].  For a	sequence number	s, the
           result of such an addition, s', is defined as

             s' =	(s + n)	modulo (2 ^	SERIAL_BITS)

           where the addition and modulus operations here act upon values that
           are non-negative	values of unbounded	size in	the	usual ways of
           integer arithmetic.

           Addition	of a value outside the range
           [0 .. (2^(SERIAL_BITS - 1) -	1)]	is undefined.
        */
        ACE_UINT32 operator+(ACE_UINT32 num) {
           return (m_SerialNum = (m_SerialNum + num) % SERIAL_VALUE_LESS_ONE);
        }
        ACE_UINT32 operator+(PANA_SerialNumber &num) { 
            return ((*this) + num.Value()); 
        }
        ACE_UINT32 operator++() { 
            return ((*this) + 1); 
        }

        /*
          3.2. Comparison

          Any two serial numbers, s1 and s2, may be compared.  The definition
          of the result of this comparison is as follows.

          For the purposes of this definition, consider two integers, i1 and
          i2, from the unbounded set of non-negative integers, such that i1 and
          s1 have the same numeric value, as do i2 and s2.  Arithmetic and
          comparisons applied to i1 and i2 use ordinary unbounded integer
          arithmetic.

          Then, s1 is said to be equal to s2 if and only if i1 is equal to i2,
          in all other cases, s1 is not equal to s2.

          s1 is said to be less than s2 if, and only if, s1 is not equal to s2,
          and

               (i1 < i2 and i2 - i1 < 2^(SERIAL_BITS - 1)) or
               (i1 > i2 and i1 - i2 > 2^(SERIAL_BITS - 1))

          s1 is said to be greater than s2 if, and only if, s1 is not equal to
          s2, and

               (i1 < i2 and i2 - i1 > 2^(SERIAL_BITS - 1)) or
               (i1 > i2 and i1 - i2 < 2^(SERIAL_BITS - 1))

          Note that there are some pairs of values s1 and s2 for which s1 is
          not equal to s2, but for which s1 is neither greater than, nor less
          than, s2.  An attempt to use these ordering operators on such pairs
          of values produces an undefined result.

          The reason for this is that those pairs of values are such that any
          simple definition that were to define s1 to be less than s2 where
          (s1, s2) is such a pair, would also usually cause s2 to be less than
          s1, when the pair is (s2, s1).  This would mean that the particular
          order selected for a test could cause the result to differ, leading
          to unpredictable implementations.

          While it would be possible to define the test in such a way that the
          inequality would not have this surprising property, while being
          defined for all pairs of values, such a definition would be

          unnecessarily burdensome to implement, and difficult to understand,
          and would still allow cases where

               s1 < s2 and (s1 + 1) > (s2 + 1)

          which is just as non-intuitive.

          Thus the problem case is left undefined, implementations are free to
          return either result, or to flag an error, and users must take care
          not to depend on any particular outcome.  Usually this will mean
          avoiding allowing those particular pairs of numbers to co-exist.

          The relationships greater than or equal to, and less than or equal
          to, follow in the natural way from the above definitions.
        */
        ACE_UINT32 operator==(ACE_UINT32 num) {
           return (m_SerialNum == num);
        }
        ACE_UINT32 operator==(PANA_SerialNumber &num) { 
           return ((*this) == num.Value()); 
        }
        ACE_UINT32 operator<(ACE_UINT32 num) {            
            return (((m_SerialNum < num) && 
                     ((num - m_SerialNum) < SERIAL_VALUE_LESS_ONE)) || 
                    ((m_SerialNum > num) && 
                     ((m_SerialNum - num) > SERIAL_VALUE_LESS_ONE)));
        }
        ACE_UINT32 operator<(PANA_SerialNumber &num) { 
            return ((*this) < num.Value()); 
        }
        ACE_UINT32 operator>(ACE_UINT32 num) {
            return (((m_SerialNum < num) && 
                     ((num - m_SerialNum) > SERIAL_VALUE_LESS_ONE)) ||
                    ((m_SerialNum > num) && 
                     ((m_SerialNum - num) < SERIAL_VALUE_LESS_ONE)));
        }
        ACE_UINT32 operator>(PANA_SerialNumber &num) { 
            return ((*this) > num.Value()); 
        }
        PANA_SerialNumber &operator=(ACE_UINT32 num) { 
            m_SerialNum = num;
            return (*this);
        }
        ACE_UINT32 Value() { 
            return m_SerialNum; 
        }
        ACE_UINT32 Reset() { 
            return (m_SerialNum = GenerateISN());
        }
        static inline ACE_UINT32 GenerateISN(ACE_UINT32 seed = 0)  {
            if (seed == 0) {
                ACE_System_Time::get_local_system_time(seed);
            }
            // simple time seeded randon number generator
            ACE_OS::srand(seed + ACE_OS::rand());
            return ACE_OS::rand();
        }

    protected:
        ACE_UINT32 m_SerialNum;
};

#endif /* __PANA_SERIAL_NUM_H__ */

