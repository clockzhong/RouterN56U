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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>

#include "rc.h"

#define foreach_x(x)		for (i=0; i<nvram_get_int(x); i++)

#define BATTLENET_PORT		6112
#define TRANSMISSION_PPORT	51413
#define TRANSMISSION_RPORT	9091
#define ARIA_PPORT		16888
#define ARIA_RPORT		6800

#define MODULE_WEBSTR_MASK	0x01

#define DAYS_MATCH		" --kerneltz --weekdays "

/* state match - xt_conntrack or xt_state (xt_conntrack more slower than xt_state) */
#if defined (USE_MATCH_CONNTRACK)
#define CT_STATE		"conntrack --ctstate"
#else
#define CT_STATE		"state --state"
#endif

char *g_buf;
char g_buf_pool[1024];

static void
g_buf_init()
{
	g_buf = g_buf_pool;
	g_buf_pool[0] = 0;
}

static char *
g_buf_alloc(char *g_buf_now)
{
	g_buf += strlen(g_buf_now)+1;

	return (g_buf_now);
}

static char *
proto_conv(char *proto_name, int idx)
{
	char *proto;
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", proto_name, idx);
	proto=nvram_safe_get(itemname_arr);

	if (!strncasecmp(proto, "Both", 3)) strcpy(g_buf, "both");
	else if (!strncasecmp(proto, "TCP", 3)) strcpy(g_buf, "tcp");
	else if (!strncasecmp(proto, "UDP", 3)) strcpy(g_buf, "udp");
	else if (!strncasecmp(proto, "OTHER", 5)) strcpy(g_buf, "other");
	else strcpy(g_buf,"tcp");

	return (g_buf_alloc(g_buf));
}

static char *
protoflag_conv(char *proto_name, int idx, int isFlag)
{
	char *proto;
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", proto_name, idx);
	proto=nvram_safe_get(itemname_arr);

	strcpy(g_buf, "");

	if (!isFlag)
	{
		if (strncasecmp(proto, "UDP", 3)==0) strcpy(g_buf, "udp");
		else strcpy(g_buf, "tcp");
	}
	else
	{
		if (strlen(proto)>3)
		{
			strcpy(g_buf, proto+4);
		}
		else strcpy(g_buf,"");
	}

	return (g_buf_alloc(g_buf));
}

static char *
portrange_conv(char *port_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", port_name, idx);
	strcpy(g_buf, nvram_safe_get(itemname_arr));

	return (g_buf_alloc(g_buf));
}

static char *
ip_conv(char *ip_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	sprintf(g_buf, "%s", nvram_safe_get(itemname_arr));

	return (g_buf_alloc(g_buf));
}

static char *
general_conv(char *ip_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	sprintf(g_buf, "%s", nvram_safe_get(itemname_arr));

	return (g_buf_alloc(g_buf));
}

static char *
filter_conv(char *proto, char *flag, char *srcip, char *srcport, char *dstip, char *dstport)
{
	char newstr[64];

	strcpy(g_buf, "");

	if (strcmp(proto, "")!=0)
	{
		sprintf(newstr, " -p %s", proto);
		strcat(g_buf, newstr);
	}

	if (strcmp(flag, "")!=0)
	{
		sprintf(newstr, " --tcp-flags %s %s", flag, flag);
		strcat(g_buf, newstr);
	}
	
	if (strcmp(srcip, "")!=0)
	{
		if (strchr(srcip , '-'))
			sprintf(newstr, " --src-range %s", srcip);
		else	
			sprintf(newstr, " -s %s", srcip);
		strcat(g_buf, newstr);
	}

	if (strcmp(srcport, "")!=0)
	{
		sprintf(newstr, " --sport %s", srcport);
		strcat(g_buf, newstr);
	}

	if (strcmp(dstip, "")!=0)
	{
		if (strchr(dstip, '-'))
			sprintf(newstr, " --dst-range %s", dstip);
		else	
			sprintf(newstr, " -d %s", dstip);
		strcat(g_buf, newstr);
	}

	if (strcmp(dstport, "")!=0)
	{
		sprintf(newstr, " --dport %s", dstport);
		strcat(g_buf, newstr);
	}

	return (g_buf_alloc(g_buf));
}

