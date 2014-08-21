/**
 * @file os.h
 * 
 * OS specific functions
 * Copyright (c) 2005-2009, Jouni Malinen <j@w1.fi>
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

#ifndef OS_H
#define OS_H
#include <stdio.h>
#include <stdlib.h>

#include <memory.h>
#include <openssl/ssl.h>
#include <sys/time.h>
#include <endian.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#define TRUE 	1
#define FALSE 	0
#define WPA_GET_BE16(a) ((uint16_t) (((a)[0] << 8) | (a)[1]))
#define WPA_PUT_BE16(a, val)			\
	do {					\
		(a)[0] = ((uint16_t) (val)) >> 8;	\
		(a)[1] = ((uint16_t) (val)) & 0xff;	\
	} while (0)
#define WPA_GET_BE32(a) ((((uint32_t) (a)[0]) << 24) | (((uint32_t) (a)[1]) << 16) | \
			 (((uint32_t) (a)[2]) << 8) | ((uint32_t) (a)[3]))
#define WPA_PUT_BE32(a, val)					\
	do {							\
		(a)[0] = (uint8_t) ((((uint32_t) (val)) >> 24) & 0xff);	\
		(a)[1] = (uint8_t) ((((uint32_t) (val)) >> 16) & 0xff);	\
		(a)[2] = (uint8_t) ((((uint32_t) (val)) >> 8) & 0xff);	\
		(a)[3] = (uint8_t) (((uint32_t) (val)) & 0xff);		\
	} while (0)
#define os_memcpy(a,b,c) memcpy(a,b,c)
#define os_malloc(a)	malloc(a)
#define os_free(a)	free(a)
#define os_memset(a,b,c)	memset(a,b,c)
#define os_strlen(a)	strlen(a)
#define os_memmove(d, s, n) memmove((d), (s), (n))

#if __BYTE_ORDER == __BIG_ENDIAN

#define host_to_be32(n) (n)
#define host_to_be16(n)	(n)

#elif  __BYTE_ORDER == __LITTLE_ENDIAN
#define host_to_be32(n) ((n&0xff000000)>>24)+((n&0x00ff0000)>>8)+((n&0x0000ff00)<<8)+((n&0x000000ff)<<24)
#define host_to_be16(n)	((n&0xff00)>>8)+((n&0x00ff)<<8)

#endif

typedef long os_time_t;
#define debug_level 	0
#define MSG_DEBUG	1
#define show_keys	1
struct os_time {
	os_time_t sec;
	os_time_t usec;
};

enum {
	TLS_CIPHER_NONE,
	TLS_CIPHER_RC4_SHA /* 0x0005 */ ,
	TLS_CIPHER_AES128_SHA /* 0x002f */ ,
	TLS_CIPHER_RSA_DHE_AES128_SHA /* 0x0031 */ ,
	TLS_CIPHER_ANON_DH_AES128_SHA	/* 0x0034 */
};

char *os_readfile(const char *name, size_t * len);
int hexstr2bin(const char *hex, uint8_t * buf, size_t len);
int wpa_snprintf_hex(char *buf, size_t buf_size, const uint8_t * data,
		     size_t len);
void hexdump(int level, const char *title, const uint8_t * buf, size_t len);
int os_get_random(unsigned char *buf, size_t len);
int os_get_time(struct os_time *t);
#endif				/* OS_H */
