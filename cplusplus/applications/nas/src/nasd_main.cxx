/**
 * @file nasd_main.cxx
 * @copy 2002-2014 Open Diameter Project | GNU LGPL 2.1+ - a copy of the license
 * should have been included with the corresponding code
 * @author Open Diameter Project | changes (diameter-developers@lists.sourceforge.net)
 *
 * @brief Example NASD application
 * @note EAPDEFINE is set via ./configure options
 */

#include "nasd_pana.h"
#include "nasd_diameter_eap.h"
//#include "nasd_eap_backend.h"
#include "nasd_policy_script.h"

#define NASD_DEFAULT_CFG_FILE  "/etc/opendiameter/nas/config/nasd.xml"
#define NASD_USAGE "\nUsage: nasd [cfg_file]\n cfg_file - NASD XML configuration file\n"

/// Forward declaration of cfgFile ()
std::string cfgFile(int argc, char **argv);

/**
 * main(int argc, char **argv)
 * 
 * @brief Main - everyone has one
 * @param argc
 * @param argv
 * @return 0 if there is an error (should never return)
 */
int main(int argc, char **argv)
{
	/// Set configuration file name
	std::string fname = cfgFile(argc, argv);

    /**
    * Node writers MUST add an initializer specific to their
    * instance below
    */
	NASD_PanaInitializer apPanaInit;
#ifdef EAPDEFINE
	NASD_DiameterEapInitializer aaaDiameterEapInit;
#endif
	//NASD_EapBackendInitializer aaaEapBackendInit;
	NASD_PolicyScriptInitializer plcyScriptInit;

	std::string strApPanaName("pana");
#ifdef EAPDEFINE
	std::string strAaaDiameterEapName("diameter_eap");
#endif
	//std::string strAaaEapBackendName("local_eap_auth");
	std::string strPlcyScriptName("script");

	/// Register each initializer
	NASD_CnInitializer_I->Register(strApPanaName, apPanaInit);
#ifdef EAPDEFINE
	NASD_CnInitializer_I->Register(strAaaDiameterEapName,
				       aaaDiameterEapInit);
#endif
	//NASD_CnInitializer_I->Register(strAaaEapBackendName, aaaEapBackendInit);
	NASD_CnInitializer_I->Register(strPlcyScriptName, plcyScriptInit);

	/// Start each Initializer
	if (NASD_CnInitializer_I->Start(fname.data())) {
		while (NASD_CnInitializer_I->IsRunning()) ;
		NASD_CnInitializer_I->Stop();
	}
	return (0);
}

/**
 * cfgFile(int argc, char **argv)
 * 
 * @brief Throws errors and usage if not correct
 * @param argc
 * @param argv
 * @return string
 */
std::string cfgFile(int argc, char **argv)
{
	std::string fname = NASD_DEFAULT_CFG_FILE;
	try {
		if (argc == 2) {
			fname = argv[1];
			throw(fname == "--help") ? -1 : 0;
		}
		throw(argc == 1) ? 0 : -1;
	}
	catch(int rc) {
		if (rc < 0) {
			NASD_LOG(LM_INFO, "%s\n", NASD_USAGE);
			exit(0);
		}
	}
	return fname;
}