static void
timematch_conv(char *mstr, char *nv_date, char *nv_time)
{
	char *datestr[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char *time, *date;
	int i, comma;

	date = nvram_get(nv_date);
	time = nvram_get(nv_time);

	if (!date) date = "1111111";
	if (!time) time = "00002359";

	mstr[0] = 0;

	if (strlen(date) != 7 || !strcmp(date, "0000000") || strlen(time) != 8)
		return;

	if (strncmp(date, "1111111", 7) == 0 && strncmp(time, "00002359", 8) == 0) 
		return;

	sprintf(mstr, " -m time --timestart %c%c:%c%c:00 --timestop %c%c:%c%c:59%s",
		time[0], time[1], time[2], time[3], time[4], time[5], time[6], time[7], DAYS_MATCH);

	comma = 0;
	for (i=0; i<7; i++)
	{
		if (date[i] == '1')
		{
			if (comma)
				strcat(mstr, ",");
			else
				comma = 1;
			
			strcat(mstr, datestr[i]);
		}
	}
}

static char *
iprange_ex_conv(char *ip_name, int idx)
{
	char *ip;
	char itemname_arr[32];
	char startip[16], endip[16];
	int i, j, k;
	int mask;

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	ip=nvram_safe_get(itemname_arr);
	strcpy(g_buf, "");

	// scan all ip string
	i=j=k=0;
	mask=32;

	while (*(ip+i))
	{
		if (*(ip+i)=='*')
		{
			startip[j++] = '0';
			endip[k++] = '0';
			// 255 is for broadcast
			mask-=8;
		}
		else
		{
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		++i;
	}

	startip[j++] = 0;
	endip[k++] = 0;

	if (mask==32)
		sprintf(g_buf, "%s", startip);
	else if (mask==0)
		strcpy(g_buf, "");
	else sprintf(g_buf, "%s/%d", startip, mask);

	return (g_buf_alloc(g_buf));
}


static int
is_valid_filter_date(char *nv_date)
{
	char *date = nvram_get(nv_date);
	if (!date) date = "1111111";

	if (strlen(date) != 7 || !strcmp(date, "0000000"))
		return 0;

	return 1;
}

static int
is_valid_filter_time(char *nv_time, char *nv_time1, char *nv_enable, char *nv_enable1)
{
	char starttime1[8], endtime1[8];
	char starttime2[8], endtime2[8];
	int  enable1 = nvram_get_int(nv_enable);
	int  enable2 = nvram_get_int(nv_enable1);
	char *time1 = nvram_get(nv_time);
	char *time2 = nvram_get(nv_time1);
	if (!time1) time1 = "00002359";
	if (!time2) time2 = "00002359";

	memset(starttime1, 0, sizeof(starttime1));
	memset(starttime2, 0, sizeof(starttime2));
	memset(endtime1, 0, sizeof(endtime1));
	memset(endtime2, 0, sizeof(endtime2));

	if (enable1 != 1 && enable2 != 1)
		return 0;

	if (enable1 == 1)
	{
		if (strlen(time1) != 8)
			goto err;
		
		strncpy(starttime1, time1, 4);
		strncpy(endtime1, time1 + 4, 4);
		
		if (atoi(starttime1) >= atoi(endtime1))
			goto err;
	}

	if (enable2 == 1)
	{
		if (strlen(time2) != 8)
			goto err;
		
		strncpy(starttime2, time2, 4);
		strncpy(endtime2, time2 + 4, 4);
		
		if (atoi(starttime2) >= atoi(endtime2))
			goto err;
	}

	if (enable1 == 1 && enable2 == 1)
	{
		if ((atoi(starttime1) > atoi(starttime2)) && 
			((atoi(starttime2) > atoi(endtime1)) || (atoi(endtime2) > atoi(endtime1))))
			goto err;
		
		if ((atoi(starttime2) > atoi(starttime1)) && 
			((atoi(starttime1) > atoi(endtime2)) || (atoi(endtime1) > atoi(endtime2))))
			goto err;
	}

	return 1;

err:
	return 0;
}

void
ip2class(const char *addr, const char *mask, char *out_buf, size_t out_len)
{
	unsigned int val, ip;
	struct in_addr in;
	int i=0;

	// only handle class A,B,C
	val = (unsigned int)inet_addr(mask);
	ip = (unsigned int)inet_addr(addr);

	// oleg patch ~
	in.s_addr = ip & val;

	for (val = ntohl(val); val; i++)
		val <<= 1;

	snprintf(out_buf, out_len, "%s/%d", inet_ntoa(in), i);
}

// WAN, MAN, LAN
void
fill_static_routes(char *buf, int len, const char *ift)
{
	int i, len_iter;
	char buf_iter[128];
	char *ip, *netmask, *gateway, *metric, *interface;

	if (!buf || len < 32)
		return;

	buf[0] = '\0';

	foreach_x("sr_num_x")
	{
		g_buf_init();
		
		ip = general_conv("sr_ipaddr_x", i);
		netmask = general_conv("sr_netmask_x", i);
		gateway = general_conv("sr_gateway_x", i);
		metric = general_conv("sr_matric_x", i);
		interface = general_conv("sr_if_x", i);
		
		if (strcmp(interface, ift) == 0)
		{
			snprintf(buf_iter, sizeof(buf_iter), "%s:%s:%s:%d ", ip, netmask, gateway, atoi(metric));
			len_iter = strlen(buf_iter);
			if (len < (len_iter + 1))
				break;
			
			strcat(buf, buf_iter);
			
			len -= len_iter;
		}
	}

	/* remove last space */
	len_iter = strlen(buf);
	if (len_iter > 0)
		buf[len_iter-1] = '\0';
}

static int
include_mac_filter(FILE *fp, char *logdrop)
{
	int i, mac_num, mac_filter;
	char mac_timematch[128], mac_buf[64], nv_date[32], nv_time[32];
	char *filter_mac, *dtype, *ftype;
	
	mac_filter = nvram_get_int("macfilter_enable_x");
	if (mac_filter > 0)
	{
		dtype = IPT_CHAIN_NAME_MAC_LIST;
		
		if (mac_filter == 2)
			ftype = logdrop;
		else
			ftype = "RETURN";
		
		mac_num = 0;
		foreach_x("macfilter_num_x")
		{
			g_buf_init();
			
			filter_mac = mac_conv("macfilter_list_x", i, mac_buf);
			if (*filter_mac)
			{
				mac_num++;
				sprintf(nv_date, "macfilter_date_x%d", i);
				sprintf(nv_time, "macfilter_time_x%d", i);
				timematch_conv(mac_timematch, nv_date, nv_time);
				fprintf(fp, "-A %s -m mac --mac-source %s%s -j %s\n", dtype, filter_mac, mac_timematch, ftype);
			}
		}
		
		if (mac_filter != 2 && mac_num > 0)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
		
		if (mac_num < 1)
			mac_filter = 0;
	}

	return mac_filter;
}

static int
include_webstr_filter(FILE *fp)
{
	int i, url_enable, url_enable_1, webstr_items;
	char url_timematch[128], url_timematch_1[128], nvname[32], *filterstr, *dtype;

	webstr_items = 0;

	if (is_valid_filter_time("url_time_x", "url_time_x_1", "url_enable_x", "url_enable_x_1") && 
	    is_valid_filter_date("url_date_x"))
	{
		dtype = IPT_CHAIN_NAME_URL_LIST;
		
		url_enable = nvram_get_int("url_enable_x");
		url_enable_1 = nvram_get_int("url_enable_x_1");
		
		if (url_enable == 1)
			timematch_conv(url_timematch, "url_date_x", "url_time_x");
		if (url_enable_1 == 1)
			timematch_conv(url_timematch_1, "url_date_x", "url_time_x_1");
		
		foreach_x("url_num_x")
		{
			sprintf(nvname, "url_keyword_x%d", i);
			filterstr = nvram_safe_get(nvname);
			if (strncasecmp(filterstr, "http://", 7) == 0)
				filterstr += 7;
			else if (strncasecmp(filterstr, "https://", 8) == 0)
				filterstr += 8;
			
			if (*filterstr)
			{
				if (url_enable == 1)
				{
					fprintf(fp, "-A %s -p tcp%s -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n", dtype, url_timematch, filterstr);
					webstr_items++;
				}
				
				if (url_enable_1 == 1)
				{
					fprintf(fp, "-A %s -p tcp%s -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n", dtype, url_timematch_1, filterstr);
					webstr_items++;
				}
			}
		}
	}

	return webstr_items;
}

static void
include_vpns_clients(FILE *fp)
{
	FILE *fpls;
	char ifname[64] = {0};
	char *dtype = IPT_CHAIN_NAME_VPN_LIST;

	fpls = fopen(VPN_SERVER_LEASE_FILE, "r");
	if (fpls) {
		while(fscanf(fpls, "%s %*s %*s %*s\n", ifname) > 0) {
			if (ppp_ifindex(ifname) >= VPN_SERVER_PPP_UNIT)
				fprintf(fp, "-A %s -i %s -j %s\n", dtype, ifname, "ACCEPT");
		}
		
		fclose(fpls);
	}
}

static void
include_lw_filter(FILE *fp, char *lan_if, char *wan_if, char *logaccept, char *logdrop)
{
	int i, lw_enable, lw_enable_1;
	char lw_timematch[128], lw_timematch_1[128], icmp_ptr[64];
	char *icmplist, *filterstr, *dtype, *ftype, *jtype;
	char *proto, *flag, *srcip, *srcport, *dstip, *dstport;

	dtype = "FORWARD";

	if (is_valid_filter_time("filter_lw_time_x", "filter_lw_time_x_1", "fw_lw_enable_x", "fw_lw_enable_x_1") && 
	    is_valid_filter_date("filter_lw_date_x"))
	{
		lw_enable = nvram_get_int("fw_lw_enable_x");
		lw_enable_1 = nvram_get_int("fw_lw_enable_x_1");
		
		if (lw_enable == 1)
			timematch_conv(lw_timematch, "filter_lw_date_x", "filter_lw_time_x");
		if (lw_enable_1 == 1)
			timematch_conv(lw_timematch_1, "filter_lw_date_x", "filter_lw_time_x_1");
		
		if (nvram_match("filter_lw_default_x", "DROP"))
		{
			jtype = logdrop;
			ftype = logaccept;
		}
		else
		{
			jtype = logaccept;
			ftype = logdrop;
		}
		
		foreach_x("filter_lw_num_x")
		{
			g_buf_init();
			
			proto = protoflag_conv("filter_lw_proto_x", i, 0);
			flag = protoflag_conv("filter_lw_proto_x", i, 1);
			srcip = iprange_ex_conv("filter_lw_srcip_x", i);
			srcport = portrange_conv("filter_lw_srcport_x", i);
			dstip = iprange_ex_conv("filter_lw_dstip_x", i);
			dstport = portrange_conv("filter_lw_dstport_x", i);
			filterstr = filter_conv(proto, flag, srcip, srcport, dstip, dstport);
			
			if (*filterstr)
			{
				if (lw_enable == 1)
					fprintf(fp, "-A %s -i %s %s%s -j %s\n", dtype, lan_if, filterstr, lw_timematch, ftype);
				
				if (lw_enable_1 == 1)
					fprintf(fp, "-A %s -i %s %s%s -j %s\n", dtype, lan_if, filterstr, lw_timematch_1, ftype);
			}
		}
		
		// ICMP
		icmp_ptr[0] = 0;
		foreach(icmp_ptr, nvram_safe_get("filter_lw_icmp_x"), icmplist)
		{
			if (*icmp_ptr)
			{
				if (lw_enable == 1)
					fprintf(fp, "-A %s -i %s -o %s -p icmp --icmp-type %s%s -j %s\n", dtype, lan_if, wan_if, icmp_ptr, lw_timematch, ftype);
				
				if (lw_enable_1 == 1)
					fprintf(fp, "-A %s -i %s -o %s -p icmp --icmp-type %s%s -j %s\n", dtype, lan_if, wan_if, icmp_ptr, lw_timematch_1, ftype);
			}
		}
		
		// Default
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, jtype);
	}
}

static void
include_vts_filter(FILE *fp, char *lan_ip, char *logaccept, int forward_chain)
{
	int i;
	char *proto, *protono, *port, *lport, *dstip, *dtype;
	char dstports[32];

	dtype = (forward_chain) ? "FORWARD" : "INPUT";

	foreach_x("vts_num_x")
	{
		g_buf_init();
		
		dstip = ip_conv("vts_ipaddr_x", i);
		if (!is_valid_ipv4(dstip))
			continue;
		
		if (forward_chain)
		{
			if (strcmp(lan_ip, dstip) == 0)
				continue;
		}
		else
		{
			if (strcmp(lan_ip, dstip) != 0)
				continue;
		}
		
		proto = proto_conv("vts_proto_x", i);
		port = portrange_conv("vts_port_x", i);
		lport = portrange_conv("vts_lport_x", i);
		
		if (lport && strlen(lport)!=0)
			snprintf(dstports, sizeof(dstports), "%s", lport);
		else
			snprintf(dstports, sizeof(dstports), "%s", port);
		
		if (*dstports)
		{
			if (strcmp(proto, "tcp")==0 || strcmp(proto, "both")==0)
				fprintf(fp, "-A %s -p %s -d %s --dport %s -j %s\n", dtype, "tcp", dstip, dstports, logaccept);
			
			if (strcmp(proto, "udp")==0 || strcmp(proto, "both")==0)
				fprintf(fp, "-A %s -p %s -d %s --dport %s -j %s\n", dtype, "udp", dstip, dstports, logaccept);
		}
		
		if (strcmp(proto, "other")==0)
		{
			protono = portrange_conv("vts_protono_x", i);
			if (*protono)
				fprintf(fp, "-A %s -p %s -d %s -j %s\n", dtype, protono, dstip, logaccept);
		}
	}
}

static void
include_vts_nat(FILE *fp)
{
	int i;
	char *proto, *protono, *port, *lport, *dstip, *dtype;
	
	dtype = IPT_CHAIN_NAME_VSERVER;
	
	foreach_x("vts_num_x")
	{
		g_buf_init();
		
		dstip = ip_conv("vts_ipaddr_x", i);
		if (!is_valid_ipv4(dstip))
			continue;
		
		proto = proto_conv("vts_proto_x", i);
		port = portrange_conv("vts_port_x", i);
		lport = portrange_conv("vts_lport_x", i);
		
		if (strcmp(proto, "tcp")==0 || strcmp(proto, "both")==0)
		{
			if (lport && strlen(lport)!=0 && strcmp(port, lport)!=0)
				fprintf(fp, "-A %s -p %s --dport %s -j DNAT --to-destination %s:%s\n", dtype, "tcp", port, dstip, lport);
			else
				fprintf(fp, "-A %s -p %s --dport %s -j DNAT --to %s\n", dtype, "tcp", port, dstip);
		}
		
		if (strcmp(proto, "udp")==0 || strcmp(proto, "both")==0)
		{
			if (lport && strlen(lport)!=0 && strcmp(port, lport)!=0)
				fprintf(fp, "-A %s -p %s --dport %s -j DNAT --to-destination %s:%s\n", dtype, "udp", port, dstip, lport);
			else
				fprintf(fp, "-A %s -p %s --dport %s -j DNAT --to %s\n", dtype, "udp", port, dstip);
		}
		
		if (strcmp(proto, "other")==0)
		{
			protono = portrange_conv("vts_protono_x", i);
			if (*protono)
				fprintf(fp, "-A %s -p %s -j DNAT --to %s\n", dtype, protono, dstip);
		}
	}
}

static void
include_masquerade(FILE *fp, char *wan_if, char *wan_ip, char *lan_net)
{
	char *dtype = "POSTROUTING";

	if (wan_ip)
		fprintf(fp, "-A %s -o %s -s %s -j SNAT --to-source %s\n", dtype, wan_if, lan_net, wan_ip);
	else
		fprintf(fp, "-A %s -o %s -s %s -j MASQUERADE\n", dtype, wan_if, lan_net);
}

static int
is_need_tcp_mss_wan(int unit, char *man_if)
{
	if (get_usb_modem_wan(unit) ) {
		int modem_mtu = nvram_safe_get_int("modem_mtu", 1500, 1000, 1500);
		if (modem_mtu != 1500)
			return 1;
	} else {
		int wan_proto = get_wan_proto(unit);
		if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
		    wan_proto == IPV4_WAN_PROTO_PPTP ||
		    wan_proto == IPV4_WAN_PROTO_L2TP)
			return 1;
		
		if (get_interface_mtu(man_if) != 1500)
			return 1;
	}

	return 0;
}

