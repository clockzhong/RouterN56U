/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <net/if_arp.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <ralink_priv.h>
#include <iwlib.h>
#include <flash_mtd.h>

#include "rc.h"

#define MAX_FRW 64

int
get_wireless_mac(int is_5ghz)
{
	char macaddr[18];
	unsigned char buffer[ETHER_ADDR_LEN];
#if BOARD_5G_IN_SOC
	int i_offset = (is_5ghz) ? OFFSET_MAC_ADDR_WSOC : OFFSET_MAC_ADDR_INIC;
#elif BOARD_2G_IN_SOC
#if BOARD_HAS_5G_RADIO
	int i_offset = (is_5ghz) ? OFFSET_MAC_ADDR_INIC : OFFSET_MAC_ADDR_WSOC;
#else
	int i_offset = OFFSET_MAC_ADDR_WSOC;
#endif
#endif
	memset(buffer, 0, sizeof(buffer));
	memset(macaddr, 0, sizeof(macaddr));
	if (FRead(buffer, i_offset, ETHER_ADDR_LEN)<0) {
		puts("Unable to read MAC from EEPROM!");
		return -1;
	}

	ether_etoa(buffer, macaddr);
#if BOARD_HAS_5G_RADIO
	printf("%s EEPROM MAC address: %s\n", (is_5ghz) ? "5GHz" : "2.4GHz", macaddr);
#else
	printf("%s EEPROM MAC address: %s\n", "2.4GHz", macaddr);
#endif

	return 0;
}

int
set_wireless_mac(int is_5ghz, const char *mac)
{
	unsigned char ea[ETHER_ADDR_LEN];
#if BOARD_5G_IN_SOC
	int i_offset = (is_5ghz) ? OFFSET_MAC_ADDR_WSOC : OFFSET_MAC_ADDR_INIC;
#elif BOARD_2G_IN_SOC
#if BOARD_HAS_5G_RADIO
	int i_offset = (is_5ghz) ? OFFSET_MAC_ADDR_INIC : OFFSET_MAC_ADDR_WSOC;
#else
	int i_offset = OFFSET_MAC_ADDR_WSOC;
#endif
#endif
	if (ether_atoe(mac, ea)) {
		if (FWrite(ea, i_offset, ETHER_ADDR_LEN) == 0) {
			if (get_wireless_mac(is_5ghz) == 0)
				puts("\nPlease reboot router!");
		} else {
			puts("Write MAC to EEPROM FAILED!");
			return -1;
		}
	} else {
		printf("MAC [%s] is not valid MAC address!\n", mac);
		return EINVAL;
	}

	return 0;
}

int
get_wireless_cc(void)
{
	unsigned char CC[4];

	memset(CC, 0, sizeof(CC));
	if (FRead(CC, OFFSET_COUNTRY_CODE, 2) < 0) {
		puts("Unable to read Country Code from EEPROM!");
		return -1;
	}

	if (CC[0] == 0xff && CC[1] == 0xff)	// 0xffff is default
		printf("EEPROM CC: %s\n", "Undefined");
	else
		printf("EEPROM CC: %s\n", (char *)CC);

	return 0;
}

int
set_wireless_cc(const char *cc)
{
	unsigned char CC[4];

	/* Please refer to ISO3166 code list for other countries and can be found at
	 * http://www.iso.org/iso/en/prods-services/iso3166ma/02iso-3166-code-lists/list-en1.html#sz
	 */

	     if (!strcasecmp(cc, "DB")) ;
	else if (!strcasecmp(cc, "AL")) ;
	else if (!strcasecmp(cc, "DZ")) ;
	else if (!strcasecmp(cc, "AR")) ;
	else if (!strcasecmp(cc, "AM")) ;
	else if (!strcasecmp(cc, "AU")) ;
	else if (!strcasecmp(cc, "AT")) ;
	else if (!strcasecmp(cc, "AZ")) ;
	else if (!strcasecmp(cc, "BH")) ;
	else if (!strcasecmp(cc, "BY")) ;
	else if (!strcasecmp(cc, "BE")) ;
	else if (!strcasecmp(cc, "BZ")) ;
	else if (!strcasecmp(cc, "BO")) ;
	else if (!strcasecmp(cc, "BR")) ;
	else if (!strcasecmp(cc, "BN")) ;
	else if (!strcasecmp(cc, "BG")) ;
	else if (!strcasecmp(cc, "CA")) ;
	else if (!strcasecmp(cc, "CL")) ;
	else if (!strcasecmp(cc, "CN")) ;
	else if (!strcasecmp(cc, "CO")) ;
	else if (!strcasecmp(cc, "CR")) ;
	else if (!strcasecmp(cc, "HR")) ;
	else if (!strcasecmp(cc, "CY")) ;
	else if (!strcasecmp(cc, "CZ")) ;
	else if (!strcasecmp(cc, "DK")) ;
	else if (!strcasecmp(cc, "DO")) ;
	else if (!strcasecmp(cc, "EC")) ;
	else if (!strcasecmp(cc, "EG")) ;
	else if (!strcasecmp(cc, "SV")) ;
	else if (!strcasecmp(cc, "EE")) ;
	else if (!strcasecmp(cc, "FI")) ;
	else if (!strcasecmp(cc, "FR")) ;
	else if (!strcasecmp(cc, "GE")) ;
	else if (!strcasecmp(cc, "DE")) ;
	else if (!strcasecmp(cc, "GR")) ;
	else if (!strcasecmp(cc, "GT")) ;
	else if (!strcasecmp(cc, "HN")) ;
	else if (!strcasecmp(cc, "HK")) ;
	else if (!strcasecmp(cc, "HU")) ;
	else if (!strcasecmp(cc, "IS")) ;
	else if (!strcasecmp(cc, "IN")) ;
	else if (!strcasecmp(cc, "ID")) ;
	else if (!strcasecmp(cc, "IR")) ;
	else if (!strcasecmp(cc, "IE")) ;
	else if (!strcasecmp(cc, "IL")) ;
	else if (!strcasecmp(cc, "IT")) ;
	else if (!strcasecmp(cc, "JP")) ;
	else if (!strcasecmp(cc, "JO")) ;
	else if (!strcasecmp(cc, "KZ")) ;
	else if (!strcasecmp(cc, "KP")) ;
	else if (!strcasecmp(cc, "KR")) ;
	else if (!strcasecmp(cc, "KW")) ;
	else if (!strcasecmp(cc, "LV")) ;
	else if (!strcasecmp(cc, "LB")) ;
	else if (!strcasecmp(cc, "LI")) ;
	else if (!strcasecmp(cc, "LT")) ;
	else if (!strcasecmp(cc, "LU")) ;
	else if (!strcasecmp(cc, "MO")) ;
	else if (!strcasecmp(cc, "MK")) ;
	else if (!strcasecmp(cc, "MY")) ;
	else if (!strcasecmp(cc, "MX")) ;
	else if (!strcasecmp(cc, "MC")) ;
	else if (!strcasecmp(cc, "MA")) ;
	else if (!strcasecmp(cc, "NL")) ;
	else if (!strcasecmp(cc, "NZ")) ;
	else if (!strcasecmp(cc, "NO")) ;
	else if (!strcasecmp(cc, "OM")) ;
	else if (!strcasecmp(cc, "PK")) ;
	else if (!strcasecmp(cc, "PA")) ;
	else if (!strcasecmp(cc, "PE")) ;
	else if (!strcasecmp(cc, "PH")) ;
	else if (!strcasecmp(cc, "PL")) ;
	else if (!strcasecmp(cc, "PT")) ;
	else if (!strcasecmp(cc, "PR")) ;
	else if (!strcasecmp(cc, "QA")) ;
	else if (!strcasecmp(cc, "RO")) ;
	else if (!strcasecmp(cc, "RU")) ;
	else if (!strcasecmp(cc, "SA")) ;
	else if (!strcasecmp(cc, "SG")) ;
	else if (!strcasecmp(cc, "SK")) ;
	else if (!strcasecmp(cc, "SI")) ;
	else if (!strcasecmp(cc, "ZA")) ;
	else if (!strcasecmp(cc, "ES")) ;
	else if (!strcasecmp(cc, "SE")) ;
	else if (!strcasecmp(cc, "CH")) ;
	else if (!strcasecmp(cc, "SY")) ;
	else if (!strcasecmp(cc, "TW")) ;
	else if (!strcasecmp(cc, "TH")) ;
	else if (!strcasecmp(cc, "TT")) ;
	else if (!strcasecmp(cc, "TN")) ;
	else if (!strcasecmp(cc, "TR")) ;
	else if (!strcasecmp(cc, "UA")) ;
	else if (!strcasecmp(cc, "AE")) ;
	else if (!strcasecmp(cc, "GB")) ;
	else if (!strcasecmp(cc, "US")) ;
	else if (!strcasecmp(cc, "UY")) ;
	else if (!strcasecmp(cc, "UZ")) ;
	else if (!strcasecmp(cc, "VE")) ;
	else if (!strcasecmp(cc, "VN")) ;
	else if (!strcasecmp(cc, "YE")) ;
	else if (!strcasecmp(cc, "ZW")) ;
	else
	{
		puts("Invalid input Country Code!");
		return EINVAL;
	}

	memset(&CC[0], toupper(cc[0]), 1);
	memset(&CC[1], toupper(cc[1]), 1);

	if (FWrite(CC, OFFSET_COUNTRY_CODE, 2) == 0) {
		get_wireless_cc();
	} else {
		puts("Write Country Code to EEPROM FAILED!");
		return -1;
	}

	return 0;
}

