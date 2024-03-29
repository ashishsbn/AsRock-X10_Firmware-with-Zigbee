/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

//#include <osal_common.h>
#include <sys_base.h>
#include "cli_lib.h"

/******************************************************************************
 *
 * description:
 *   convert str to MAC address, like HHHH.HHHH.HHHH
 *
 * input:
 *   str - address string
 *
 * output:
 *   mac - pointer to MAC address
 *
 * return:
 *   0  - okay
 *   -1 - invalid MAC string
 *
 ******************************************************************************/
int cli_str_2_mac(const char *str, sa_u8_t * mac)
{
#define CLI_MAC_STR_CHARS    "0123456789abcdefABCDEF."

	int strLen;
	int i;
	char valStr[8];
	int valLen;
	sa_u32_t val;
	const char *cp;
	const char *pDot;	/* pointer to '.' or '\0' */
	char *endPtr;

	/* parameter check */
	if (NULL == str) {
		return -1;
	}

	/* string length check */
	strLen = strlen(str);
	if ((strLen < CLI_MAC_STR_LEN_MIN)
	    || (strLen > CLI_MAC_STR_LEN_MAX)) {
		return -1;
	}

	/* character check */
	if (strspn(str, CLI_MAC_STR_CHARS) != strLen) {
		return -1;
	}

	cp = str;

	for (i = 0; i < 3; i++) {
		if (i < 2) {
			pDot = strchr(cp, '.');
		} else {
			pDot = cp + strlen(cp);
		}

		valLen = pDot - cp;

		if ((NULL == pDot)
		    || (valLen <= 0)
		    || (valLen > 4)) {
			return -1;
		}

		memcpy(valStr, cp, valLen);
		valStr[valLen] = '\0';

		val = strtoul(valStr, &endPtr, 16);
		if ((*endPtr != '\0')
		    || (val > 0xffff)) {
			return -1;
		}

		if (mac != NULL) {
			mac[2 * i] = (sa_u8_t) ((val >> 8) & 0xff);
			mac[2 * i + 1] = (sa_u8_t) (val & 0xff);
		}

		cp = pDot + 1;
	}

	return 0;
}

/******************************************************************************
 *
 * description:
 *   convert MAC address to str, like "1111.2222.3333"
 *
 * input:
 *   mac - pointer to MAC address
 *
 * output:
 *   str  - string buf to contain result
 *
 * return:
 *   0  - okay
 *   -1 - fail
 *
 ******************************************************************************/