static char *
get_tcp_mss_ifname_vpns(int vpns_type)
{
#if defined (APP_OPENVPN)
	/* NOTE: Not use TCPMSS target for OpenVPN tunX interface:
	   1) do not use --tun-mtu or --link-mtu (this is changed MTU on tunX interface)
	   2) please use --fragment or/and --mssfix
	*/
	if (vpns_type == 2)
		return NULL;
	else
#endif
		return "ppp+";
}

static char *
get_tcp_mss_ifname_vpnc(int vpnc_type)
{
#if defined (APP_OPENVPN)
	/* NOTE: Not use TCPMSS target for OpenVPN tunX interface:
	   1) do not use --tun-mtu or --link-mtu (this is changed MTU on tunX interface)
	   2) please use --fragment or/and --mssfix
	*/
	if (vpnc_type == 2)
		return NULL;
	else
#endif
		return IFNAME_CLIENT_PPP;
}

static int
get_sshd_bfp_time(int bfp_mode)
{
	int i_bfp_time = 3600;		// 60 min

	if (bfp_mode == 1)
		i_bfp_time = 60;	// 1 min
	else if (bfp_mode == 2)
		i_bfp_time = 300;	// 5 min
	else if (bfp_mode == 3)
		i_bfp_time = 600;	// 10 min
	else if (bfp_mode == 4)
		i_bfp_time = 1800;	// 30 min

	return i_bfp_time;
}

static int
ipt_filter_rules(char *man_if, char *wan_if, char *lan_if, char *lan_ip,
                 char *logaccept, char *logdrop, int tcp_mss_need)
{
	FILE *fp;
	char *ftype, *dtype;
	const char *ipt_file = "/tmp/ipt_filter.rules";
	int ret, wport, lport;
	int is_nat_enabled, is_dos_enabled, is_fw_enabled, is_logaccept, is_logdrop;
	int i_vpns_enable, i_vpns_type, i_vpns_actl, i_http_proto, i_bfplimit_ref;
	int i_vpnc_enable, i_vpnc_type, i_vpnc_sfw, i_mac_filter;
#if defined (APP_OPENVPN)
	int i_vpns_ov_mode = nvram_get_int("vpns_ov_mode");
	int i_vpnc_ov_mode = nvram_get_int("vpnc_ov_mode");
#endif
#if defined (USE_IPV6)
	int ipv6_type = get_ipv6_type();
#endif

	ret = 0;
	i_bfplimit_ref = 0;

	is_logaccept = (strcmp(logaccept, IPT_CHAIN_NAME_LOG_ACCEPT) == 0) ? 1 : 0;
	is_logdrop = (strcmp(logdrop, IPT_CHAIN_NAME_LOG_DROP) == 0) ? 1 : 0;

	is_nat_enabled = nvram_match("wan_nat_x", "1");
	is_dos_enabled = nvram_match("fw_dos_x", "1");
	is_fw_enabled  = nvram_match("fw_enable_x", "1");

	i_vpns_enable  = nvram_get_int("vpns_enable");
	i_vpns_type    = nvram_get_int("vpns_type");
	i_vpns_actl    = nvram_get_int("vpns_actl");

	i_vpnc_enable  = nvram_get_int("vpnc_enable");
	i_vpnc_type    = nvram_get_int("vpnc_type");
	i_vpnc_sfw     = nvram_get_int("vpnc_sfw");

	if (!(fp=fopen(ipt_file, "w")))
		return 0;

	fprintf(fp, "*%s\n", "filter");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", (is_fw_enabled) ? "DROP" :  "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_FORWARD);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_VPN_LIST);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_MAC_LIST);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_URL_LIST);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_BFP_LIMIT);
	if (is_dos_enabled)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_DOS_LIMIT);
	if (is_logaccept)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LOG_ACCEPT);
	if (is_logdrop)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LOG_DROP);

	// INPUT chain
	dtype = "INPUT";

	// maclist chain
	i_mac_filter = include_mac_filter(fp, logdrop);

	/* Policy for all traffic from MAC-filtered LAN clients */
	if (i_mac_filter > 0 && nvram_match("fw_mac_drop", "1"))
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_MAC_LIST);

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", "ACCEPT");

	if (is_fw_enabled) {
		/* Accept all traffic from LAN clients */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");
		
		/* Accept all traffic from localhost */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, "lo", "ACCEPT");
		
#if defined (USE_IPV6)
		/* Accept SIT (IPv6-in-IPv4) packets (might be seen as INVALID) */
		if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
			fprintf(fp, "-A %s -p %d -j %s\n", dtype, 41, "ACCEPT");
#endif
		/* Drop the wrong state, INVALID, packets */
		fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", logdrop);
		
		/* Accept multicast (all, except udp port 1900) */
		if (nvram_match("mr_enable_x", "1") || nvram_invmatch("udpxy_enable_x", "0")
#if defined (APP_XUPNPD)
		 || nvram_invmatch("xupnpd_enable_x", "0")
#endif
		) {
			fprintf(fp, "-A %s -p 2 -d 224.0.0.0/4 -j %s\n", dtype, logaccept);
			fprintf(fp, "-A %s -p udp -d 224.0.0.0/4 ! --dport %d -j %s\n", dtype, 1900, "ACCEPT");
		}
	}

	/* DoS attack limits */
	if (is_dos_enabled)
		fprintf(fp, "-A %s -i %s -m %s %s -j %s\n", dtype, wan_if, CT_STATE, "NEW", IPT_CHAIN_NAME_DOS_LIMIT);

	if (is_fw_enabled) {
		/* Accept DHCPv4 */
		fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 67, 68, logaccept);
		