int
atoh(const char *a, unsigned char *e)
{
	char *c = (char *) a;
	int i = 0;

	memset(e, 0, MAX_FRW);
	for (;;) {
		e[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == MAX_FRW)
			break;
	}
	return i;
}

char*
htoa(const unsigned char *e, char *a, int len)
{
	char *c = a;
	int i;

	for (i = 0; i < len; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", e[i] & 0xff);
	}
	return a;
}

int
pincheck(const char *a)
{
	unsigned char *c = (char *) a;
	int i = 0;

	for (;;) {
		if (*c>0x39 || *c<0x30)
			break;
		else
			i++;
		if (!*c++ || i == 8)
			break;
	}
	return (i == 8);
}

int
pinvalidate(const char *pin_string)
{
	unsigned long PIN = strtoul(pin_string, NULL, 10);
	unsigned long int accum = 0;
	unsigned int len = strlen(pin_string);

	if (len != 4 && len != 8)
		return  -1;

	if (len == 8) {
		accum += 3 * ((PIN / 10000000) % 10);
		accum += 1 * ((PIN / 1000000) % 10);
		accum += 3 * ((PIN / 100000) % 10);
		accum += 1 * ((PIN / 10000) % 10);
		accum += 3 * ((PIN / 1000) % 10);
		accum += 1 * ((PIN / 100) % 10);
		accum += 3 * ((PIN / 10) % 10);
		accum += 1 * ((PIN / 1) % 10);

		if (0 == (accum % 10))
			return 0;
	}
	else if (len == 4)
		return 0;

	return -1;
}

int
setPIN(const char *pin)
{
	if (pincheck(pin) && !pinvalidate(pin))
	{
		FWrite((char *)pin, OFFSET_PIN_CODE, 8);
		char PIN[9];
		memset(PIN, 0, 9);
		memcpy(PIN, pin, 8);
		puts(PIN);
	}
	return 0;
}

int
getBootVer(void)
{
	unsigned char btv[5];
	memset(btv, 0, sizeof(btv));
	FRead(btv, OFFSET_BOOT_VER, 4);
	puts(btv);

	return 0;
}

int
getPIN(void)
{
	unsigned char PIN[9];
	memset(PIN, 0, sizeof(PIN));
	FRead(PIN, OFFSET_PIN_CODE, 8);
	if (PIN[0]!=0xff)
		puts(PIN);
	return 0;
}

int
getCountryRegion(const char *str)
{
	int i_code;

	if (    (strcasecmp(str, "CA") == 0) || (strcasecmp(str, "CO") == 0) ||
		(strcasecmp(str, "DO") == 0) || (strcasecmp(str, "GT") == 0) ||
		(strcasecmp(str, "MX") == 0) || (strcasecmp(str, "NO") == 0) ||
		(strcasecmp(str, "PA") == 0) || (strcasecmp(str, "PR") == 0) ||
		(strcasecmp(str, "TW") == 0) || (strcasecmp(str, "US") == 0) ||
		(strcasecmp(str, "UZ") == 0))
		i_code = 0;   // channel 1-11
	else if (strcasecmp(str, "DB") == 0)
		i_code = 5;   // channel 1-14
	else
		i_code = 1;   // channel 1-13

	return i_code;
}

int
getCountryRegionABand(const char *str)
{
	int i_code;

	if ( (!strcasecmp(str, "AL")) ||
				(!strcasecmp(str, "DZ")) ||
				(!strcasecmp(str, "AU")) ||
				(!strcasecmp(str, "BH")) ||
				(!strcasecmp(str, "BY")) ||
				(!strcasecmp(str, "CA")) ||
				(!strcasecmp(str, "CL")) ||
				(!strcasecmp(str, "CO")) ||
				(!strcasecmp(str, "CR")) ||
				(!strcasecmp(str, "DO")) ||
				(!strcasecmp(str, "EC")) ||
				(!strcasecmp(str, "SV")) ||
				(!strcasecmp(str, "GT")) ||
				(!strcasecmp(str, "HN")) ||
				(!strcasecmp(str, "HK")) ||
				(!strcasecmp(str, "IN")) ||
				(!strcasecmp(str, "IL")) ||
				(!strcasecmp(str, "JO")) ||
				(!strcasecmp(str, "KZ")) ||
				(!strcasecmp(str, "KW")) ||
				(!strcasecmp(str, "LB")) ||
				(!strcasecmp(str, "MO")) ||
				(!strcasecmp(str, "MK")) ||
				(!strcasecmp(str, "MY")) ||
				(!strcasecmp(str, "MX")) ||
				(!strcasecmp(str, "MA")) ||
				(!strcasecmp(str, "NZ")) ||
				(!strcasecmp(str, "NO")) ||
				(!strcasecmp(str, "OM")) ||
				(!strcasecmp(str, "PK")) ||
				(!strcasecmp(str, "PA")) ||
				(!strcasecmp(str, "PR")) ||
				(!strcasecmp(str, "QA")) ||
				(!strcasecmp(str, "RO")) ||
				(!strcasecmp(str, "RU")) ||
				(!strcasecmp(str, "SA")) ||
				(!strcasecmp(str, "SG")) ||
				(!strcasecmp(str, "SY")) ||
				(!strcasecmp(str, "TH")) ||
				(!strcasecmp(str, "UA")) ||
				(!strcasecmp(str, "AE")) ||
				(!strcasecmp(str, "US")) ||
				(!strcasecmp(str, "VN")) ||
				(!strcasecmp(str, "YE")) ||
				(!strcasecmp(str, "ZW")) )
	{
		i_code = 0;
	}
	else if ( (!strcasecmp(str, "AT")) ||
				(!strcasecmp(str, "BE")) ||
				(!strcasecmp(str, "BR")) ||
				(!strcasecmp(str, "BG")) ||
				(!strcasecmp(str, "CY")) ||
				(!strcasecmp(str, "DK")) ||
				(!strcasecmp(str, "EE")) ||
				(!strcasecmp(str, "FI")) ||
				(!strcasecmp(str, "DE")) ||
				(!strcasecmp(str, "GR")) ||
				(!strcasecmp(str, "HU")) ||
				(!strcasecmp(str, "IS")) ||
				(!strcasecmp(str, "IE")) ||
				(!strcasecmp(str, "IT")) ||
				(!strcasecmp(str, "LV")) ||
				(!strcasecmp(str, "LI")) ||
				(!strcasecmp(str, "LT")) ||
				(!strcasecmp(str, "LU")) ||
				(!strcasecmp(str, "NL")) ||
				(!strcasecmp(str, "PL")) ||
				(!strcasecmp(str, "PT")) ||
				(!strcasecmp(str, "SK")) ||
				(!strcasecmp(str, "SI")) ||
				(!strcasecmp(str, "ZA")) ||
				(!strcasecmp(str, "ES")) ||
				(!strcasecmp(str, "SE")) ||
				(!strcasecmp(str, "CH")) ||
				(!strcasecmp(str, "GB")) ||
				(!strcasecmp(str, "UZ")) )
	{
		i_code = 1;
	}
	else if ( (!strcasecmp(str, "AM")) ||
				(!strcasecmp(str, "AZ")) ||
				(!strcasecmp(str, "HR")) ||
				(!strcasecmp(str, "CZ")) ||
				(!strcasecmp(str, "EG")) ||
				(!strcasecmp(str, "FR")) ||
				(!strcasecmp(str, "GE")) ||
				(!strcasecmp(str, "MC")) ||
				(!strcasecmp(str, "TT")) ||
				(!strcasecmp(str, "TN")) ||
				(!strcasecmp(str, "TR")) )
	{
		i_code = 2;
	}
	else if ( (!strcasecmp(str, "AR")) ||
			(!strcasecmp(str, "TW")) )
	{
		i_code = 3;
	}
	else if ( (!strcasecmp(str, "BZ")) ||
				(!strcasecmp(str, "BO")) ||
				(!strcasecmp(str, "BN")) ||
				(!strcasecmp(str, "CN")) ||
				(!strcasecmp(str, "ID")) ||
				(!strcasecmp(str, "IR")) ||
				(!strcasecmp(str, "PE")) ||
				(!strcasecmp(str, "PH")) )
	{
		i_code = 4;
	}
	else if (	(!strcasecmp(str, "KP")) ||
				(!strcasecmp(str, "KR")) ||
				(!strcasecmp(str, "UY")) ||
				(!strcasecmp(str, "VE")) )
	{
		i_code = 5;
	}
	else if (!strcasecmp(str, "DB"))
	{
		i_code = 7;
	}
	else if (!strcasecmp(str, "JP"))
	{
		i_code = 9;
	}
	else
	{
		i_code = 1;
	}

	return i_code;
}

