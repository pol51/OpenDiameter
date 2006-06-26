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

/*!
 * PANA global exception class definitions
 */

#ifndef __PANA_EXCEPTIONS_H__
#define __PANA_EXCEPTIONS_H__

#include <string>

/*!
 * Exception class for errors and return codes.
 * Note that this class is throwable.
 */
class PANA_Exception 
{
   public:
      /*!
       * Exception code enumeration. New error codes
       * MUST be appended here
       */
      typedef enum {
         SUCCESS = 0,
         FAILED,
         NO_MEMORY,
         THREAD_FAILED,
         ENQUEUE_FAILED,
         TRANSPORT_FAILED,
         MESSAGE_PARSING_ERROR,
         AVP_FACTORY_ERROR,
         STATIC_GUARD_TRIGGER,
         DATABASE_ERROR,
         PARSING_ERROR,
         SESSIONID_ERROR,
         ENTRY_NOT_FOUND,
         CONFIG_ERROR,
         INVALID_MESSAGE,
         MISSING_EAP_PAYLOAD
      } CODE;

   public:
      /*!
       * constructor
       *
       * \param code Default  code
       * \param description Sting description of the error code
       */
       PANA_Exception(CODE code, std::string &description) : 
                      code_(code),
                      description_(description) { }

      /*!
       * constructor
       *
       * \param code Default  code
       * \param description Sting description of the error code
       */
       PANA_Exception(CODE code, const char *description) : 
                      code_(code),
                      description_(description) { }

       /*!
        * Access method for getting current
        * code exception value
        */
       CODE code() {  return code_; }

       /*!
        * Access method for modifying current
        * code exception value
        *
        * \param code New exception code
        */
       void code(CODE code) { code_ = code; }

       /*!
        * Access methods to string description
        */
       std::string &description() { return description_; }

       /*!
        * Access methods to setting string description
        *
        * \param description New descripton for this error code
        */
       void description(std::string &description) { description_ = description; }

   private:
       CODE code_; /*<< Current exception code */

       std::string description_; /*<< Description of current error */
};

#endif /* __PANA_EXCEPTIONS_H__ */