#if defined (SUPPORT_HTTPS)
		i_http_proto = nvram_get_int("http_proto");
#else
		i_http_proto = 0;
#endif
		if ((i_http_proto == 0 || i_http_proto == 2) && nvram_match("misc_http_x", "1")) {
			wport = nvram_get_int("misc_httpport_x");
			lport = nvram_get_int("http_lanport");
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, logaccept);
		}
#if defined (SUPPORT_HTTPS)
		if ((i_http_proto == 1 || i_http_proto == 2) && nvram_match("https_wopen", "1")) {
			wport = nvram_get_int("https_wport");
			lport = nvram_get_int("https_lport");
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, logaccept);
		}
#endif
#if defined (APP_SSHD)
		if (nvram_invmatch("sshd_enable", "0") && nvram_match("sshd_wopen", "1")) {
			i_bfplimit_ref++;
			wport = nvram_get_int("sshd_wport");
			lport = 22;
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, IPT_CHAIN_NAME_BFP_LIMIT);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, IPT_CHAIN_NAME_BFP_LIMIT);
		}
#endif
#if defined (APP_FTPD)
		if (nvram_invmatch("enable_ftp", "0") && nvram_match("ftpd_wopen", "1")) {
			wport = nvram_get_int("ftpd_wport");
			lport = 21;
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, logaccept);
		}
#endif
		lport = nvram_get_int("udpxy_enable_x");
		if (lport > 1023 && nvram_match("udpxy_wopen", "1")) {
			wport = nvram_get_int("udpxy_wport");
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, logaccept);
		}
#if defined (APP_TRMD)
		if (nvram_match("trmd_enable", "1") && is_torrent_support()) {
			wport = nvram_get_int("trmd_pport");
			if (wport < 1024 || wport > 65535) wport = TRANSMISSION_PPORT;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (nvram_match("trmd_ropen", "1")) {
				wport = nvram_get_int("trmd_rport");
				if (wport < 1024 || wport > 65535) wport = TRANSMISSION_RPORT;
				fprintf(fp, "-A %s -i %s -p tcp --dport %d -j %s\n", dtype, wan_if, wport, logaccept);
			}
		}
#endif
#if defined (APP_ARIA)
		if (nvram_match("aria_enable", "1") && is_aria_support()) {
			wport = nvram_get_int("aria_pport");
			if (wport < 1024 || wport > 65535) wport = ARIA_PPORT;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (nvram_match("aria_ropen", "1")) {
				wport = nvram_get_int("aria_rport");
				if (wport < 1024 || wport > 65535) wport = ARIA_RPORT;
				fprintf(fp, "-A %s -i %s -p tcp --dport %d -j %s\n", dtype, wan_if, wport, logaccept);
			}
		}
#endif
		if (i_vpns_enable) {
			int i_need_vpnlist = 1;
#if defined (APP_OPENVPN)
			if (i_vpns_type == 2) {
				char *ov_prot = (nvram_get_int("vpns_ov_prot") > 0) ? "tcp" : "udp";
				int i_ov_port = nvram_safe_get_int("vpns_ov_port", 1194, 1, 65535);
				
				if (i_vpns_ov_mode == 0)
					i_need_vpnlist = 0;
				fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, ov_prot, i_ov_port, logaccept);
			} else
#endif
			if (i_vpns_type == 1) {
				fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, "udp", 1701, logaccept);
			} else {
				fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, "tcp", 1723, logaccept);
				fprintf(fp, "-A %s -p %d -j %s\n", dtype, 47, logaccept);
			}
			
			/* Jump to vpnlist chain */
			if (i_need_vpnlist && (i_vpns_actl == 0 || i_vpns_actl == 2))
				fprintf(fp, "-A %s -j %s\n", dtype, IPT_CHAIN_NAME_VPN_LIST);
		}
		
		/* Accept to VPN client */
		if (i_vpnc_enable && i_vpnc_sfw != 1) {
#if defined (APP_OPENVPN)
			if (i_vpnc_type == 2) {
				if (i_vpnc_ov_mode == 1)
					fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_CLIENT_TUN, "ACCEPT");
				else if (nvram_get_int("vpnc_ov_cnat") == 1)
					fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_CLIENT_TAP, "ACCEPT");
			} else
#endif
			{
				fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_CLIENT_PPP, "ACCEPT");
			}
		}
		
		/* add Virtual Server rules for router host */
		if (is_nat_enabled && nvram_match("vts_enable_x", "1")) {
			include_vts_filter(fp, lan_ip, logaccept, 0);
		}
		
		/* Accept ICMPv4 packets */
		if (nvram_invmatch("misc_ping_x", "0")) {
			/* Accept all ICMP and UDP ports 33434-33534 for traceroute */
			fprintf(fp, "-A %s -p icmp -j %s\n", dtype, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d:%d -j %s\n", dtype, 33434, 33534, logaccept);
		} else {
			/* Accept all ICMP, exclude ping */
			fprintf(fp, "-A %s -p icmp ! --icmp-type %s -j %s\n", dtype, "echo-request", logaccept);
#if defined (USE_IPV6)
			if (ipv6_type == IPV6_6IN4) {
				/* pass ICMP ping from 6in4 remote endpoint */
				char *tun_remote = nvram_safe_get("ip6_6in4_remote");
				if (is_valid_ipv4(tun_remote))
					fprintf(fp, "-A %s -p icmp --icmp-type %s -s %s -j %s\n", dtype, "echo-request", tun_remote, logaccept);
			}
#endif
		}
		
		/* Drop all (only for log) */
		if (is_logdrop)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	// FORWARD chain
	dtype = "FORWARD";

	if (i_mac_filter > 0)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_MAC_LIST);

	if (is_fw_enabled) {
		/* Accept the redirect packets (might be seen as INVALID) */
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, lan_if, lan_if, "ACCEPT");
	}

	/* use url filter before accepting ESTABLISHED packets */
	if (include_webstr_filter(fp) > 0) {
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_URL_LIST);
		ret |= MODULE_WEBSTR_MASK;
	}

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
	if (!tcp_mss_need) {
		if (i_vpnc_enable) {
			char *vpnc_if = get_tcp_mss_ifname_vpnc(i_vpnc_type);
			if (vpnc_if)
				fprintf(fp, "-A %s%s -o %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype, "", vpnc_if);
		}
		if (i_vpns_enable) {
			char *vpns_if = get_tcp_mss_ifname_vpns(i_vpns_type);
			if (vpns_if)
				fprintf(fp, "-A %s%s -i %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype, "", vpns_if);
		}
	} else {
		fprintf(fp, "-A %s%s -o %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype, " !", lan_if);
	}

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", "ACCEPT");

	if (is_fw_enabled) {
		/* Drop all packets in the INVALID state */
		fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", logdrop);
		
		/* Pass multicast (do not log) */
		if (nvram_match("mr_enable_x", "1"))
			fprintf(fp, "-A %s -p udp -d 224.0.0.0/4 -j %s\n", dtype, "ACCEPT");
#if 0
		/* Filter out invalid WAN->WAN connections */
		fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, wan_if, lan_if, logdrop);
		if (strcmp(man_if, wan_if))
			fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, man_if, lan_if, logdrop);
#endif
	}

	/* DoS attacks from WAN */
	if (is_dos_enabled)
		fprintf(fp, "-A %s -i %s -m %s %s -j %s\n", dtype, wan_if, CT_STATE, "NEW", IPT_CHAIN_NAME_DOS_LIMIT);

	/* Block LAN outbound traffic for specified VPN packets */
	if (nvram_match("fw_pt_pptp", "0"))
		fprintf(fp, "-A %s -i %s -p %d -j %s\n", dtype, lan_if, 47, logdrop);
	if (nvram_match("fw_pt_ipsec", "0")) {
		fprintf(fp, "-A %s -i %s -p %d -j %s\n", dtype, lan_if, 50, logdrop);
		fprintf(fp, "-A %s -i %s -p %d -j %s\n", dtype, lan_if, 51, logdrop);
	}
	if (nvram_match("fw_pt_pptp", "0"))
		fprintf(fp, "-A %s -i %s -p tcp --dport %d -j %s\n", dtype, lan_if, 1723, logdrop);
	if (nvram_match("fw_pt_l2tp", "0"))
		fprintf(fp, "-A %s -i %s -p udp --dport %d -j %s\n", dtype, lan_if, 1701, logdrop);
	if (nvram_match("fw_pt_ipsec", "0")) {
		fprintf(fp, "-A %s -i %s -p udp --dport %d -j %s\n", dtype, lan_if, 500, logdrop);
		fprintf(fp, "-A %s -i %s -p udp --dport %d -j %s\n", dtype, lan_if, 4500, logdrop);
	}

	/* FILTER from LAN to WAN */
	include_lw_filter(fp, lan_if, wan_if, logaccept, logdrop);

	if (is_fw_enabled) {
		/* Accept LAN other outbound traffic  */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");
		
		/* Accept VPN server client's outbound traffic */
		if (i_vpns_enable) {
			int i_need_vpnlist = 1;
#if defined (APP_OPENVPN)
			if (i_vpns_type == 2 && i_vpns_ov_mode == 0)
				i_need_vpnlist = 0;
#endif
			/* Jump to vpnlist chain */
			if (i_need_vpnlist) {
				if (i_vpns_actl == 0 || i_vpns_actl == 1)
					fprintf(fp, "-A %s -j %s\n", dtype, IPT_CHAIN_NAME_VPN_LIST);
				else
					fprintf(fp, "-A %s -o %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_VPN_LIST);
			}
		}
		
		/* Accept inbound traffic from VPN client (do not log) */
		if (i_vpnc_enable && i_vpnc_sfw != 1) {
#if defined (APP_OPENVPN)
			if (i_vpnc_type == 2) {
				if (i_vpnc_ov_mode == 1)
					fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_CLIENT_TUN, "ACCEPT");
				else if (nvram_get_int("vpnc_ov_cnat") == 1)
					fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_CLIENT_TAP, "ACCEPT");
			} else
#endif
			{
				fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_CLIENT_PPP, "ACCEPT");
			}
		}
		
		if (is_nat_enabled) {
			char *dmz_ip;
			
			/* Accept to BattleNET */
			if (nvram_match("sp_battle_ips", "1"))
				fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, BATTLENET_PORT, logaccept);
			