static int
check_sku_file_exist(const char *prefix, const char *spec, char *out_buff, size_t out_size)
{
	snprintf(out_buff, out_size, "/etc_ro/Wireless/SingleSKU%s_%s.dat", prefix, spec);
	return check_if_file_exist(out_buff);
}

static int
gen_ralink_config(int is_soc_ap, int is_aband, int disable_autoscan)
{
	FILE *fp;
	char *p_str, *dat_file, *sku_file, *regspec, *prefix;
	char macbuf[36], list[2048], sku_link[64];
	int i, i_num,  i_val, i_wmm;
	int i_mode_x, i_gmode, i_auth, i_encr, i_wep, i_wds;
	int i_ssid_num, i_channel, i_channel_max, i_HTBW_MAX, i_VHTBW_MAX;
	int i_stream_tx, i_stream_rx, i_mphy, i_mmcs, i_fix, i_mcs;
	int i_val_mbss[2];
	char *c_val_mbss[2];

	i_ssid_num = 2; // AP+GuestAP
	i_channel_max = 13;

	if (!is_aband)
		prefix = "rt";
	else
		prefix = "wl";

	if (is_soc_ap) {
		dat_file = "/etc/Wireless/RT2860/RT2860AP.dat";
		sku_file = "/etc/Wireless/RT2860/SingleSKU.dat";
	} else {
		dat_file = "/etc/Wireless/iNIC/iNIC_ap.dat";
		sku_file = "/etc/Wireless/iNIC/SingleSKU.dat";
	}

	// 1T1R, 1T2R, 2T2R, 2T3R, 3T3R
	i_stream_tx = nvram_wlan_get_int(prefix, "stream_tx");
	i_stream_rx = nvram_wlan_get_int(prefix, "stream_rx");

	if (i_stream_tx < 1)
		i_stream_tx = 1;
	if (i_stream_rx < 1)
		i_stream_rx = 1;

	if (!is_aband) {
		if (i_stream_tx > BOARD_NUM_ANT_2G_TX)
			i_stream_tx = BOARD_NUM_ANT_2G_TX;
		
		if (i_stream_rx > BOARD_NUM_ANT_2G_RX)
			i_stream_rx = BOARD_NUM_ANT_2G_RX;
	} else {
		if (i_stream_tx > BOARD_NUM_ANT_5G_TX)
			i_stream_tx = BOARD_NUM_ANT_5G_TX;
		
		if (i_stream_rx > BOARD_NUM_ANT_5G_RX)
			i_stream_rx = BOARD_NUM_ANT_5G_RX;
	}

	if (i_stream_tx == 0 || i_stream_rx == 0)
		return 1; // this band is not supported

	i_mode_x = nvram_wlan_get_int(prefix, "mode_x");
	i_gmode = nvram_wlan_get_int(prefix, "gmode");

	regspec = nvram_safe_get("regspec_code");

	if (!(fp=fopen(dat_file, "w+")))
		return -1;

	fprintf(fp, "#The word of \"Default\" must not be removed\nDefault\n");

	//CountryRegion
	p_str = nvram_wlan_get("rt", "country_code");
	i_val = getCountryRegion(p_str);
	fprintf(fp, "CountryRegion=%d\n", i_val);

	if (!is_aband) {
		if (i_val == 0) // USA
			i_channel_max = 11;
		else if (i_val == 5) // Debug
			i_channel_max = 14;
		
		unlink(sku_file);
		
		snprintf(sku_link, sizeof(sku_link), "%s/SingleSKU%s.dat", "/etc/storage/wlan", "");
		if (!check_if_file_exist(sku_link)) {
			int sku_exist = 0;
			char *spec_fallback = "CE";
			
			if (strcasecmp(p_str, "US") == 0 ||
			    strcasecmp(p_str, "TW") == 0 ||
			    strcasecmp(p_str, "JP") == 0) { // Use FCC fallback rule yet
				spec_fallback = "FCC";
				if (strcasecmp(regspec, "NCC") == 0)
					sku_exist = check_sku_file_exist("", "NCC", sku_link, sizeof(sku_link));
			} else {
				if (strcasecmp(regspec, "AU") == 0)
					sku_exist = check_sku_file_exist("", "AU", sku_link, sizeof(sku_link));
				else if (strcasecmp(regspec, "SG") == 0)
					sku_exist = check_sku_file_exist("", "SG", sku_link, sizeof(sku_link));
			}
			
			if (!sku_exist)
				check_sku_file_exist("", spec_fallback, sku_link, sizeof(sku_link));
		}
		
		if (check_if_file_exist(sku_link))
			symlink(sku_link, sku_file);
	}

	//CountryRegion for A band
	p_str = nvram_wlan_get("wl", "country_code");
	i_val = getCountryRegionABand(p_str);
	fprintf(fp, "CountryRegionABand=%d\n", i_val);

	if (is_aband) {
		unlink(sku_file);
		
		snprintf(sku_link, sizeof(sku_link), "%s/SingleSKU%s.dat", "/etc/storage/wlan", "_5G");
		if (!check_if_file_exist(sku_link)) {
			int sku_exist = 0;
			char *spec_fallback = "CE";
			
			if (strcasecmp(p_str, "US") == 0 ||
			    strcasecmp(p_str, "TW") == 0 ||
			    strcasecmp(p_str, "JP") == 0 || // Use FCC fallback rule yet
			    strcasecmp(p_str, "DB") == 0) {
				spec_fallback = "FCC";
				if (strcasecmp(regspec, "NCC") == 0)
					sku_exist = check_sku_file_exist("_5G", "NCC", sku_link, sizeof(sku_link));
				else if (strcasecmp(regspec, "SG") == 0)
					sku_exist = check_sku_file_exist("_5G", "SG", sku_link, sizeof(sku_link));
				else if (strcasecmp(regspec, "AU") == 0)
					sku_exist = check_sku_file_exist("_5G", "AU", sku_link, sizeof(sku_link));
			}
			
			if (!sku_exist)
				check_sku_file_exist("_5G", spec_fallback, sku_link, sizeof(sku_link));
		}
		
		if (check_if_file_exist(sku_link))
			symlink(sku_link, sku_file);
	}

	//CountryCode
	p_str = nvram_wlan_get(prefix, "country_code");
	if (strlen(p_str) != 2)
		p_str = "GB";
	fprintf(fp, "CountryCode=%s\n", p_str);

	//ChannelGeography (Indoor+Outdoor)
	fprintf(fp, "ChannelGeography=%d\n", 2);

	//BssidNum
	fprintf(fp, "BssidNum=%d\n", i_ssid_num);

	//SSID
	fprintf(fp, "SSID%d=%s\n", 1, nvram_wlan_get(prefix, "ssid"));
	fprintf(fp, "SSID%d=%s\n", 2, nvram_wlan_get(prefix, "guest_ssid"));
	for (i = 3; i <= 8; i++)
		fprintf(fp, "SSID%d=%s\n", i, "");

	//Network Mode
	i_VHTBW_MAX = 0;
	if (!is_aband) {
		i_val = PHY_11GN_MIXED;
		switch (i_gmode)
		{
		case 0:  // B
			i_val = PHY_11B;
			break;
		case 1:  // B,G
			i_val = PHY_11BG_MIXED;
			break;
		case 2:  // B,G,N
			i_val = PHY_11BGN_MIXED;
			break;
		case 3:  // N
			i_val = PHY_11N;
			break;
		case 4:  // G
			i_val = PHY_11G;
			break;
		case 5:  // G,N
			i_val = PHY_11GN_MIXED;
			break;
		}
	} else {
#if BOARD_HAS_5G_11AC
		i_val = PHY_11VHT_N_A_MIXED;
#else
		i_val = PHY_11AN_MIXED;
#endif
		switch (i_gmode)
		{
		case 0:  // A
			i_val = PHY_11A;
			break;
		case 1:  // N
			i_val = PHY_11N_5G;
			break;
		case 2:  // A/N
			i_val = PHY_11AN_MIXED;
			break;
#if BOARD_HAS_5G_11AC
		case 3:  // N/AC
			i_val = PHY_11VHT_N_MIXED;
			break;
		case 4:  // A/N/AC
			i_val = PHY_11VHT_N_A_MIXED;
			break;
#endif
		}
	}
	if (i_val == PHY_11VHT_N_A_MIXED || i_val == PHY_11VHT_N_MIXED)
		i_VHTBW_MAX = 1;
	fprintf(fp, "WirelessMode=%d\n", i_val);

	//Channel
	i_channel = nvram_wlan_get_int(prefix, "channel");
	if (i_channel == 0 && disable_autoscan) {
		i_channel = (is_aband) ? 36 : 1;
	}
	fprintf(fp, "Channel=%d\n", i_channel);

	//AutoChannelSelect
	i_val = (i_channel == 0) ? 2 : 0;
	fprintf(fp, "AutoChannelSelect=%d\n", i_val);

	//BasicRate (not supported in 5G mode)
	if (!is_aband) {
		i_val = 15; // 1, 2, 5.5, 11 Mbps
		switch (i_gmode)
		{
		case 0:  // B
			i_val = 3; // 1, 2 Mbps
			break;
		case 4:  // G
		case 5:  // G,N
			i_val = 351; // 1, 2, 5.5, 11, 6, 12, 24 Mbps
			break;
		}
		
		fprintf(fp, "BasicRate=%d\n", i_val);
	}

	//BeaconPeriod [20..1000], default 100
	i_val = nvram_wlan_get_int(prefix, "bcn");
	if (i_val < 20 || i_val > 1000) i_val = 100;
	fprintf(fp, "BeaconPeriod=%d\n", i_val);

	//DTIM Period [1..255], default 1
	i_val = nvram_wlan_get_int(prefix, "dtim");
	if (i_val < 1 || i_val > 255) i_val = 1;
	fprintf(fp, "DtimPeriod=%d\n", i_val);

	//TxPower [0..100], default 100
	i_val = nvram_wlan_get_int(prefix, "TxPower");
	if (i_val < 0 || i_val > 100) i_val = 100;
	fprintf(fp, "TxPower=%d\n", i_val);

	//DisableOLBC
	fprintf(fp, "DisableOLBC=%d\n", 0);

	//BGProtection (Always OFF for 5GHz)
	i_val = 2; // off
	if (!is_aband && (i_gmode == 1 || i_gmode == 2)) {
		p_str = nvram_wlan_get(prefix, "gmode_protection");
		if (!strcmp(p_str, "auto"))
			i_val = 0;
		else if (!strcmp(p_str, "on"))
			i_val = 1;
	}
	fprintf(fp, "BGProtection=%d\n", i_val);

	//TxPreamble (0=Long, 1=Short)
	i_val = nvram_wlan_get_int(prefix, "preamble");
	if (i_val < 0 || i_val > 1) i_val = 0;
	fprintf(fp, "TxPreamble=%d\n", i_val);

	//RTSThreshold [1..2347], default 2347
	i_val = nvram_wlan_get_int(prefix, "rts");
	if (i_val < 1 || i_val > 2347) i_val = 2347;
	fprintf(fp, "RTSThreshold=%d\n", i_val);

	//FragThreshold [256..2346], default 2346
	i_val = nvram_wlan_get_int(prefix, "frag");
	if (i_val < 256 || i_val > 2346) i_val = 2346;
	fprintf(fp, "FragThreshold=%d\n", i_val);

	//TxBurst
	i_val = nvram_wlan_get_int(prefix, "TxBurst");
	if (i_val) i_val = 1;
	fprintf(fp, "TxBurst=%d\n", i_val);

	//PktAggregate
	i_val = nvram_wlan_get_int(prefix, "PktAggregate");
	if (i_val) i_val = 1;
	fprintf(fp, "PktAggregate=%d\n", i_val);

	//FreqDelta
	fprintf(fp, "FreqDelta=%d\n", 0);

	//WmmCapable (MBSSID used)
	i_wmm = nvram_wlan_get_int(prefix, "wme");
	fprintf(fp, "WmmCapable=%d;%d\n", i_wmm, i_wmm);

	fprintf(fp, "APAifsn=3;7;1;1\n");
	fprintf(fp, "APCwmin=4;4;3;2\n");
	fprintf(fp, "APCwmax=6;10;4;3\n");
	fprintf(fp, "APTxop=0;0;94;47\n");
	fprintf(fp, "APACM=0;0;0;0\n");
	fprintf(fp, "BSSAifsn=3;7;2;2\n");
	fprintf(fp, "BSSCwmin=4;4;3;2\n");
	fprintf(fp, "BSSCwmax=10;10;4;3\n");
	fprintf(fp, "BSSTxop=0;0;94;47\n");
	fprintf(fp, "BSSACM=0;0;0;0\n");

	//AckPolicy
	p_str = nvram_wlan_get(prefix, "wme_no_ack");
	i_val = (strcmp(p_str, "on")) ? 0 : 1;
	if (!i_wmm)
		i_val = 0;
	if (!is_aband) {
		if (i_gmode != 0 && i_gmode != 1 && i_gmode != 4) // != (B, B/G, G)
			i_val = 0;
	} else {
		if (i_gmode != 0) // != A only
			i_val = 0;
	}
	list[0] = 0;
	for (i = 0; i < 4; i++)
		sprintf(list+strlen(list), "%d;", i_val);
	list[strlen(list) - 1] = '\0';
	fprintf(fp, "AckPolicy=%s\n", list);

	//APSDCapable
	i_val = nvram_wlan_get_int(prefix, "APSDCapable");
	if (i_val) i_val = 1;
	if (!i_wmm) i_val = 0;
	fprintf(fp, "APSDCapable=%d\n", i_val);

	//DLSCapable (MBSSID used)
	fprintf(fp, "DLSCapable=%d;%d\n", 0, 0);

	//NoForwarding (MBSSID used)
	i_val_mbss[0] = nvram_wlan_get_int(prefix, "ap_isolate");
	i_val_mbss[1] = nvram_wlan_get_int(prefix, "guest_ap_isolate");
	fprintf(fp, "NoForwarding=%d;%d\n", i_val_mbss[0], i_val_mbss[1]);
	
	//NoForwardingBTNBSSID
	i_val = nvram_wlan_get_int(prefix, "mbssid_isolate");
	if (i_val) i_val = 1;
	fprintf(fp, "NoForwardingBTNBSSID=%d\n", i_val);

	//HideSSID (MBSSID used)
	i_val_mbss[0] = nvram_wlan_get_int(prefix, "closed");
	i_val_mbss[1] = nvram_wlan_get_int(prefix, "guest_closed");
	fprintf(fp, "HideSSID=%d;%d\n", i_val_mbss[0], i_val_mbss[1]);

	//ShortSlot
	fprintf(fp, "ShortSlot=%d\n", 1);

	fprintf(fp, "IEEE80211H=%d\n", 0);
	fprintf(fp, "CarrierDetect=%d\n", 0);
	fprintf(fp, "PreAntSwitch=\n");
	fprintf(fp, "PhyRateLimit=%d\n", 0);
	fprintf(fp, "DebugFlags=%d\n", 0);
	fprintf(fp, "FineAGC=%d\n", 0);
	fprintf(fp, "StreamMode=%d\n", (is_aband) ? 3 : 0);
	fprintf(fp, "StreamModeMac0=\n");
	fprintf(fp, "StreamModeMac1=\n");
	fprintf(fp, "StreamModeMac2=\n");
	fprintf(fp, "StreamModeMac3=\n");
	fprintf(fp, "CSPeriod=%d\n", 10);
	fprintf(fp, "RDRegion=%s\n", "FCC"); // used for Radar Detection
	fprintf(fp, "StationKeepAlive=%d;%d\n", 0, 0);
	fprintf(fp, "DfsLowerLimit=%d\n", 0);
	fprintf(fp, "DfsUpperLimit=%d\n", 0);
	fprintf(fp, "DfsIndoor=%d\n", 0);
	fprintf(fp, "DFSParamFromConfig=%d\n", 0);
	fprintf(fp, "FCCParamCh0=\n");
	fprintf(fp, "FCCParamCh1=\n");
	fprintf(fp, "FCCParamCh2=\n");
	fprintf(fp, "FCCParamCh3=\n");
	fprintf(fp, "CEParamCh0=\n");
	fprintf(fp, "CEParamCh1=\n");
	fprintf(fp, "CEParamCh2=\n");
	fprintf(fp, "CEParamCh3=\n");
	fprintf(fp, "JAPParamCh0=\n");
	fprintf(fp, "JAPParamCh1=\n");
	fprintf(fp, "JAPParamCh2=\n");
	fprintf(fp, "JAPParamCh3=\n");
	fprintf(fp, "JAPW53ParamCh0=\n");
	fprintf(fp, "JAPW53ParamCh1=\n");
	fprintf(fp, "JAPW53ParamCh2=\n");
	fprintf(fp, "JAPW53ParamCh3=\n");
	fprintf(fp, "FixDfsLimit=%d\n", 0);
	fprintf(fp, "LongPulseRadarTh=%d\n", 0);
	fprintf(fp, "AvgRssiReq=%d\n", 0);
	fprintf(fp, "DFS_R66=%d\n", 0);
	fprintf(fp, "BlockCh=\n");

	//GreenAP
	i_val = nvram_wlan_get_int(prefix, "greenap");
	if (i_val) i_val = 1;
	fprintf(fp, "GreenAP=%d\n", i_val);

	//AuthMode (MBSSID used)
	i_auth = 0; // Open
	c_val_mbss[0] = "OPEN";
	c_val_mbss[1] = "OPEN";
	i_val = nvram_wlan_get_int(prefix, "wpa_mode");
	p_str = nvram_wlan_get(prefix, "auth_mode");
	if (!strcmp(p_str, "shared"))
	{
		i_auth = 1; // Shared
		c_val_mbss[0] = "SHARED";
	}
	else if (!strcmp(p_str, "psk"))
	{
		if (i_val == 1) {
			i_auth = 2; // WPA PSK
			c_val_mbss[0] = "WPAPSK";
		} else if (i_val == 2) {
			i_auth = 3; // WPA2 PSK
			c_val_mbss[0] = "WPA2PSK";
		} else {
			i_auth = 4; // WPA PSK or WPA2 PSK
			c_val_mbss[0] = "WPAPSKWPA2PSK";
		}
	}
	else if (!strcmp(p_str, "wpa"))
	{
		if (i_val == 3) {
			i_auth = 5; // WPA ENT
			c_val_mbss[0] = "WPA";
		} else {
			i_auth = 7; // WPA ENT or WPA2 ENT
			c_val_mbss[0] = "WPA1WPA2";
		}
	}
	else if (!strcmp(p_str, "wpa2"))
	{
		i_auth = 6; // WPA2 ENT
		c_val_mbss[0] = "WPA2";
	}
	else if (!strcmp(p_str, "radius"))
	{
		i_auth = 8; // 8021X EAP with Radius
	}
	
	i_val = nvram_wlan_get_int(prefix, "guest_wpa_mode");
	p_str = nvram_wlan_get(prefix, "guest_auth_mode");
	if (!strcmp(p_str, "psk"))
	{
		if (i_val == 1)
			c_val_mbss[1] = "WPAPSK";
		else if (i_val == 2)
			c_val_mbss[1] = "WPA2PSK";
		else
			c_val_mbss[1] = "WPAPSKWPA2PSK";
	}
	fprintf(fp, "AuthMode=%s;%s\n", c_val_mbss[0], c_val_mbss[1]);

	//EncrypType (MBSSID used)
	i_encr = 0;  // None
	c_val_mbss[0] = "NONE";
	c_val_mbss[1] = "NONE";
	i_wep = nvram_wlan_get_int(prefix, "wep_x");
	if ((i_auth == 0 && i_wep != 0) || i_auth == 1 || i_auth == 8) {
		i_encr = 1;  // WEP
		c_val_mbss[0] = "WEP";
	} else if (i_auth != 0) {
		p_str = nvram_wlan_get(prefix, "crypto");
		if (!strcmp(p_str, "tkip")) {
			i_encr = 2;  // TKIP
			c_val_mbss[0] = "TKIP";
		} else if (!strcmp(p_str, "aes")) {
			i_encr = 3;  // AES
			c_val_mbss[0] = "AES";
		} else if (!strcmp(p_str, "tkip+aes")) {
			i_encr = 4;  // TKIP or AES
			c_val_mbss[0] = "TKIPAES";
		}
	}
	p_str = nvram_wlan_get(prefix, "guest_auth_mode");
	if (!strcmp(p_str, "psk")) {
		p_str = nvram_wlan_get(prefix, "guest_crypto");
		if (!strcmp(p_str, "tkip"))
			c_val_mbss[1] = "TKIP";
		else if (!strcmp(p_str, "aes"))
			c_val_mbss[1] = "AES";
		else if (!strcmp(p_str, "tkip+aes"))
			c_val_mbss[1] = "TKIPAES";
	}
	fprintf(fp, "EncrypType=%s;%s\n", c_val_mbss[0], c_val_mbss[1]);

	//Wapi
	for (i = 1; i <= 8; i++)
		fprintf(fp, "WapiPsk%d=\n", i);
	fprintf(fp, "WapiPskType=\n");
	fprintf(fp, "Wapiifname=\n");
	fprintf(fp, "WapiAsCertPath=\n");
	fprintf(fp, "WapiUserCertPath=\n");
	fprintf(fp, "WapiAsIpAddr=\n");
	fprintf(fp, "WapiAsPort=\n");

	// Mesh
	fprintf(fp, "MeshAutoLink=%d\n", 0);
	fprintf(fp, "MeshAuthMode=\n");
	fprintf(fp, "MeshEncrypType=\n");
	fprintf(fp, "MeshDefaultkey=%d\n", 0);
	fprintf(fp, "MeshWEPKEY=\n");
	fprintf(fp, "MeshWPAKEY=\n");
	fprintf(fp, "MeshId=\n");

	//RekeyInterval (MBSSID used, auto copy to all BSSID)
	p_str = "TIME";
	i_val = nvram_wlan_get_int(prefix, "wpa_gtk_rekey");
	if (i_val == 0)
		p_str = "DISABLE";
	fprintf(fp, "RekeyMethod=%s\n", p_str);
	fprintf(fp, "RekeyInterval=%d\n", i_val);

	//PMKCachePeriod
	fprintf(fp, "PMKCachePeriod=%d\n", 10);

	//WPAPSK
	fprintf(fp, "WPAPSK%d=%s\n", 1, nvram_wlan_get(prefix, "wpa_psk"));
	fprintf(fp, "WPAPSK%d=%s\n", 2, nvram_wlan_get(prefix, "guest_wpa_psk"));
	for (i = 3; i <= 8; i++)
		fprintf(fp, "WPAPSK%d=%s\n", i, "");

	//DefaultKeyID
	i_val = nvram_wlan_get_int(prefix, "key");
	if (i_val < 1 || i_val > 4) i_val = 1;
	fprintf(fp, "DefaultKeyID=%d\n", i_val);

	sprintf(list, "%s_key%d", prefix, i_val);
	if ((strlen(nvram_safe_get(list)) == 5) || (strlen(nvram_safe_get(list)) == 13))
	{
		nvram_wlan_set(prefix, "key_type", "1");
	}
	else if ((strlen(nvram_safe_get(list)) == 10) || (strlen(nvram_safe_get(list)) == 26))
	{
		nvram_wlan_set(prefix, "key_type", "0");
	}

	//Key1Type(0 -> Hex, 1->Ascii)
	fprintf(fp, "Key%dType=%s\n", 1, nvram_wlan_get(prefix, "key_type"));
	//Key1Str
	fprintf(fp, "Key%dStr%d=%s\n", 1, 1, nvram_wlan_get(prefix, "key1"));
	for (i = 2; i <= 8; i++)
		fprintf(fp, "Key%dStr%d=%s\n", 1, i, "");

	//Key2Type
	fprintf(fp, "Key%dType=%s\n", 2, nvram_wlan_get(prefix, "key_type"));
	//Key2Str
	fprintf(fp, "Key%dStr%d=%s\n", 2, 1, nvram_wlan_get(prefix, "key2"));
	for (i = 2; i <= 8; i++)
		fprintf(fp, "Key%dStr%d=%s\n", 2, i, "");

	//Key3Type
	fprintf(fp, "Key%dType=%s\n", 3, nvram_wlan_get(prefix, "key_type"));
	//Key3Str
	fprintf(fp, "Key%dStr%d=%s\n", 3, 1, nvram_wlan_get(prefix, "key3"));
	for (i = 2; i <= 8; i++)
		fprintf(fp, "Key%dStr%d=%s\n", 3, i, "");

	//Key4Type
	fprintf(fp, "Key%dType=%s\n", 4, nvram_wlan_get(prefix, "key_type"));
	//Key4Str
	fprintf(fp, "Key%dStr%d=%s\n", 4, 1, nvram_wlan_get(prefix, "key4"));
	for (i = 2; i <= 8; i++)
		fprintf(fp, "Key%dStr%d=%s\n", 4, i, "");

	fprintf(fp, "HSCounter=%d\n", 0);

	//HT_RDG
	i_val = nvram_wlan_get_int(prefix, "HT_RDG");
	if (i_val) i_val = 1;
	fprintf(fp, "HT_HTC=%d\n", i_val);
	fprintf(fp, "HT_RDG=%d\n", i_val);

	//HT_LinkAdapt
	fprintf(fp, "HT_LinkAdapt=%d\n", 0);

	//HT_OpMode
	i_val = nvram_wlan_get_int(prefix, "HT_OpMode");
	if (!is_aband) {
		if (i_gmode != 3)
			i_val = 0; // GreenField only for N only
	} else {
		if (i_gmode != 1 && i_gmode != 3)
			i_val = 0; // GreenField only for N, N/AC only
	}
	fprintf(fp, "HT_OpMode=%d\n", i_val);

	//HT_MpduDensity
	i_val = nvram_wlan_get_int(prefix, "HT_MpduDensity");
	if (i_val < 0 || i_val > 7) i_val = 5;
	fprintf(fp, "HT_MpduDensity=%d\n", i_val);

	// HT_EXTCHA
	i_HTBW_MAX = 1;
	if (!is_aband) {
		int i_EXTCHA_MAX = 0;
		
		if ((i_channel >= 0) && (i_channel <= 7))
			i_EXTCHA_MAX = 1;
		else if ((i_channel >= 8) && (i_channel <= 13))
			i_EXTCHA_MAX = ((i_channel_max - i_channel) < 4) ? 0 : 1;
		else
			i_HTBW_MAX = 0; // Ch14 force BW=20
		
		i_val = nvram_get_int("rt_HT_EXTCHA");
		if (i_val) i_val = 1;
		if (i_channel >= 1 && i_channel <= 4)
			i_val = 1;
		else if (i_val > i_EXTCHA_MAX)
			i_val = 0;
		
		fprintf(fp, "HT_EXTCHA=%d\n", i_val);
	} else {
		int i_EXTCHA = 1;
		
		if (i_channel != 0)
		{
			switch (i_channel)
			{
			case 36:
			case 44:
			case 52:
			case 60:
			case 100:
			case 108:
			case 116:
			case 124:
			case 132:
			case 149:
			case 157:
				i_EXTCHA = 1;
				break;
			case 40:
			case 48:
			case 56:
			case 64:
			case 104:
			case 112:
			case 120:
			case 128:
			case 136:
			case 153:
			case 161:
				i_EXTCHA = 0;
				break;
			default:
				i_HTBW_MAX = 0;
				break;
			}
		}
		fprintf(fp, "HT_EXTCHA=%d\n", i_EXTCHA);
	}

	//HT_BW
	i_val = nvram_wlan_get_int(prefix, "HT_BW");
	if (i_val > 1) i_val = 1;
	if (i_HTBW_MAX == 0) i_val = 0;
	fprintf(fp, "HT_BW=%d\n", i_val);

	//HT_BSSCoexistence
	fprintf(fp, "HT_BSSCoexistence=%d\n", 0);

	//HT_BSSCoexAPCntThr
	fprintf(fp, "HT_BSSCoexAPCntThr=%d\n", 10);

	//HT_AutoBA
	i_val = nvram_wlan_get_int(prefix, "HT_AutoBA");
	if (i_val) i_val = 1;
	fprintf(fp, "HT_AutoBA=%d\n", i_val);

	//HT_BADecline
	fprintf(fp, "HT_BADecline=%d\n", 0);

	//HT_AMSDU
	i_val = nvram_wlan_get_int(prefix, "HT_AMSDU");
	fprintf(fp, "HT_AMSDU=%d\n", i_val);

	//HT_BAWinSize
	i_val = nvram_wlan_get_int(prefix, "HT_BAWinSize");
	if (i_val < 1 || i_val > 64) i_val = 64;
	fprintf(fp, "HT_BAWinSize=%d\n", i_val);

	//HT_GI
	fprintf(fp, "HT_GI=%d\n", 1);

	//HT_STBC
	fprintf(fp, "HT_STBC=%d\n", 1);

	i_fix = 0;  // FixedTxMode=OFF
	i_mcs = 33; // HT_MCS=Auto
	i_val = nvram_wlan_get_int(prefix, "guest_mcs_mode");
	switch (i_val)
	{
	case 1: // HTMIX (1S) 19.5-45 Mbps
		i_mcs = 2;
		break;
	case 2: // HTMIX (1S) 15-30 Mbps
		i_mcs = 1;
		break;
	case 3: // HTMIX (1S) 6.5-15 Mbps
		i_mcs = 0;
		break;
	case 4: // OFDM 12 Mbps
		i_fix = 2;
		i_mcs = 2;
		break;
	case 5: // OFDM 9 Mbps
		i_fix = 2;
		i_mcs = 1;
		break;
	case 6: // OFDM 6 Mbps
		i_fix = 2;
		i_mcs = 0;
		break;
	case 7: // CCK 5.5 Mbps
		if (!is_aband) {
			i_fix = 1;
			i_mcs = 2;
		}
		break;
	case 8: // CCK 2 Mbps
		if (!is_aband) {
			i_fix = 1;
			i_mcs = 1;
		}
		break;
	case 9: // CCK 1 Mbps
		if (!is_aband) {
			i_fix = 1;
			i_mcs = 0;
		}
		break;
	}

	//FixedTxMode (MBSSID used)
	fprintf(fp, "FixedTxMode=%d;%d\n", 0, i_fix);

	//HT_MCS (MBSSID used), force AUTO for Main
	fprintf(fp, "HT_MCS=%d;%d\n", 33, i_mcs);

	//HT_TxStream
	fprintf(fp, "HT_TxStream=%d\n", i_stream_tx);

	//HT_RxStream
	fprintf(fp, "HT_RxStream=%d\n", i_stream_rx);

	//HT_PROTECT
	fprintf(fp, "HT_PROTECT=%d\n", 1);

	//HT_DisallowTKIP
	fprintf(fp, "HT_DisallowTKIP=%d\n", 0);

	//HT_LDPC
	fprintf(fp, "HT_LDPC=%d\n", 1);

#if BOARD_HAS_5G_11AC
	if (is_aband) {
		//VHT_BW
		i_val = nvram_wlan_get_int(prefix, "HT_BW");
		i_val = (i_val > 1) ? 1 : 0;
		if (i_HTBW_MAX == 0 || i_VHTBW_MAX == 0) i_val = 0;
		fprintf(fp, "VHT_BW=%d\n", i_val);
		
		//VHT_SGI
		fprintf(fp, "VHT_SGI=%d\n", 1);
		
		//VHT_BW_SIGNAL
		fprintf(fp, "VHT_BW_SIGNAL=%d\n", 0);
		
		//VHT_DisallowNonVHT
		i_val = nvram_wlan_get_int(prefix, "VHT_Only");
		if (i_val) i_val = 1;
		if (i_gmode != 3 && i_gmode != 4) i_val = 0;
		fprintf(fp, "VHT_DisallowNonVHT=%d\n", i_val);
		
		//VHT_LDPC
		fprintf(fp, "VHT_LDPC=%d\n", 1);
	}
#endif

	//Wsc
	fprintf(fp, "WscConfMode=%d\n", 0);
	fprintf(fp, "WscConfStatus=%d\n", 2);
	fprintf(fp, "WscVendorPinCode=%s\n", nvram_safe_get("secret_code"));
	fprintf(fp, "WscManufacturer=%s\n", "ASUSTeK Computer Inc.");
	fprintf(fp, "WscModelName=%s\n", "WPS Router");
	fprintf(fp, "WscDeviceName=%s\n", "ASUS WPS Router");
	fprintf(fp, "WscModelNumber=%s\n", BOARD_NAME);
	fprintf(fp, "WscSerialNumber=%s\n", "00000000");
	fprintf(fp, "WscV2Support=%d\n", 1);

	// ITxBfEn
	if (is_aband) {
		i_val = nvram_wlan_get_int(prefix, "txbf");
		if (i_val > 0 && nvram_wlan_get_int(prefix, "txbf_en") == 1)
			i_val = 1;
		else
			i_val = 0;
		fprintf(fp, "ITxBfEn=%d\n", i_val);
		fprintf(fp, "ETxBfEnCond=%d\n", i_val);
	}

	//AccessPolicy0
	i_val = 0;
	p_str = nvram_wlan_get(prefix, "macmode");
	if (!strcmp(p_str, "allow"))
		i_val = 1;
	else if (!strcmp(p_str, "deny"))
		i_val = 2;
	fprintf(fp, "AccessPolicy%d=%d\n", 0, i_val);

	list[0] = 0;
	if (i_val != 0)
	{
		char wlan_param[32];
		
		sprintf(wlan_param, "%s_%s", prefix, "maclist_x");
		i_num = nvram_wlan_get_int(prefix, "macnum_x");
		for (i = 0; i < i_num; i++)
			sprintf(list+strlen(list), "%s;", mac_conv(wlan_param, i, macbuf));
		if (i_num > 0)
			list[strlen(list) - 1] = '\0';
	}

	//AccessControlList0
	fprintf(fp, "AccessControlList%d=%s\n", 0, list);

	//AccessPolicy1
	//AccessControlList1
	if (nvram_wlan_get_int(prefix, "guest_macrule") == 1)
	{
		fprintf(fp, "AccessPolicy%d=%d\n", 1, i_val);
		fprintf(fp, "AccessControlList%d=%s\n", 1, list);
	}
	else
	{
		fprintf(fp, "AccessPolicy%d=%d\n", 1, 0);
		fprintf(fp, "AccessControlList%d=%s\n", 1, "");
	}

	for (i = 2; i <= 8; i++) {
		fprintf(fp, "AccessPolicy%d=%d\n", i, 0);
		fprintf(fp, "AccessControlList%d=%s\n", i, "");
	}

	//WdsEnable
	i_wds = 0;
	if (i_mode_x == 1 || i_mode_x == 2) {
		// WDS support only OPEN+NONE, OPEN+WEP and WPA2PSK+AES
		if ((i_auth == 0) || (i_auth == 3 && i_encr == 3)) {
			if (i_mode_x == 2) {
				if (nvram_wlan_get_int(prefix, "wdsapply_x") == 0)
					i_wds = 4;
				else
					i_wds = 3;
			} else {
				i_wds = 2;
			}
		}
	}
	fprintf(fp, "WdsEnable=%d\n", i_wds);

	//WdsPhyMode
	p_str = "HTMIX";
	if (!is_aband) {
		if (i_gmode == 0) // B
			p_str = "CCK";
		else if (i_gmode == 1 || i_gmode == 4) // B,G or G
			p_str = "OFDM";
		else if (i_gmode == 3) // N
			p_str = "GREENFIELD";
	} else {
		if (i_gmode == 0) // A
			p_str = "OFDM";
		else if (i_gmode == 1) // N
			p_str = "GREENFIELD";
	}
	fprintf(fp, "WdsPhyMode=%s\n", p_str);

	//WdsEncrypType
	p_str = "NONE;NONE;NONE;NONE";
	if (i_auth == 0 && i_wep != 0)
		p_str = "WEP;WEP;WEP;WEP";
	else if (i_auth == 3 && i_encr == 3)
		p_str = "AES;AES;AES;AES";
	fprintf(fp, "WdsEncrypType=%s\n", p_str);

	//WdsList
	list[0] = 0;
	if (i_wds == 2 || i_wds == 3) {
		char wlan_param[32];
		
		sprintf(wlan_param, "%s_%s", prefix, "wdslist_x");
		i_num = nvram_wlan_get_int(prefix, "wdsnum_x");
		for (i = 0; i < i_num; i++)
			sprintf(list+strlen(list), "%s;", mac_conv(wlan_param, i, macbuf));
		if (i_num > 0)
			list[strlen(list) - 1] = '\0';
	}
	fprintf(fp, "WdsList=%s\n", list);

	//WdsKey
	p_str = "";
	if (i_auth == 0 && i_wep != 0) {
		i_val = nvram_wlan_get_int(prefix, "key");
		if (i_val < 1 || i_val > 4) i_val = 1;
		fprintf(fp, "WdsDefaultKeyID=%d;%d;%d;%d\n", i_val, i_val, i_val, i_val);
		sprintf(list, "%s_key%d", prefix, i_val);
		p_str = nvram_safe_get(list);
	} else if (i_auth == 3 && i_encr == 3) {
		fprintf(fp, "WdsDefaultKeyID=\n");
		p_str = nvram_wlan_get(prefix, "wpa_psk");
	} else {
		fprintf(fp, "WdsDefaultKeyID=\n");
	}

	for (i = 0; i < 4; i++)
		fprintf(fp, "Wds%dKey=%s\n", i, p_str);

	// RADIUS
	fprintf(fp, "session_timeout_interval=%d\n", 0);

	if (i_auth == 5 || i_auth == 6 || i_auth == 7 || i_auth == 8)
	{
		fprintf(fp, "own_ip_addr=%s\n", nvram_safe_get("lan_ipaddr_t"));
		fprintf(fp, "EAPifname=%s\n", IFNAME_BR);
		fprintf(fp, "PreAuthifname=%s\n", IFNAME_BR);
	}
	else
	{
		fprintf(fp, "own_ip_addr=%s\n", "");
		fprintf(fp, "EAPifname=%s\n", "");
		fprintf(fp, "PreAuthifname=%s\n", "");
	}

	//PreAuth (MBSSID used)
	fprintf(fp, "PreAuth=0;0\n");

	//IEEE8021X (MBSSID used)
	i_val = 0;
	if (i_auth == 8)
		i_val = 1;
	fprintf(fp, "IEEE8021X=%d;%d\n", i_val, 0);

	//RADIUS_Server (MBSSID used)
	p_str = nvram_wlan_get(prefix, "radius_ipaddr");
	fprintf(fp, "RADIUS_Server=%s;%s\n", p_str, p_str);

	//RADIUS_Port (MBSSID used)
	i_val = nvram_wlan_get_int(prefix, "radius_port");
	fprintf(fp, "RADIUS_Port=%d;%d\n", i_val, i_val);

	//RADIUS_Key
	p_str = nvram_wlan_get(prefix, "radius_key");
	fprintf(fp, "RADIUS_Key%d=%s\n", 1, p_str);
	fprintf(fp, "RADIUS_Key%d=%s\n", 2, p_str);
	for (i = 3; i <= 8; i++)
		fprintf(fp, "RADIUS_Key%d=%s\n", i, "");

	//WiFiTest
	fprintf(fp, "WiFiTest=%d\n", 0);

	//TGnWifiTest
	fprintf(fp, "TGnWifiTest=%d\n", 0);

	//ApCliEnable
	i_val = 0;
	p_str = nvram_wlan_get(prefix, "sta_ssid");
	if ((i_mode_x == 3 || i_mode_x == 4) && strlen(p_str) > 0)
		i_val = 1;
	fprintf(fp, "ApCliEnable=%d\n", i_val);
	fprintf(fp, "ApCliSsid=%s\n", p_str);
	fprintf(fp, "ApCliBssid=\n");

	p_str = nvram_wlan_get(prefix, "sta_auth_mode");
	if (!strcmp(p_str, "psk"))
	{
		if (nvram_wlan_get_int(prefix, "sta_wpa_mode") == 1)
			fprintf(fp, "ApCliAuthMode=%s\n", "WPAPSK");
		else
			fprintf(fp, "ApCliAuthMode=%s\n", "WPA2PSK");
		
		//EncrypType
		p_str = nvram_wlan_get(prefix, "sta_crypto");
		if (!strcmp(p_str, "tkip"))
			fprintf(fp, "ApCliEncrypType=%s\n", "TKIP");
		else
			fprintf(fp, "ApCliEncrypType=%s\n", "AES");
		
		fprintf(fp, "ApCliWPAPSK=%s\n", nvram_wlan_get(prefix, "sta_wpa_psk"));
	}
	else
	{
		fprintf(fp, "ApCliAuthMode=%s\n", "OPEN");
		fprintf(fp, "ApCliEncrypType=%s\n", "NONE");
		fprintf(fp, "ApCliWPAPSK=%s\n", "");
	}

	fprintf(fp, "ApCliDefaultKeyID=%d\n", 0);
	for (i = 1; i <= 4; i++) {
		fprintf(fp, "ApCliKey%dType=%d\n", i, 0);
		fprintf(fp, "ApCliKey%dStr=\n", i);
	}

	//ApCliAPSDCapable
	i_val = nvram_wlan_get_int(prefix, "APSDCapable");
	if (i_val) i_val = 1;
	if (!i_wmm) i_val = 0;
	fprintf(fp, "ApCliAPSDCapable=%d\n", i_val);

	//RadioOn
	fprintf(fp, "RadioOn=%d\n", 1);

	// IgmpSnEnable (internal IGMP Snooping)
	i_val = 0;
#if defined(USE_RT3352_MII)
	if (!is_aband) {
		i_val = nvram_wlan_get_int(prefix, "IgmpSnEnable");
		if (i_val) i_val = 1;
	}
#endif
	fprintf(fp, "IgmpSnEnable=%d\n", i_val);

	/*	McastPhyMode, PHY mode for Multicast frames
	 *	McastMcs, MCS for Multicast frames, ranges from 0 to 15
	 *
	 *	MODE=2, MCS=0: Legacy OFDM 6Mbps
	 *	MODE=2, MCS=1: Legacy OFDM 9Mbps
	 *	MODE=2, MCS=2: Legacy OFDM 12Mbps
	 *	MODE=2, MCS=3: Legacy OFDM 18Mbps
	 *	MODE=2, MCS=4: Legacy OFDM 24Mbps
	 * 	MODE=2, MCS=5: Legacy OFDM 36Mbps
	 *	MODE=2, MCS=6: Legacy OFDM 48Mbps
	 *	MODE=2, MCS=7: Legacy OFDM 54Mbps
	 *
	 *	MODE=3, MCS=0: HTMIX 6.5/15Mbps
	 *	MODE=3, MCS=1: HTMIX 15/30Mbps
	 *	MODE=3, MCS=2: HTMIX 19.5/45Mbps
	 *	MODE=3, MCS=8: HTMIX 13/30Mbps
	 *	MODE=3, MCS=9: HTMIX 26/60Mbps
	 */

	i_mphy = 2; // OFDM
	i_mmcs = 0; // 6 Mbps

	i_val = nvram_wlan_get_int(prefix, "mrate");
	switch (i_val)
	{
	case 0: // Auto
		i_mphy = 0;
		i_mmcs = 0;
		break;
	case 1: // CCK 1 Mbps
		if (!is_aband) {
			i_mphy = 1;
			i_mmcs = 0;
		}
		break;
	case 2: // CCK 2 Mbps
		if (!is_aband) {
			i_mphy = 1;
			i_mmcs = 1;
		}
		break;
	case 3: // OFDM 6 Mbps
		i_mphy = 2;
		i_mmcs = 0;
		break;
	case 4: // OFDM 9 Mbps
		i_mphy = 2;
		i_mmcs = 1;
		break;
	case 5: // OFDM 12 Mbps
		i_mphy = 2;
		i_mmcs = 2;
		break;
	case 6: // HTMIX (1S) 6.5-15 Mbps
		i_mphy = 3;
		i_mmcs = 0;
		break;
	case 7: // HTMIX (1S) 15-30 Mbps
		i_mphy = 3;
		i_mmcs = 1;
		break;
	}

	fprintf(fp, "McastPhyMode=%d\n", i_mphy);
	fprintf(fp, "McastMcs=%d\n", i_mmcs);

#if defined (USE_RT3352_MII)
	if (!is_aband) {
		fprintf(fp, "ExtEEPROM=%d\n", 1);
		if (!get_ap_mode()) {
			fprintf(fp, "VLAN_ID=%d;%d\n", 1, INIC_GUEST_VLAN_VID);
			fprintf(fp, "VLAN_TAG=%d;%d\n", 0, 0);
			fprintf(fp, "VLAN_Priority=%d;%d\n", 0, 0);
			fprintf(fp, "SwitchRemoveTag=1;1;1;1;1;0;0\n"); // RT3352 embedded switch
		}
	}
#endif

	load_user_config(fp, "/etc/storage/wlan", (is_aband) ? "AP_5G.dat" : "AP.dat", NULL);

	fclose(fp);

	return 0;
}