int cli_mac_2_str(const sa_u8_t * mac, char *str)
{
	if ((NULL == mac) || (NULL == str)) {
		return -1;
	}

	sprintf(str, "%02x%02x.%02x%02x.%02x%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	return 0;
}

/******************************************************************************
 *
 * description:
 *   convert u8 array to str, like "11112222.3333ffff.eeeeaaaa"
 *
 * input:
 *   array    - pointer to u8 array
 *   arrayNum - number of u8
 *   strLen   - str length
 *
 * output:
 *   str  - string buf to contain result
 *
 * return:
 *   0  - okay
 *   -1 - fail
 *
 ******************************************************************************/
int cli_u8_array_2_str(const sa_u8_t * array, int arrayNum, char *str,
		       int strLen)
{
	int i;
	char tempStr[16];

	if ((NULL == array) || (NULL == str)) {
		return -1;
	}

	if (strLen < (arrayNum * 2 + arrayNum / 4)) {
		snprintf(str, strLen, "short buffer");
		return -1;
	}

	for (i = 0; i < arrayNum; i++) {
		sprintf(tempStr, "%02x", array[i]);

		if (((i % 4) == 3) && (i != (arrayNum - 1))) {
			strcat(tempStr, ".");
		}

		strcat(str, tempStr);
	}

	return 0;
}

#if 0
int cli_str_2_sak(const char *str, sa_u8_t * sak)
{
	int i;
	int strLen = strlen(str);
	sa_u32_t key_addr[4];

	if (str_len != 35) {
		return -1;
	}

	if (strspn(str, "1234567890abcdefABCDEF:") != str_len) {
		return -1;
	}

	for (i = 0; i < str_len; i++) {
		if (i == 8 || i == 17 || i == 26) {
			if (str[i] != ':') {
				return -1;
			}
		} else {
			if (str[i] == ':') {
				return -1;
			}
		}
	}

	sscanf(str, "%8x:%8x:%8x:%8x", (sa_u32_t *) & key_addr[0],
	       (sa_u32_t *) & key_addr[1],
	       (sa_u32_t *) & key_addr[2], (sa_u32_t *) & key_addr[3]);

	for (i = 0; i < 4; i++) {
		key[i * 4] = (key_addr[i] >> 24) & 0xff;
		key[i * 4 + 1] = (key_addr[i] >> 16) & 0xff;
		key[i * 4 + 2] = (key_addr[i] >> 8) & 0xff;
		key[i * 4 + 3] = (key_addr[i]) & 0xff;
	}

	return 0;
}
#endif

/******************************************************************************
 *
 * description:
 *   convert str to IPv4 address
 *
 * input:
 *   str - address string
 *
 * output:
 *   ip - pointer to IPv4 address
 *
 * return:
 *   0  - okay
 *   -1 - invalid ip string
 *
 ******************************************************************************/
int cli_str_2_ipv4(const char *str, sa_u8_t * ipv4)
{
#define CLI_IPV4_ADDR_CHARS     "0123456789."

	int strLen;
	int i;
	char valStr[4];
	int valLen;
	sa_u32_t val;
	const char *cp;
	const char *pDot;	/* pointer to '.' or '\0' */
	char *endPtr;

	/* parameter check */
	if (NULL == str) {
		return -1;
	}

	/* string length check */
	strLen = strlen(str);
	if ((strLen < CLI_IPV4_STR_LEN_MIN)
	    || (strLen > CLI_IPV4_STR_LEN_MAX)) {
		return -1;
	}

	/* character check */
	if (strspn(str, CLI_IPV4_ADDR_CHARS) != strLen) {
		return -1;
	}

	cp = str;

	for (i = 0; i < 4; i++) {
		if (i < 3) {
			pDot = strchr(cp, '.');
		} else {
			pDot = cp + strlen(cp);
		}

		valLen = pDot - cp;

		if ((NULL == pDot)
		    || (valLen <= 0)
		    || (valLen > 3)) {
			return -1;
		}

		memcpy(valStr, cp, valLen);
		valStr[valLen] = '\0';

		val = strtoul(valStr, &endPtr, 10);
		if ((*endPtr != '\0')
		    || (val > 255)) {
			return -1;
		}

		if (ipv4 != NULL) {
			ipv4[i] = (sa_u8_t) val;
		}

		cp = pDot + 1;
	}

	return 0;
}

/******************************************************************************
 *
 * description:
 *   convert ipv4 address to str, like "192.168.1.1"
 *
 * input:
 *   ipv4 - pointer to IPv4 address
 *
 * output:
 *   str  - string buf to contain result
 *
 * return:
 *   0  - okay
 *   -1 - fail
 *
 ******************************************************************************/
int cli_ipv4_2_str(const sa_u8_t * ipv4, char *str)
{
	if ((NULL == ipv4) || (NULL == str)) {
		return -1;
	}

	sprintf(str, "%d.%d.%d.%d", ipv4[0], ipv4[1], ipv4[2], ipv4[3]);

	return 0;
}

/******************************************************************************
 *
 * description:
 *   convert str to IPv6 address
 *
 * input:
 *   str - address string
 *
 * output:
 *   ip - pointer 16 bytes for IPv6 address
 *
 * return:
 *   0  - okay
 *   -1 - invalid IPv6 string
 *
 ******************************************************************************/
int cli_str_2_ipv6(const char *str, sa_u8_t * ipV6)
{
#define CLI_IPV6_ADDR_LEN   16
#define CLI_IPV6_ADDR_CHARS "0123456789abcdefABCDEF:.%[]"

	sa_u16_t val = 0;
	sa_u32_t tmpVal;
	int zero = -1;		/* zero compress start index */
	int ipIndex = 0;
	char c;
	int strLen;
	int i;

	/* parameter check */
	if (NULL == str) {
		return -1;
	}

	strLen = strlen(str);
	if ((strLen < CLI_IPV6_STR_LEN_MIN)
	    || (strLen > CLI_IPV6_STR_LEN_MAX)) {
		return -1;
	}

	/* character check */
	if (strspn(str, CLI_IPV6_ADDR_CHARS) != strLen) {
		return -1;
	}

	/* check [X:X::X:X] case */
	if ('[' == *str) {
		if (str[strLen - 1] != ']') {
			/* [ and ] dismatch */
			return -1;
		}
		str++;
	}

	/* handle every character */
	for (i = 0; i <= strLen; i++) {
		c = str[i];

		if ((':' == c) || ('\0' == c) || (']' == c)) {
			if (ipV6 != NULL) {
				ipV6[ipIndex] = (val >> 8) & 0xff;
				ipV6[ipIndex + 1] = val & 0xff;
			}
			ipIndex += 2;
			val = 0;

			if (('\0' == c) || (']' == c)) {
				break;
			}

			/* check Zero compression case */
			if (':' == str[i + 1]) {
				if (':' == str[i + 2]) {
					/* ::: is invalid */
					return -1;
				}

				if (zero > 0) {
					/* only alow one zero compression */
					return -1;
				}

				zero = ipIndex;
				i++;
			}
		} else {
			if ((c >= '0') && (c <= '9')) {
				tmpVal = c - '0';
			} else if ((c >= 'a') && (c <= 'f')) {
				tmpVal = c - 'a' + 10;
			} else if ((c >= 'A') && (c <= 'F')) {
				tmpVal = c - 'A' + 10;
			} else {
				/* illegal char */
				return -1;
			}

			val = (val << 4) + (tmpVal & 0xf);
		}
	}

	if (ipIndex < CLI_IPV6_ADDR_LEN) {
		if (zero < 0) {
			/* too short address */
			return -1;
		}

		if (ipV6 != NULL) {
			memmove(&ipV6[zero + CLI_IPV6_ADDR_LEN - ipIndex],
				&ipV6[zero], ipIndex - zero);
			memset(&ipV6[zero], 0, CLI_IPV6_ADDR_LEN - ipIndex);
		}
	}

	return 0;
}

/******************************************************************************
 *
 * description:
 *   convert ipv6 address to str, like "2001::1"
 *
 * input:
 *   ipv6 - pointer to IPv6 address
 *
 * output:
 *   str  - string buf to contain result
 *
 * return:
 *   0  - okay
 *   -1 - fail
 *
 ******************************************************************************/
int cli_ipv6_2_str(const sa_u8_t * ipv6, char *str)
{
	int zStart = -1;	/* zero section start */
	int zEnd = -1;		/* zero section end */
	int fzStart = -1;	/* final zero section start */
	int fzEnd = -1;		/* final zero section end */
	sa_u16_t val;
	int len = 0;
	int i;

	/* parameter check */
	if ((NULL == ipv6) || (NULL == str)) {
		return -1;
	}

	/*
	 * Pre process 8 sections, find every continue zero
	 * section, from zStart to zEnd. Record longest continue
	 * zero sections to fzStart - zEnd.
	 */
	for (i = 0; i < 8; i++) {
		/* check zero section */
		if ((0 == ipv6[2 * i]) && (0 == ipv6[2 * i + 1])) {
			if (-1 == zStart) {
				zStart = zEnd = i;
			} else {
				zEnd = i;
			}
		} else {
			if (zStart != -1) {
				if ((-1 == fzStart)
				    || ((fzEnd - fzStart) < (zEnd - zStart))) {
					fzStart = zStart;
					fzEnd = zEnd;
				}

				zStart = -1;
				zEnd = -1;
			} else {
				/* do nothing */
			}
		}
	}

	if (zStart != -1) {
		if ((-1 == fzStart)
		    || ((fzEnd - fzStart) < (zEnd - zStart))) {
			fzStart = zStart;
			fzEnd = zEnd;
		}

		zStart = -1;
		zEnd = -1;
	}

	/*
	 * print sections
	 */
	str[0] = '\0';
	for (i = 0; i < 8; i++) {
		if ((-1 != fzStart) && (i == fzStart)) {
			if (0 == fzStart) {
				len += sprintf(str + len, "::");
			} else {
				len += sprintf(str + len, ":");
			}
		} else if ((-1 != fzStart) && (i > fzStart) && (i <= fzEnd)) {
			/* print nothing */
		} else {
			val = (ipv6[2 * i] << 8) + ipv6[2 * i + 1];

			if (i == 7) {
				len += sprintf(str + len, "%x", val);
			} else {
				len += sprintf(str + len, "%x:", val);
			}
		}
	}

	return 0;
}

int cli_ipv6_2_str_uncompress(const sa_u8_t * ipv6, char *str)
{
	sa_u16_t val;
	int len = 0;
	int i;

	/* parameter check */
	if ((NULL == ipv6) || (NULL == str)) {
		return -1;
	}

	str[0] = '\0';

	for (i = 0; i < 8; i++) {
		val = (ipv6[2 * i] << 8) + ipv6[2 * i + 1];

		if (i == 7) {
			len += sprintf(str + len, "%x", val);
		} else {
			len += sprintf(str + len, "%x:", val);
		}
	}

	return 0;
}

int cli_ipv6_2_str_contiki(const sa_u8_t * ipv6, char *str)
{
	sa_u16_t a;
	unsigned int i;
	int f;
	int len = 0;

	/* parameter check */
	if ((NULL == ipv6) || (NULL == str)) {
		return -1;
	}

	for (i = 0, f = 0; i < 16; i += 2) {
		a = (ipv6[i] << 8) + ipv6[i + 1];

		if (a == 0 && f >= 0) {
			if (f++ == 0) {
				len += sprintf(str + len, "::");
			}
		} else {
			if (f > 0) {
				f = -1;
			} else if (i > 0) {
				len += sprintf(str + len, ":");
			}

			len += sprintf(str + len, "%x", a);
		}
	}

	return 0;
}

/******************************************************************************
 *
 * description:
 *   convert str to hex, like "0x87ab"
 *
 * input:
 *   str - hex string
 *
 * output:
 *   pHex - pointer to unsigned 32 bits hex
 *
 * return:
 *   0  - okay
 *   -1 - invalid list string
 *
 ******************************************************************************/
int cli_str_2_hex(const char *str, sa_u32_t * pHex)
{
	char *endPtr;

	if ((NULL == str) || (NULL == pHex)) {
		return -1;
	}

	*pHex = strtoul(str, &endPtr, 16);
	if (*endPtr != '\0') {
		return -1;
	}

	return 0;
}

int cli_str_2_long_hex(const char *str, sa_u64_t * pU64)
{
	*pU64 = 0;

	return 0;
}

static int _cli_str_char_num(const char *str, char c)
{
	int strLen, i, count;

	if (NULL == str) {
		return 0;
	}

	strLen = strlen(str);
	count = 0;

	for (i = 0; i < strLen; i++) {
		if (str[i] == c) {
			count++;
		}
	}

	return count;
}

/******************************************************************************
 *
 * description:
 *   convert str to list, like "1,3,7-18,22"
 *
 * input:
 *   str - list string
 *   min - min value of each number
 *   max - max value of each number
 *
 * output:
 *   pPortArray - pointer to list array, if it is NULL then do not put result in it.
 *
 * return:
 *   0  - okay
 *   -1 - invalid list string
 *
 ******************************************************************************/
int cli_str_2_list(const char *str, const sa_u32_t min, const sa_u32_t max,
		   ARRAY_T * pPortArray)
{
#define CLI_LIST_CHARS "0123456789,-"
#define CLI_LIST_MAX_DECIMAL_LEN 10	/* max "4294967295" */
#define CLI_LIST_MAX_SECTION_LEN 21	/* max "4294967294-4294967295" */

	int strLen;
	sa_u32_t i;
	const char *sectionStart;
	const char *pComma;	/* pointer to ',' or '\0' */
	char *pDash;		/* pointer to '-' or '\0' */
	int dashNum;
	char decimalStr[CLI_LIST_MAX_DECIMAL_LEN + 1];
	int decimalStrLen;
	char sectionStr[CLI_LIST_MAX_SECTION_LEN + 1];
	int sectionStrLen;
	sa_u32_t val, start, end, lastVal = 0;
	sa_bool_t lastValValid = SA_FALSE;
	char *endPtr;
	sa_bool_t endFlag = SA_FALSE;

	/* parameter check */
	if (NULL == str) {
		return -1;
	}

	strLen = strlen(str);

	/* character check */
	if (strspn(str, CLI_LIST_CHARS) != strLen) {
		return -1;
	}

	/* the first character must be digital
	   if (!CLI_IS_DIGITAL(str[0])
	   {
	   return -1;
	   } */

	sectionStart = str;

	while (!endFlag) {
		pComma = strchr(sectionStart, ',');
		if (NULL == pComma) {
			endFlag = SA_TRUE;
			pComma = sectionStart + strlen(sectionStart);	/* pComma pointer to '\0' */
		}

		sectionStrLen = pComma - sectionStart;
		if (sectionStrLen > CLI_LIST_MAX_SECTION_LEN) {
			/* too long section string */
			return -1;
		}

		memcpy(sectionStr, sectionStart, sectionStrLen);
		sectionStr[sectionStrLen] = '\0';

		/* handle sectionStr, like "23" or "1-12" */
		dashNum = _cli_str_char_num(sectionStr, '-');
		if (0 == dashNum) {
			if (strlen(sectionStr) > CLI_LIST_MAX_DECIMAL_LEN) {
				/* too long decimal string */
				return -1;
			}

			/* convert sectionStr to value */
			val = strtoul(sectionStr, &endPtr, 10);
			if ((*endPtr != '\0')
			    || (min > val)
			    || (max < val)
			    || (lastValValid && (val <= lastVal))) {
				return -1;
			}

			if (pPortArray != NULL) {
				array_insert_slot(pPortArray, (void *)val);
			}

			lastVal = val;
			lastValValid = SA_TRUE;
		} else if (1 == dashNum) {
			/* check whether dash is first character or laster character */
			if (('-' == sectionStr[0])
			    || ('-' == sectionStr[sectionStrLen - 1])) {
				return -1;
			}

			pDash = strchr(sectionStr, '-');

			/*
			 * handle start number before '-'
			 */
			decimalStrLen = pDash - sectionStr;
			if (decimalStrLen > CLI_LIST_MAX_DECIMAL_LEN) {
				/* too long decimal string */
				return -1;
			}

			memcpy(decimalStr, sectionStr, decimalStrLen);
			decimalStr[decimalStrLen] = '\0';

			start = strtoul(decimalStr, &endPtr, 10);
			if ((*endPtr != '\0')
			    || (min > start)
			    || (max < start)
			    || (lastValValid && (start <= lastVal))) {
				return -1;
			}

			lastVal = start;
			lastValValid = SA_TRUE;

			/*
			 * handle end number after '-'
			 */
			decimalStrLen =
			    sectionStrLen - (pDash + 1 - sectionStr);
			if (decimalStrLen > CLI_LIST_MAX_DECIMAL_LEN) {
				/* too long decimal string */
				return -1;
			}

			memcpy(decimalStr, pDash + 1, decimalStrLen);
			decimalStr[decimalStrLen] = '\0';

			end = strtoul(decimalStr, &endPtr, 10);
			if ((*endPtr != '\0')
			    || (min > end)
			    || (max < end)
			    || (lastValValid && (end <= lastVal))) {
				return -1;
			}

			lastVal = end;
			lastValValid = SA_TRUE;

			/* insert every number in the range to array */
			for (i = start; i <= end; i++) {
				if (pPortArray != NULL) {
					array_insert_slot(pPortArray,
							  (void *)i);
				}
			}
		} else {	/* dashNum > 1 */

			return -1;
		}

		/* move to character after ',' */
		sectionStart = pComma + 1;
	}

	return 0;
}

/******************************************************************************
 *
 * description:
 *   convert list to str, like "1,3,7-18,22"
 *
 * input:
 *   list - pointer to 32bit array
 *   num  - number of list
 *
 * output:
 *   str  - string buf to contain result
 *
 * return:
 *   0  - okay
 *   -1 - fail
 *
 ******************************************************************************/
int cli_list_2_str(const sa_u32_t * list, const int num, char *str)
{
	int i;
	sa_u32_t start, end;

	if ((NULL == list) || (NULL == str)) {
		return -1;
	}

	str[0] = '\0';

	if (0 == num) {
		return 0;
	}

	for (i = 0; i < (num - 1); i++) {
		if (list[i] >= list[i + 1]) {
			return -1;
		}
	}

	start = end = list[0];

	for (i = 1; i < num; i++) {
		if ((list[i] - end) == 1) {
			end = list[i];
		} else {
			if (start == end) {
				sprintf(str + strlen(str), "%d,", start);
			} else {
				sprintf(str + strlen(str), "%d-%d,", start,
					end);
			}

			start = end = list[i];
		}
	}

	if (start == end) {
		sprintf(str + strlen(str), "%d,", start);
	} else {
		sprintf(str + strlen(str), "%d-%d,", start, end);
	}

	str[strlen(str) - 1] = '\0';

	return 0;
}

int cli_uint64_2_dec_str(sa_u64_t * num, char *str)
{
	unsigned long *value_p;
	value_p = (unsigned long *)num;
	if (*value_p == 0)
		sprintf(str, "%lu", *(value_p + 1));
	else {
		int width = 0, i, j;
		unsigned long long init_value = 1, value_array[30];
		char compute[30];
		unsigned long long value = *num;
		do {
			value_array[width] = init_value;
			width++;
			init_value *= 10;
		} while (value > init_value);
		for (i = 0; i < width; i++) {
			compute[i] = 0;
			for (j = 0; j < 10; j++) {
				if ((value_array[width - 1 - i] * j <= value)
				    && (value_array[width - 1 - i] * (j + 1) >
					value)) {
					value -= value_array[width - 1 - i] * j;
					compute[i] = j;
					break;
				}
			}
		}
		for (i = 0; i < width; i++) {
			*str = compute[i] + '0';
			str++;
		}
		*str = '\0';
	}
	return 0;
}

int cli_uint64_2_hex_str(sa_u64_t * num, char *str)
{
	unsigned long *value_p;
	value_p = (unsigned long *)num;
	if (*value_p == 0)
		sprintf(str, "0x%lx", *(value_p + 1));
	else
		sprintf(str, "0x%lx%lx", *value_p, *(value_p + 1));
	return 0;
}

/* sci=11223344:55667788 */
int cli_str_2_sci(const char *str, sa_u8_t * sci)
{
	int i;
	int str_len = strlen(str);
	sa_u32_t sci_addr[2];

	if (str_len != 17) {
		return -1;
	}

	if (strspn(str, "1234567890abcdefABCDEF:") != str_len) {
		return -1;
	}

	for (i = 0; i < str_len; i++) {
		if (i == 8) {
			if (str[i] != ':') {
				return -1;
			}
		} else {
			if (str[i] == ':') {
				return -1;
			}
		}
	}

	sscanf(str, "%8x:%8x", (sa_u32_t *) & sci_addr[0],
	       (sa_u32_t *) & sci_addr[1]);

	sci[0] = (sci_addr[0] >> 24) & 0xff;
	sci[1] = (sci_addr[0] >> 16) & 0xff;
	sci[2] = (sci_addr[0] >> 8) & 0xff;
	sci[3] = (sci_addr[0]) & 0xff;
	sci[4] = (sci_addr[1] >> 24) & 0xff;
	sci[5] = (sci_addr[1] >> 16) & 0xff;
	sci[6] = (sci_addr[1] >> 8) & 0xff;
	sci[7] = (sci_addr[1]) & 0xff;

	return 0;
}

int cli_sci_2_str(const sa_u8_t * sci, char *str)
{
	sa_u32_t sci_addr[2];

	sci_addr[0] =
	    (sci[0] << 24) | (sci[1] << 16) | (sci[2] << 8) | (sci[3]);
	sci_addr[1] =
	    (sci[4] << 24) | (sci[5] << 16) | (sci[6] << 8) | (sci[7]);

	sprintf(str, "%08x:%08x", sci_addr[0], sci_addr[1]);

	return 0;
}

/* sak=11223344:55667788:88776655:44332211 */
int cli_str_2_sak(const char *str, sa_u8_t * key)
{
	int i;
	int str_len = strlen(str);
	sa_u32_t key_addr[4];

	if (str_len != 35) {
		return -1;
	}

	if (strspn(str, "1234567890abcdefABCDEF:") != str_len) {
		return -1;
	}

	for (i = 0; i < str_len; i++) {
		if (i == 8 || i == 17 || i == 26) {
			if (str[i] != ':') {
				return -1;
			}
		} else {
			if (str[i] == ':') {
				return -1;
			}
		}
	}

	sscanf(str, "%8x:%8x:%8x:%8x", (sa_u32_t *) & key_addr[0],
	       (sa_u32_t *) & key_addr[1],
	       (sa_u32_t *) & key_addr[2], (sa_u32_t *) & key_addr[3]);

	for (i = 0; i < 4; i++) {
		((sa_u32_t *)key)[3-(i%4)] = key_addr[i];
	}

	return 0;
}

#if 0
int cli_sak_2_str(const sa_ch_t * key, char *str)
{
	int i;
	sa_u32_t key_addr[4];

	for (i = 0; i < 4; i++) {
		key_addr[i] =
		    ((sa_u32_t) key[i * 4] << 24) | ((sa_u32_t) key[i * 4 + 1]
						     << 16) | ((sa_u32_t) key[i
									      *
									      4
									      +
									      2]
							       << 8) |
		    ((sa_u32_t) key[i * 4 + 3]);
	}

	sprintf(str, "%8x:%8x:%8x:%8x", key_addr[0], key_addr[1], key_addr[2],
		key_addr[3]);

	return 0;
}
#else
int cli_sak_2_str(const sa_u8_t * key, char *str, int strLen)
{
	/*sak read from big address to little address, so we revert the key*/
	sa_u8_t i;
	sa_u8_t newkey[16];
	for (i = 0; i < 16; i++) {
		newkey[i] = key[15-i%16];
	}
	return cli_u8_array_2_str(newkey, 16, str, strLen);
}
#endif
int cli_str_2_flag(const char *str, sa_u32_t * value, flag_desc_t * desc,
		   int desc_len)
{
	int str_len;
	int session_len;

	const char *sessionStart;
	const char *strEnd;
	const char *cp;
	int i;

	*value = 0;		// init to zero

	cp = sessionStart = str;
	str_len = strlen(str);
	strEnd = str + str_len - 1;

	while (cp < strEnd) {
		cp = strchr(sessionStart, '/');
		if (NULL == cp) {
			cp = strEnd;	/* pointer to '\0' */
		}

		session_len = cp - sessionStart;

		for (i = 0; i < desc_len; i++) {
			if (strncmp(sessionStart, desc[i].str, session_len) ==
			    0) {
				*value |= desc[i].flag;
				break;
			}
		}

		if (i == desc_len) {
			/* str not in scope */
			return -1;
		}

		sessionStart = cp + 1;
	}

	return 0;
}

int cli_flag_2_str(char *str, sa_u32_t value, flag_desc_t * desc, int desc_len)
{
	int i;
	int str_len;

	for (i = 0; i < desc_len; i++) {
		if (value & desc[i].flag) {
			strcat(str, desc[i].str);
			strcat(str, "/");
		}
	}

	str_len = strlen(str);

	if (str[str_len - 1] == '/') {
		str[str_len - 1] = '\0';
	}

	return 0;
}

int cli_str_2_value(const char *str, sa_u32_t * value, flag_desc_t * desc,
		    int desc_len)
{
	int i;

	*value = 0;		// init to zero

	for (i = 0; i < desc_len; i++) {
		if (strncmp(str, desc[i].str, desc[i].cmp) == 0) {
			*value = desc[i].flag;
			return 0;
		}
	}

	return -1;
}

int cli_value_2_str(char *str, sa_u32_t value, flag_desc_t * desc, int desc_len)
{
	int i;

	for (i = 0; i < desc_len; i++) {
		if (value == desc[i].flag) {
			strcpy(str, desc[i].str);
			return 0;
		}
	}

	return -1;
}