#if defined (USE_MATCH_CONNTRACK)
			/* Accept to Virtual Servers, UPnP, DMZ */
			fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "DNAT", logaccept);
#else
			/* Accept to exposed station (DMZ) */
			dmz_ip = nvram_safe_get("dmz_ip");
			if (is_valid_ipv4(dmz_ip))
				fprintf(fp, "-A %s -d %s -j %s\n", dtype, dmz_ip, logaccept);
			
			/* Accept to Virtual Servers */
			if (nvram_match("vts_enable_x", "1"))
				include_vts_filter(fp, lan_ip, logaccept, 1);
			
			/* Jump to IGD UPnP/NAT-PMP (miniupnpd chain) */
			if (nvram_invmatch("upnp_enable_x", "0"))
				fprintf(fp, "-A %s -j %s\n", dtype, MINIUPNPD_CHAIN_IP4_FORWARD);
#endif
		}
		
		/* Drop all (only for log) */
		if (is_logdrop)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	/* vpnlist chain */
	if (i_vpns_enable) {
		dtype = IPT_CHAIN_NAME_VPN_LIST;
		ftype = "ACCEPT";
#if defined (APP_OPENVPN)
		if (i_vpns_type == 2 && i_vpns_ov_mode == 1)
			fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_SERVER_TUN, ftype);
		else
#endif
			include_vpns_clients(fp);
	}

	/* bfplimit chain */
	dtype = IPT_CHAIN_NAME_BFP_LIMIT;
	if (i_bfplimit_ref) {
		int i_bfp_mode = nvram_get_int("sshd_wbfp");
		if (i_bfp_mode > 0) {
			fprintf(fp, "-A %s -m recent --set --name %s\n", dtype, "blacklist");
			fprintf(fp, "-A %s -m recent --update --hitcount %d --seconds %d --name %s -j %s\n", dtype, 4, get_sshd_bfp_time(i_bfp_mode), "blacklist", logdrop);
		}
		fprintf(fp, "-A %s -j %s\n", dtype, logaccept);
	}

	/* doslimit chain */
	if (is_dos_enabled) {
		dtype = IPT_CHAIN_NAME_DOS_LIMIT;
		ftype = "RETURN";
		fprintf(fp, "-A %s -p tcp --syn -m limit --limit 20/s --limit-burst 30 -j %s\n", dtype, ftype);
		fprintf(fp, "-A %s -p tcp --syn -j %s\n", dtype, logdrop);
		fprintf(fp, "-A %s -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j %s\n", dtype, ftype);
		fprintf(fp, "-A %s -p tcp --tcp-flags SYN,ACK,FIN,RST RST -j %s\n", dtype, logdrop);
		fprintf(fp, "-A %s -p icmp --icmp-type %s -m limit --limit 2/s -j %s\n", dtype, "echo-request", ftype);
		fprintf(fp, "-A %s -p icmp --icmp-type %s -j %s\n", dtype, "echo-request", logdrop);
	}

	/* logaccept chain */
	if (is_logaccept) {
		dtype = IPT_CHAIN_NAME_LOG_ACCEPT;
		ftype = "ACCEPT";
		fprintf(fp, "-A %s -m %s %s -j LOG --log-prefix \"%s \" "
			    "--log-tcp-options --log-ip-options\n", dtype, CT_STATE, "NEW", ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, ftype);
	}

	/* logdrop chain */
	if (is_logdrop) {
		dtype = IPT_CHAIN_NAME_LOG_DROP;
		ftype = "DROP";
		fprintf(fp, "-A %s -m %s %s -j LOG --log-prefix \"%s \" "
			    "--log-tcp-options --log-ip-options\n", dtype, CT_STATE, "NEW,INVALID", ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, ftype);
	}

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	if (ret & MODULE_WEBSTR_MASK)
		doSystem("modprobe %s", "xt_webstr");

	doSystem("iptables-restore %s", ipt_file);

	return ret;
}

void
ipt_filter_default(void)
{
	FILE *fp;
	char *ftype, *dtype;
	int is_fw_enabled;
	const char *ipt_file = "/tmp/ipt_filter.default";

	is_fw_enabled = nvram_match("fw_enable_x", "1");

	if (!(fp=fopen(ipt_file, "w")))
		return;

	fprintf(fp, "*%s\n", "filter");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", (is_fw_enabled) ? "DROP" :  "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_FORWARD);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_VPN_LIST);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_MAC_LIST);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_URL_LIST);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_BFP_LIMIT);

	/* INPUT chain */
	dtype = "INPUT";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, "lo", ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", "DROP");
	if (is_fw_enabled)
		fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 67, 68, ftype);

	/* FORWARD chain */
	dtype = "FORWARD";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", ftype);
	fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, IFNAME_BR, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", "DROP");
	if (is_fw_enabled)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	doSystem("iptables-restore %s", ipt_file);
}

static void
ipt_mangle_rules(char *man_if)
{
	FILE *fp;
	int i_ttl_fixup;
	char *dtype;
	const char *ipt_file = "/tmp/ipt_mangle.rules";

	i_ttl_fixup = 0;
	if (nvram_match("mr_enable_x", "1"))
		i_ttl_fixup = nvram_get_int("mr_ttl_fix");

	if (i_ttl_fixup > 0) {
		module_smart_load("iptable_mangle", NULL);
		module_smart_load("xt_HL", NULL);
	}

	if (!(fp=fopen(ipt_file, "w")))
		return;

	fprintf(fp, "*%s\n", "mangle");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");

	dtype = "PREROUTING";

	if (i_ttl_fixup > 0) {
		char *viptv_iflast = nvram_safe_get("viptv_ifname");
		if (*viptv_iflast && is_interface_exist(viptv_iflast))
			man_if = viptv_iflast;
		
		if (i_ttl_fixup == 2)
			fprintf(fp, "-A %s -i %s -p udp -d 224.0.0.0/4 -j TTL %s %d\n", dtype, man_if, "--ttl-set", 64);
		else
			fprintf(fp, "-A %s -i %s -p udp -d 224.0.0.0/4 -j TTL %s %d\n", dtype, man_if, "--ttl-inc", 1);
	}

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	if (i_ttl_fixup > 0 || is_module_loaded("iptable_mangle"))
		doSystem("iptables-restore %s", ipt_file);
}

static void
ipt_raw_rules(void)
{
	FILE *fp;
	const char *ipt_file = "/tmp/ipt_raw.rules";

	if (!(fp=fopen(ipt_file, "w")))
		return;

	fprintf(fp, "*%s\n", "raw");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	if (is_module_loaded("iptable_raw"))
		doSystem("iptables-restore %s", ipt_file);
}

