#!/bin/sh

dir_storage="/etc/storage/dropbear"
rsa_key="$dir_storage/rsa_host_key"
dss_key="$dir_storage/dss_host_key"
ecdsa_key="$dir_storage/ecdsa_host_key"

func_createkeys()
{
	rm -f "$rsa_key"
	rm -f "$dss_key"
	rm -f "$ecdsa_key"
	
	[ ! -d "$dir_storage" ] && mkdir -p -m 755 $dir_storage
	/usr/bin/dropbearkey -t rsa -f "$rsa_key"
	/usr/bin/dropbearkey -t dss -f "$dss_key"
	/usr/bin/dropbearkey -t ecdsa -f "$ecdsa_key"
	chmod 600 "$rsa_key"
	chmod 600 "$dss_key"
	chmod 600 "$ecdsa_key"
}

func_start()
{
	key_s=""
	key_x="-x"
	key_4=""
	
	[ ! -d "$dir_storage" ] && mkdir -p -m 755 $dir_storage
	
	old_pattern="/etc/storage/dropbear_"
	for i in rsa_host_key dss_host_key ecdsa_host_key ; do
		[ -f "${old_pattern}_${i}" ] && mv -n "${old_pattern}_${i}" "$dir_storage/${i}"
	done
	
	if [ ! -f "$rsa_key" ] || [ ! -f "$dss_key" ] ; then
		func_createkeys
	fi
	
	if [ ! -f "$ecdsa_key" ] ; then
		/usr/bin/dropbearkey -t ecdsa -f "$ecdsa_key"
		chmod 600 "$ecdsa_key"
	fi
	
	if [ -n "$1" ] ; then
		key_s="-s"
	fi
	
	db_kex_new=`nvram get db_kex_new`
	if [ "$db_kex_new" == "1" ] ; then
		key_x=""
	fi
	
	ip6_service=`nvram get ip6_service`
	if [ -z "$ip6_service" ] && [ -d /proc/sys/net/ipv6 ] ; then
		key_4="-4"
	fi
	
	/usr/sbin/dropbear $key_x $key_4 $key_s
}

func_stop()
{
	killall -q dropbear
}

case "$1" in
start)
	func_start $2
	;;
stop)
	func_stop
	;;
newkeys)
	func_createkeys
	;;
*)
	echo "Usage: $0 {start|stop|newkeys}"
	exit 1
	;;
esac

exit 0