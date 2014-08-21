/**
 * @file main.c
 * @author Dr. Li Bing, WPA supplicant code by Journi and cleanup by Ron Brash
 * @brief Creates a single PAC using user input
 * @note PAC stands for Protected Access Credential
 */
#include "aes_wrap.h"
#include "eap_fast_generate_pac.h"

int main()
{

	uint8_t a_id[128], i_id[128], a_id_info[128];
	uint8_t encr_key_str[PAC_ENCR_KEY_LEN * 2 + 1],
	    encr_key[PAC_ENCR_KEY_LEN];
	size_t encr_key_len;
	int lifetime;
	int unused __attribute__ ((unused));

	do {
		printf("Input the A-ID:");
		unused = scanf("%s", a_id);
		if (strlen((const char *)a_id) >= 128) {
			printf("The A-ID is too long!\n");
		}
	} while (strlen((const char *)a_id) >= 128);

	do {
		printf("Input the I-ID:");
		unused = scanf("%s", i_id);
		if (strlen((const char *)i_id) >= 128) {
			printf("The I-ID is too long!\n");
		}
	} while (strlen((const char *)i_id) >= 128);

	do {
		printf("Input the A-ID_INFO:");
		unused = scanf("%s", a_id_info);
		if (strlen((const char *)a_id_info) >= 128) {
			printf("The A-ID_INFO is too long!\n");
		}
	} while (strlen((const char *)a_id_info) >= 128);

	do {
		printf
		    ("Input the encryption key (16 BYTE) with hex string( 000102030405060708090a0b0c0d0e0f ):");
		unused = scanf("%s", encr_key_str);
		encr_key_len = strlen((const char *)encr_key_str);
		if ((encr_key_len != 16 * 2) ||
		    (hexstr2bin((const char *)encr_key_str, encr_key, 16) != 0))
		{
			printf("invalid key\n");
		}
	} while ((encr_key_len != 16 * 2) ||
		 (hexstr2bin((const char *)encr_key_str, encr_key, 16) != 0));

	printf("Input the lifetime of PAC (min):");
	unused = scanf("%i", &lifetime);

	struct eap_fast_pac *pac = eap_fast_build_pac(a_id, i_id, a_id_info,
						      lifetime * 60, encr_key);

	if (pac == NULL) {
		printf("Unable to build PAC!\n");
		return (-1);
	}

	const char file[] = "pac";

	printf("Writing PAC to file\n");
	eap_fast_save_pac_bin(pac, file);

	return 0;
}
