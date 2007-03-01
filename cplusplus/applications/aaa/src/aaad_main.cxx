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

#include "aaad_config.h"
#include "aaad_call_framework.h"

/// application headers
#include "aaad_diameter_eap.h"

#define AAAD_DEFAULT_CFG_FILE  "/etc/opendiameter/aaa/config/aaad.xml"
#define AAAD_USAGE "\nUsage: aaad [cfg_file]\n\
             cfg_file - AAAD XML configuration file\n"

std::string cfgFile(int argc, char **argv);

int main(int argc, char **argv)
{
    /// ----------------------------------------
    std::string fname = cfgFile(argc, argv);

    /// ------ Define your application here ----
    AAAD_AppDiameterEap diamEap;

    /// ------ Register applications here ------
    AAAD_FRAMEWORK().Register(diamEap);

    /// ------ Application main loop -----------
    AAAD_FRAMEWORK().Start(fname);
    while (AAAD_FRAMEWORK().IsRunning()) {
    }
    AAAD_FRAMEWORK().Stop();

    return (0);
}

std::string cfgFile(int argc, char **argv)
{
    std::string fname = AAAD_DEFAULT_CFG_FILE;
    try {
        if (argc == 2) {
            fname = argv[1];
            throw (fname == "--help") ? -1 : 0;
        }
        throw (argc == 1) ? 0 : -1;
    }
    catch (int rc) {
        if (rc < 0) {
            AAAD_LOG(LM_INFO, "%s\n", AAAD_USAGE);
            exit (0);
	}
    }
    return fname;
}