#if defined (USE_IPV6)
static int
ip6t_filter_rules(char *man_if, char *wan_if, char *lan_if,
                  char *logaccept, char *logdrop, int tcp_mss_need)
{
	FILE *fp;
	char *ftype, *dtype;
	int ret, wport, lport;
	int ipv6_type, is_fw_enabled, is_logaccept, is_logdrop;
	int i_http_proto, i_bfplimit_ref, i_mac_filter;
	const char *ipt_file = "/tmp/ip6t_filter.rules";

	ret = 0;
	i_bfplimit_ref = 0;

	ipv6_type = get_ipv6_type();

	is_fw_enabled = nvram_match("fw_enable_x", "1");

	is_logaccept = (strcmp(logaccept, IPT_CHAIN_NAME_LOG_ACCEPT) == 0) ? 1 : 0;
	is_logdrop = (strcmp(logdrop, IPT_CHAIN_NAME_LOG_DROP) == 0) ? 1 : 0;

	if (!(fp=fopen(ipt_file, "w")))
		return 0;

	fprintf(fp, "*%s\n", "filter");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", (is_fw_enabled) ? "DROP" :  "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP6_FORWARD);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_MAC_LIST);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_URL_LIST);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_BFP_LIMIT);
	if (is_logaccept)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LOG_ACCEPT);
	if (is_logdrop)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LOG_DROP);

	// INPUT chain (accept_source_route=0 by default, no needed drop RH0 packet)
	dtype = "INPUT";

	// maclist chain
	i_mac_filter = include_mac_filter(fp, logdrop);

	/* Policy for all traffic from MAC-filtered LAN clients */
	if (i_mac_filter > 0 && nvram_match("fw_mac_drop", "1"))
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_MAC_LIST);

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", "ACCEPT");

	if (is_fw_enabled) {
		/* Accept all traffic from LAN clients */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");
		
		/* Accept all traffic from localhost */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, "lo", "ACCEPT");
		
		/* Accept ICMPv6 (ICMPv6 may be INVALID state!) */
		if (nvram_invmatch("misc_ping_x", "0"))
			fprintf(fp, "-A %s -p %d -j %s\n", dtype, 58, logaccept);
		else
			fprintf(fp, "-A %s -p %d ! --icmpv6-type %s -j %s\n", dtype, 58, "echo-request", logaccept);
		
		/* Drop the wrong state, INVALID, packets */
		fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", logdrop);
		
		/* Accept all multicast (do not log) */
		if (nvram_match("mr_enable_x", "1"))
			fprintf(fp, "-A %s -d ff00::/8 -j %s\n", dtype, "ACCEPT");
		
		/* Accept DHCPv6 */
		if (ipv6_type == IPV6_NATIVE_DHCP6)
			fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 547, 546, logaccept);
		
		// Firewall between WAN and Local
		
		/* http/https/ssh/ftp/udpxy accepted from wan only for original ports (no NAT in IPv6) */
#if defined (SUPPORT_HTTPS)
		i_http_proto = nvram_get_int("http_proto");
#else
		i_http_proto = 0;
#endif
		wport = nvram_get_int("misc_httpport_x");
		lport = nvram_get_int("http_lanport");
		if ((i_http_proto == 0 || i_http_proto == 2) && nvram_match("misc_http_x", "1") && (wport == lport))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
#if defined (SUPPORT_HTTPS)
		wport = nvram_get_int("https_wport");
		lport = nvram_get_int("https_lport");
		if ((i_http_proto == 1 || i_http_proto == 2) && nvram_match("https_wopen", "1") && (wport == lport))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
#endif
#if defined (APP_SSHD)
		wport = nvram_get_int("sshd_wport");
		lport = 22;
		if (nvram_invmatch("sshd_enable", "0") && nvram_match("sshd_wopen", "1") && (wport == lport)) {
			i_bfplimit_ref++;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, IPT_CHAIN_NAME_BFP_LIMIT);
		}
#endif
#if defined (APP_FTPD)
		wport = nvram_get_int("ftpd_wport");
		lport = 21;
		if (nvram_invmatch("enable_ftp", "0") && nvram_match("ftpd_wopen", "1") && (wport == lport))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
#endif
		wport = nvram_get_int("udpxy_wport");
		lport = nvram_get_int("udpxy_enable_x");
		if (lport > 1023 && nvram_match("udpxy_wopen", "1") && (wport == lport))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
#if defined (APP_TRMD)
		if (nvram_match("trmd_enable", "1") && is_torrent_support()) {
			wport = nvram_get_int("trmd_pport");
			if (wport < 1024 || wport > 65535) wport = TRANSMISSION_PPORT;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (nvram_match("trmd_ropen", "1")) {
				wport = nvram_get_int("trmd_rport");
				if (wport < 1024 || wport > 65535) wport = TRANSMISSION_RPORT;
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			}
		}
#endif
#if defined (APP_ARIA)
		if (nvram_match("aria_enable", "1") && is_aria_support()) {
			wport = nvram_get_int("aria_pport");
			if (wport < 1024 || wport > 65535) wport = ARIA_PPORT;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (nvram_match("aria_ropen", "1")) {
				wport = nvram_get_int("aria_rport");
				if (wport < 1024 || wport > 65535) wport = ARIA_RPORT;
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			}
		}
#endif
		if (nvram_match("vpns_enable", "1")) {
			int i_vpns_type = nvram_get_int("vpns_type");
#if defined (APP_OPENVPN)
			if (i_vpns_type == 2) {
				char *ov_prot = "udp";
				int i_ov_port = nvram_safe_get_int("vpns_ov_port", 1194, 1, 65535);
				if (nvram_get_int("vpns_ov_prot") > 0)
					ov_prot = "tcp";
				fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, ov_prot, i_ov_port, logaccept);
			} else
#endif
			if (i_vpns_type == 1) {
				fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, 1701, logaccept);
			} else {
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, 1723, logaccept);
				fprintf(fp, "-A %s -p %d -j %s\n", dtype, 47, logaccept);
			}
		}
		
		/* Accept UDP ports 33434-33534 for traceroute */
		if (nvram_invmatch("misc_ping_x", "0"))
			fprintf(fp, "-A %s -p udp --dport %d:%d -j %s\n", dtype, 33434, 33534, logaccept);
		
		/* Drop all (only for log) */
		if (is_logdrop)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	// FORWARD chain (accept_source_route=0 by default, no needed drop RH0 packet)
	dtype = "FORWARD";

	if (i_mac_filter > 0)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_MAC_LIST);

	if (is_fw_enabled) {
		/* Pass the redirect, might be seen as INVALID, packets */
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, lan_if, lan_if, "ACCEPT");
		
		/* Pass ICMPv6 (ICMPv6 may be INVALID state!) */
		if (nvram_invmatch("misc_ping_x", "0"))
			fprintf(fp, "-A %s -p %d -j %s\n", dtype, 58, logaccept);
		else
			fprintf(fp, "-A %s -p %d ! --icmpv6-type %s -j %s\n", dtype, 58, "echo-request", logaccept);
	}

	/* use url filter before accepting ESTABLISHED packets */
	if (include_webstr_filter(fp) > 0) {
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_URL_LIST);
		ret |= MODULE_WEBSTR_MASK;
	}

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
	if ((ipv6_type != IPV6_NATIVE_STATIC && ipv6_type != IPV6_NATIVE_DHCP6) || tcp_mss_need)
		fprintf(fp, "-A %s%s -o %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype, " !", lan_if);

	/* Pass related connections, skip rest of checks */
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", "ACCEPT");

	if (is_fw_enabled) {
		/* Drop all packets in the INVALID state */
		fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", logdrop);
		
		/* Pass multicast (do not log) */
		if (nvram_match("mr_enable_x", "1"))
			fprintf(fp, "-A %s -d ff00::/8 -j %s\n", dtype, "ACCEPT");
#if 0
		/* Filter out invalid WAN->WAN connections */
		if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
			fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, IFNAME_SIT, lan_if, logdrop);
		fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, wan_if, lan_if, logdrop);
		if (!is_wan_ipv6_if_ppp() && strcmp(man_if, wan_if))
			fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, man_if, lan_if, logdrop);
#endif
		/* Accept LAN other outbound traffic  */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");
		
		/* Jump to IPv6 pinhole (IGDv2 IP6FC or PCP) */
		if (nvram_invmatch("upnp_enable_x", "0"))
			fprintf(fp, "-A %s -j %s\n", dtype, MINIUPNPD_CHAIN_IP6_FORWARD);
		
		/* Drop all (only for log) */
		if (is_logdrop)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	// bfplimit chain
	dtype = IPT_CHAIN_NAME_BFP_LIMIT;
	if (i_bfplimit_ref) {
		int i_bfp_mode = nvram_get_int("sshd_wbfp");
		if (i_bfp_mode > 0) {
			fprintf(fp, "-A %s -m recent --set --name %s\n", dtype, "blacklist6");
			fprintf(fp, "-A %s -m recent --update --hitcount %d --seconds %d --name %s -j %s\n", dtype, 4, get_sshd_bfp_time(i_bfp_mode), "blacklist6", logdrop);
		}
		fprintf(fp, "-A %s -j %s\n", dtype, logaccept);
	}

	// logaccept chain
	if (is_logaccept) {
		dtype = IPT_CHAIN_NAME_LOG_ACCEPT;
		ftype = "ACCEPT";
		fprintf(fp, "-A %s -m %s %s -j LOG --log-prefix \"%s \" "
			    "--log-tcp-options --log-ip-options\n", dtype, CT_STATE, "NEW", ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, ftype);
	}

	// logdrop chain
	if (is_logdrop) {
		dtype = IPT_CHAIN_NAME_LOG_DROP;
		ftype = "DROP";
		fprintf(fp, "-A %s -m %s %s -j LOG --log-prefix \"%s \" "
			    "--log-tcp-options --log-ip-options\n", dtype, CT_STATE, "NEW,INVALID", ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, ftype);
	}

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	if (ret & MODULE_WEBSTR_MASK)
		doSystem("modprobe %s", "xt_webstr");

	doSystem("ip6tables-restore %s", ipt_file);

	return ret;
}

