/*
 * EAP peer method: EAP-FAST PAC file processing
 * Copyright (c) 2004-2007, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef EAP_FAST_PAC_H
#define EAP_FAST_PAC_H

#include "base/eap_fast_common.h"

#ifdef __cplusplus
extern "C"{
#endif

void eap_fast_free_pac(struct eap_fast_pac *pac);

int eap_fast_add_pac(struct eap_fast_pac **pac_root,
		     struct eap_fast_pac **pac_current,
		     struct eap_fast_pac *entry);
int eap_fast_load_pac(struct eap_fast_pac **pac_root,
		      const char *pac_file);
int eap_fast_save_pac(struct eap_fast_pac *pac_root,
		      const char *pac_file);
size_t eap_fast_pac_list_truncate(struct eap_fast_pac *pac_root,
				  size_t max_len);
int eap_fast_load_pac_bin(struct eap_fast_pac **pac_root,
			  const char *pac_file);
int eap_fast_save_pac_bin(struct eap_fast_pac *pac_root,
			  const char *pac_file);

#ifdef __cplusplus
}
#endif

#endif /* EAP_FAST_PAC_H */