int
gen_ralink_config_2g(int disable_autoscan)
{
	return gen_ralink_config(BOARD_2G_IN_SOC, 0, disable_autoscan);
}

int
gen_ralink_config_5g(int disable_autoscan)
{
#if BOARD_HAS_5G_RADIO
	return gen_ralink_config(BOARD_5G_IN_SOC, 1, disable_autoscan);
#else
	return -1;
#endif
}

static int
wl_ioctl(char *ifname, int cmd, struct iwreq *pwrq)
{
	int ret = 0;
 	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	/* do it */
	strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
	if ((ret = ioctl(s, cmd, pwrq)) < 0)
		perror(pwrq->ifr_name);

	/* cleanup */
	close(s);
	return ret;
}

int
get_apcli_connected(char *ifname)
{
	int apcli_con = 0;
	struct iwreq wrq;

	if (wl_ioctl(ifname, SIOCGIWAP, &wrq) >= 0) {
		wrq.u.ap_addr.sa_family = ARPHRD_ETHER;
		if (wrq.u.ap_addr.sa_data[0] ||
		    wrq.u.ap_addr.sa_data[1] ||
		    wrq.u.ap_addr.sa_data[2] ||
		    wrq.u.ap_addr.sa_data[3] ||
		    wrq.u.ap_addr.sa_data[4] ||
		    wrq.u.ap_addr.sa_data[5])
			apcli_con = 1;
	}

	return apcli_con;
}