void
ip6t_filter_default(void)
{
	FILE *fp;
	char *ftype, *dtype;
	int is_fw_enabled;
	const char *ipt_file = "/tmp/ip6t_filter.default";

	is_fw_enabled = nvram_match("fw_enable_x", "1");

	if (!(fp=fopen(ipt_file, "w")))
		return;

	fprintf(fp, "*%s\n", "filter");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", (is_fw_enabled) ? "DROP" :  "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP6_FORWARD);

	// INPUT chain
	dtype = "INPUT";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, "lo", ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -p %d ! --icmpv6-type %s -j %s\n", dtype, 58, "echo-request", ftype);
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", "DROP");
	if (is_fw_enabled)
		fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 547, 546, ftype);

	// FORWARD chain
	dtype = "FORWARD";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", ftype);
	fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, IFNAME_BR, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -p %d -j %s\n", dtype, 58, ftype);
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", "DROP");
	if (is_fw_enabled)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	doSystem("ip6tables-restore %s", ipt_file);
}

static void
ip6t_mangle_rules(char *man_if)
{
	FILE *fp;
	const char *ipt_file = "/tmp/ip6t_mangle.rules";

	if (!(fp=fopen(ipt_file, "w")))
		return;

	fprintf(fp, "*%s\n", "mangle");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	if (is_module_loaded("ip6table_mangle"))
		doSystem("ip6tables-restore %s", ipt_file);
}
#endif

static int
ipt_nat_rules(char *man_if, char *man_ip,
              char *wan_if, char *wan_ip,
              char *lan_if, char *lan_ip, char *lan_net,
              int use_man)
{
	FILE *fp;
	int wport, lport, is_nat_enabled, is_fw_enabled;
	int i_vpns_enable, i_vpnc_enable, i_vpns_type, i_vpnc_type, i_vpnc_sfw, i_http_proto;
	char *dtype;
	const char *ipt_file = "/tmp/ipt_nat.rules";

	is_nat_enabled = nvram_match("wan_nat_x", "1");
	is_fw_enabled = nvram_match("fw_enable_x", "1");

	i_vpns_enable = nvram_get_int("vpns_enable");
	i_vpnc_enable = nvram_get_int("vpnc_enable");
	i_vpns_type = nvram_get_int("vpns_type");
	i_vpnc_type = nvram_get_int("vpnc_type");
	i_vpnc_sfw = nvram_get_int("vpnc_sfw");

	if (!is_valid_ipv4(wan_ip))
		wan_ip = NULL;

	if (!is_valid_ipv4(man_ip))
		man_ip = NULL;

	if (!(fp=fopen(ipt_file, "w")))
		return 0;

	fprintf(fp, "*%s\n", "nat");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_VSERVER);
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_NAT);

	// VSERVER chain
	dtype = IPT_CHAIN_NAME_VSERVER;

	if (wan_ip)
		fprintf(fp, "-A PREROUTING -d %s -j %s\n", wan_ip, dtype);

	if (man_ip && use_man)
		fprintf(fp, "-A PREROUTING -d %s -j %s\n", man_ip, dtype);

	if (is_nat_enabled) {
		char dmz_ip[16];
		int is_use_dmz;
		
		snprintf(dmz_ip, sizeof(dmz_ip), "%s", nvram_safe_get("dmz_ip"));
		is_use_dmz = (is_valid_ipv4(dmz_ip)) ? 1 : 0;
		
		/* BattleNET (PREROUTING + POSTROUTING) */
		if (wan_ip && nvram_match("sp_battle_ips", "1")) {
			fprintf(fp, "-A %s -p udp -d %s --sport %d -j NETMAP --to %s\n", "PREROUTING", wan_ip, BATTLENET_PORT, lan_net);
			fprintf(fp, "-A %s -p udp -s %s --dport %d -j NETMAP --to %s\n", "POSTROUTING", lan_net, BATTLENET_PORT, wan_ip);
		}
		
		/* masquerade WAN connection for LAN clients */
		include_masquerade(fp, wan_if, wan_ip, lan_net);
		
		/* masquerade MAN connection for LAN clients */
		if (use_man)
			include_masquerade(fp, man_if, man_ip, lan_net);
		
		/* masquerade VPN client connection for LAN clients */
		if (i_vpnc_enable && i_vpnc_sfw != 2) {
#if defined (APP_OPENVPN)
			if (i_vpnc_type == 2) {
				if (nvram_get_int("vpnc_ov_mode") == 1)
					include_masquerade(fp, IFNAME_CLIENT_TUN, NULL, lan_net);
				else if (nvram_get_int("vpnc_ov_cnat") == 1)
					include_masquerade(fp, IFNAME_CLIENT_TAP, NULL, lan_net);
			} else
#endif
			{
				include_masquerade(fp, IFNAME_CLIENT_PPP, NULL, lan_net);
			}
		}
		
		/* masquerade WAN connection for VPN server clients */
		if (i_vpns_enable) {
			char vpn_net[24] = {0};
			ip2class(nvram_safe_get("vpns_vnet"), VPN_SERVER_SUBNET_MASK, vpn_net, sizeof(vpn_net));
			if (strcmp(vpn_net, lan_net) != 0) {
				int i_vpns_vuse = nvram_get_int("vpns_vuse");
				int i_vpns_actl = nvram_get_int("vpns_actl");
#if defined (APP_OPENVPN)
				if (i_vpns_type == 2) {
					if (nvram_get_int("vpns_ov_mode") == 1) {
						if (i_vpns_actl == 0 || i_vpns_actl == 1)
							include_masquerade(fp, wan_if, wan_ip, vpn_net);
						
						/* masquerade VPN server clients to LAN */
						if (i_vpns_vuse == 2)
							include_masquerade(fp, lan_if, lan_ip, vpn_net);
					}
				} else
#endif
				{
					if (i_vpns_vuse && (i_vpns_actl == 0 || i_vpns_actl == 1))
						include_masquerade(fp, wan_if, wan_ip, vpn_net);
					
					/* masquerade VPN server clients to LAN */
					if (i_vpns_vuse == 2)
						include_masquerade(fp, lan_if, lan_ip, vpn_net);
				}
			}
		}
		
		/* masquerade LAN to LAN (NAT loopback) */
		if (nvram_match("nf_nat_loop", "1"))
			fprintf(fp, "-A POSTROUTING -o %s -s %s -d %s -j SNAT --to-source %s\n", lan_if, lan_net, lan_net, lan_ip);
		
		/* Local ports remap (http/https/ssh/ftp/udpxy) */
		if (is_fw_enabled) {
#if defined (SUPPORT_HTTPS)
			i_http_proto = nvram_get_int("http_proto");
#else
			i_http_proto = 0;
#endif
			if ((i_http_proto == 0 || i_http_proto == 2) && nvram_match("misc_http_x", "1")) {
				wport = nvram_get_int("misc_httpport_x");
				lport = nvram_get_int("http_lanport");
				if (wport < 80 || wport > 65535) {
					wport = 8080;
					nvram_set_int("misc_httpport_x", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
#if defined (SUPPORT_HTTPS)
			if ((i_http_proto == 1 || i_http_proto == 2) && nvram_match("https_wopen", "1")) {
				wport = nvram_get_int("https_wport");
				lport = nvram_get_int("https_lport");
				if (wport < 81 || wport > 65535) {
					wport = 8443;
					nvram_set_int("https_wport", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
#endif
#if defined (APP_SSHD)
			if (nvram_invmatch("sshd_enable", "0") && nvram_match("sshd_wopen", "1")) {
				wport = nvram_get_int("sshd_wport");
				lport = 22;
				if (wport < 22 || wport > 65535) {
					wport = 10022;
					nvram_set_int("sshd_wport", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
#endif
#if defined (APP_FTPD)
			if (nvram_invmatch("enable_ftp", "0") && nvram_match("ftpd_wopen", "1")) {
				wport = nvram_get_int("ftpd_wport");
				lport = 21;
				if (wport < 21 || wport > 65535) {
					wport = 21;
					nvram_set_int("ftpd_wport", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
#endif
			lport = nvram_get_int("udpxy_enable_x");
			if (lport > 1023 && nvram_match("udpxy_wopen", "1")) {
				wport = nvram_get_int("udpxy_wport");
				if (wport < 1024 || wport > 65535) {
					wport = lport;
					nvram_set_int("udpxy_wport", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
		}
		
		/* check DMZ host is set, pre-route several traffic to router local first */
		if (is_use_dmz) {
#if defined (USE_IPV6)
			int ipv6_type;
#endif
			/* pre-route for local VPN server */
			if (nvram_match("vpns_enable", "1")) {
				int i_vpns_type = nvram_get_int("vpns_type");
#if defined (APP_OPENVPN)
				if (i_vpns_type == 2) {
					char *ov_prot = "udp";
					int i_ov_port = nvram_safe_get_int("vpns_ov_port", 1194, 1, 65535);
					if (nvram_get_int("vpns_ov_prot") > 0)
						ov_prot = "tcp";
					fprintf(fp, "-A %s -p %s --dport %d -j DNAT --to-destination %s\n", 
							dtype, ov_prot, i_ov_port, lan_ip);
				} else
#endif
				if (i_vpns_type == 1) {
					fprintf(fp, "-A %s -p udp --dport %d -j DNAT --to-destination %s\n", dtype, 1701, lan_ip);
				} else {
					fprintf(fp, "-A %s -p %d -j DNAT --to-destination %s\n", dtype, 47, lan_ip);
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s\n", dtype, 1723, lan_ip);
				}
			}
#if defined (APP_TRMD)
			/* pre-route for local Transmission */
			if (nvram_match("trmd_enable", "1") && is_torrent_support()) {
				wport = nvram_get_int("trmd_pport");
				if (wport < 1024 || wport > 65535) wport = TRANSMISSION_PPORT;
				fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s\n", dtype, wport, lan_ip);
				fprintf(fp, "-A %s -p udp --dport %d -j DNAT --to-destination %s\n", dtype, wport, lan_ip);
				
				if (nvram_match("trmd_ropen", "1")) {
					wport = nvram_get_int("trmd_rport");
					if (wport < 1024 || wport > 65535) wport = TRANSMISSION_RPORT;
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s\n", dtype, wport, lan_ip);
				}
			}
#endif
#if defined (APP_ARIA)
			/* pre-route for local Aria2 */
			if (nvram_match("aria_enable", "1") && is_aria_support()) {
				wport = nvram_get_int("aria_pport");
				if (wport < 1024 || wport > 65535) wport = ARIA_PPORT;
				fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s\n", dtype, wport, lan_ip);
				fprintf(fp, "-A %s -p udp --dport %d -j DNAT --to-destination %s\n", dtype, wport, lan_ip);
				
				if (nvram_match("aria_ropen", "1")) {
					wport = nvram_get_int("aria_rport");
					if (wport < 1024 || wport > 65535) wport = ARIA_RPORT;
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s\n", dtype, wport, lan_ip);
				}
			}
#endif
#if defined (USE_IPV6)
			/* pre-route for local IPv6 (SIT) */
			ipv6_type = get_ipv6_type();
			if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
				fprintf(fp, "-A %s -p %d -j DNAT --to-destination %s\n", dtype, 41, lan_ip);
#endif
			/* pre-route for local ping */
			if (nvram_invmatch("misc_ping_x", "0"))
				fprintf(fp, "-A %s -p icmp -j DNAT --to-destination %s\n", dtype, lan_ip);
		}
		
		/* Virtual Server mappings */
		if (nvram_match("vts_enable_x", "1"))
			include_vts_nat(fp);
		
		/* IGD UPnP */
		if (nvram_invmatch("upnp_enable_x", "0"))
			fprintf(fp, "-A %s -j %s\n", dtype, MINIUPNPD_CHAIN_IP4_NAT);
		
		/* Exposed station (DMZ) */
		if (is_use_dmz)
			fprintf(fp, "-A %s -j DNAT --to %s\n", dtype, dmz_ip);
	}

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	doSystem("iptables-restore %s", ipt_file);

	return 0;
}

void
ipt_nat_default(void)
{
	FILE *fp;
	int is_nat_enabled;
	const char *ipt_file = "/tmp/ipt_nat.default";

	is_nat_enabled = nvram_match("wan_nat_x", "1");

	if (!(fp=fopen(ipt_file, "w")))
		return;

	fprintf(fp, "*%s\n", "nat");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_VSERVER);
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_NAT);

	if (is_nat_enabled) {
		char *lan_if = IFNAME_BR;
		char *lan_ip = nvram_safe_get("lan_ipaddr");
		if (is_valid_ipv4(lan_ip)) {
			char lan_net[24] = {0};
			ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_net, sizeof(lan_net));
			
			/* masquerade lan to lan (NAT loopback) */
			if (nvram_match("nf_nat_loop", "1")) {
				fprintf(fp, "-A POSTROUTING -o %s -s %s -d %s -j SNAT --to-source %s\n",
					lan_if, lan_net, lan_net, lan_ip);
			}
		}
	}

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	doSystem("iptables-restore %s", ipt_file);
}

void
start_firewall_ex(void)
{
	int unit, i_tcp_mss, i_use_man;
	char rp_path[64], logaccept[16], logdrop[16];
	char wan_if[16], man_if[16], lan_if[16];
	char wan_ip[16], man_ip[16], lan_ip[16], lan_net[24] = {0};
	const char *opt_iptables_script = "/opt/bin/update_iptables.sh";
	const char *int_iptables_script = SCRIPT_POST_FIREWALL;

	unit = 0;

	snprintf(lan_if, sizeof(lan_if), "%s", IFNAME_BR);
	snprintf(man_if, sizeof(man_if), "%s", get_man_ifname(unit));
	snprintf(wan_if, sizeof(wan_if), "%s", get_wan_unit_value(unit, "ifname_t"));

	if (strlen(wan_if) < 1)
		get_wan_ifname(wan_if);

	snprintf(lan_ip, sizeof(lan_ip), "%s", nvram_safe_get("lan_ipaddr"));
	snprintf(man_ip, sizeof(man_ip), "%s", nvram_safe_get("wanx_ipaddr"));
	snprintf(wan_ip, sizeof(wan_ip), "%s", get_wan_unit_value(unit, "ipaddr"));

	ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_net, sizeof(lan_net));

	i_tcp_mss = is_need_tcp_mss_wan(unit, man_if);

	i_use_man = 0;
	if (ppp_ifindex(wan_if) >= 0 && strcmp(man_if, wan_if)) {
		if (get_wan_proto(unit) != IPV4_WAN_PROTO_PPPOE || get_wan_unit_value_int(unit, "pppoe_man") > 0)
			i_use_man = 1;
	}

	/* mcast needs rp filter to be turned off only for non default iface */
	snprintf(rp_path, sizeof(rp_path), "/proc/sys/net/ipv4/conf/%s/rp_filter", man_if);
	if (nvram_match("mr_enable_x", "1") || nvram_invmatch("udpxy_enable_x", "0")
#if defined (APP_XUPNPD)
	 || nvram_invmatch("xupnpd_enable_x", "0")
#endif
	)
		fput_int(rp_path, 0);
	else
		fput_int(rp_path, 1);

	/* Determine the log type */
	if (nvram_match("fw_log_x", "accept") || nvram_match("fw_log_x", "both"))
		strcpy(logaccept, IPT_CHAIN_NAME_LOG_ACCEPT);
	else
		strcpy(logaccept, "ACCEPT");

	if (nvram_match("fw_log_x", "drop") || nvram_match("fw_log_x", "both"))
		strcpy(logdrop, IPT_CHAIN_NAME_LOG_DROP);
	else
		strcpy(logdrop, "DROP");

	/* IPv4 Raw rules */
	ipt_raw_rules();

	/* IPv4 Mangle rules */
	ipt_mangle_rules(man_if);

	/* IPv4 NAT rules */
	ipt_nat_rules(man_if, man_ip, wan_if, wan_ip, lan_if, lan_ip, lan_net, i_use_man);

	/* IPv4 Filter rules */
	ipt_filter_rules(man_if, wan_if, lan_if, lan_ip, logaccept, logdrop, i_tcp_mss);

#if defined (USE_IPV6)
	/* IPv6 Mangle rules */
	ip6t_mangle_rules(man_if);

	/* IPv6 Filter rules */
	ip6t_filter_rules(man_if, wan_if, lan_if, logaccept, logdrop, i_tcp_mss);
#endif

	if (check_if_file_exist(int_iptables_script))
		doSystem("%s", int_iptables_script);

	if (check_if_file_exist(opt_iptables_script))
		doSystem("%s update", opt_iptables_script);

	/* enable IPv4 forward */
	set_ipv4_forward(1);

	/* try unload unused iptables modules */
	module_smart_unload("xt_webstr", 0);
	module_smart_unload("xt_HL", 0);
	module_smart_unload("iptable_raw", 0);
	module_smart_unload("iptable_mangle", 0);
	module_smart_unload("ip6table_mangle", 0);
}

