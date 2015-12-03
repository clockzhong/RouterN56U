
static long rtl8367_ioctl(struct file *file, unsigned int req, unsigned long arg)
{
	int ioctl_result = 0;
	u32 uint_value = 0;
	u32 uint_result = 0;

	rtk_api_ret_t         retVal;
	rtk_port_linkStatus_t port_link = 0;
	rtk_data_t            port_speed = 0;
	rtk_data_t            port_duplex = 0;
	rtk_stat_port_cntr_t  port_counters;

	unsigned int uint_param = (req >> RTL8367_IOCTL_CMD_LENGTH_BITS);
	req &= ((1u << RTL8367_IOCTL_CMD_LENGTH_BITS)-1);

	mutex_lock(&asic_access_mutex);

	switch(req)
	{
	case RTL8367_IOCTL_GPIO_MODE_SET:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		gpio_set_mode(uint_value);
		break;
	case RTL8367_IOCTL_GPIO_MODE_SET_BIT:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = gpio_set_mode_bit(uint_param, uint_value);
		break;
	case RTL8367_IOCTL_GPIO_MODE_GET:
		gpio_get_mode(&uint_result);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case RTL8367_IOCTL_GPIO_PIN_SET_DIRECTION:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = gpio_set_pin_direction(uint_param, uint_value);
		break;
	case RTL8367_IOCTL_GPIO_PIN_SET_VAL:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = gpio_set_pin_value(uint_param, uint_value);
		break;
	case RTL8367_IOCTL_GPIO_PIN_GET_VAL:
		ioctl_result = gpio_get_pin_value(uint_param, &uint_result);
		if (ioctl_result == 0)
			put_user(uint_result, (unsigned int __user *)arg);
		break;

	case RTL8367_IOCTL_STATUS_LINK_PORT_WAN:
		retVal = asic_status_link_port(WAN_PORT_X, &port_link);
		if (retVal == RT_ERR_OK)
			put_user(port_link, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_LINK_PORTS_WAN:
		retVal = asic_status_link_ports(1, &port_link);
		if (retVal == RT_ERR_OK)
			put_user(port_link, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_LINK_PORTS_LAN:
		retVal = asic_status_link_ports(0, &port_link);
		if (retVal == RT_ERR_OK)
			put_user(port_link, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_LINK_PORT_LAN1:
		retVal = asic_status_link_port(LAN_PORT_1, &port_link);
		if (retVal == RT_ERR_OK)
			put_user(port_link, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_LINK_PORT_LAN2:
		retVal = asic_status_link_port(LAN_PORT_2, &port_link);
		if (retVal == RT_ERR_OK)
			put_user(port_link, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_LINK_PORT_LAN3:
		retVal = asic_status_link_port(LAN_PORT_3, &port_link);
		if (retVal == RT_ERR_OK)
			put_user(port_link, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_LINK_PORT_LAN4:
		retVal = asic_status_link_port(LAN_PORT_4, &port_link);
		if (retVal == RT_ERR_OK)
			put_user(port_link, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_LINK_CHANGED:
		uint_result = asic_status_link_changed();
		put_user(uint_result, (unsigned int __user *)arg);
		break;

	case RTL8367_IOCTL_STATUS_SPEED_PORT_WAN:
		retVal = asic_status_speed_port(WAN_PORT_X, &port_link, &port_speed, &port_duplex);
		port_speed |= (port_duplex << 8);
		port_speed |= (port_link << 16);
		if (retVal == RT_ERR_OK)
			put_user(port_speed, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_SPEED_PORT_LAN1:
		retVal = asic_status_speed_port(LAN_PORT_1, &port_link, &port_speed, &port_duplex);
		port_speed |= (port_duplex << 8);
		port_speed |= (port_link << 16);
		if (retVal == RT_ERR_OK)
			put_user(port_speed, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_SPEED_PORT_LAN2:
		retVal = asic_status_speed_port(LAN_PORT_2, &port_link, &port_speed, &port_duplex);
		port_speed |= (port_duplex << 8);
		port_speed |= (port_link << 16);
		if (retVal == RT_ERR_OK)
			put_user(port_speed, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_SPEED_PORT_LAN3:
		retVal = asic_status_speed_port(LAN_PORT_3, &port_link, &port_speed, &port_duplex);
		port_speed |= (port_duplex << 8);
		port_speed |= (port_link << 16);
		if (retVal == RT_ERR_OK)
			put_user(port_speed, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_SPEED_PORT_LAN4:
		retVal = asic_status_speed_port(LAN_PORT_4, &port_link, &port_speed, &port_duplex);
		port_speed |= (port_duplex << 8);
		port_speed |= (port_link << 16);
		if (retVal == RT_ERR_OK)
			put_user(port_speed, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;

	case RTL8367_IOCTL_STATUS_CNT_PORT_WAN:
		retVal = rtk_stat_port_getAll(WAN_PORT_X, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_CNT_PORT_LAN1:
		retVal = rtk_stat_port_getAll(LAN_PORT_1, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_CNT_PORT_LAN2:
		retVal = rtk_stat_port_getAll(LAN_PORT_2, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_CNT_PORT_LAN3:
		retVal = rtk_stat_port_getAll(LAN_PORT_3, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_CNT_PORT_LAN4:
		retVal = rtk_stat_port_getAll(LAN_PORT_4, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_CNT_PORT_CPU_WAN:
		retVal = rtk_stat_port_getAll(WAN_PORT_CPU, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
#if defined(EXT_PORT_INIC)
	case RTL8367_IOCTL_STATUS_CNT_PORT_INIC:
		retVal = rtk_stat_port_getAll(EXT_PORT_INIC, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
#endif
	case RTL8367_IOCTL_STATUS_CNT_PORT_CPU_LAN:
		retVal = rtk_stat_port_getAll(LAN_PORT_CPU, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367_IOCTL_STATUS_CNT_RESET_ALL:
#if defined(EXT_PORT_INIC)
		rtk_stat_port_reset(EXT_PORT_INIC);
#endif
#if !defined(RTL8367_SINGLE_EXTIF)
		rtk_stat_port_reset(WAN_PORT_CPU);
#endif
		rtk_stat_port_reset(LAN_PORT_CPU);
		rtk_stat_port_reset(WAN_PORT_X);
		rtk_stat_port_reset(LAN_PORT_1);
		rtk_stat_port_reset(LAN_PORT_2);
		rtk_stat_port_reset(LAN_PORT_3);
		rtk_stat_port_reset(LAN_PORT_4);
		break;

	case RTL8367_IOCTL_RESET_ASIC:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		if (uint_value == SWAPI_MAGIC_RESET_ASIC)
			reset_and_init_switch(0);
		break;
	case RTL8367_IOCTL_PORTS_POWER:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_ports_power(uint_param, uint_value);
		break;
	case RTL8367_IOCTL_PORTS_WAN_LAN_POWER:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_wan_lan_ports_power(uint_param, uint_value);
		break;

	case RTL8367_IOCTL_BRIDGE_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_bridge_mode(uint_param, uint_value);
		break;
#if defined(EXT_PORT_INIC)
	case RTL8367_IOCTL_ISOLATE_INIC:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		toggle_isolation_inic(uint_value);
		break;
	case RTL8367_IOCTL_DISABLE_INIC:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		toggle_disable_inic(uint_value);
		break;
#endif
	case RTL8367_IOCTL_PORT_FORWARD_MASK:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = asic_port_forward_mask(uint_param, uint_value);
		break;

	case RTL8367_IOCTL_VLAN_RESET_TABLE:
		asic_vlan_reset_table();
		break;
	case RTL8367_IOCTL_VLAN_INGRESS_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_vlan_ingress_mode_enabled(uint_value);
		break;
	case RTL8367_IOCTL_VLAN_ACCEPT_PORT_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_vlan_accept_port_mode(uint_param, uint_value);
		break;
	case RTL8367_IOCTL_VLAN_CREATE_PORT_VID:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_vlan_create_port_vid(uint_param, uint_value);
		break;
	case RTL8367_IOCTL_VLAN_CREATE_ENTRY:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_vlan_create_entry(uint_param, uint_value);
		break;
	case RTL8367_IOCTL_VLAN_RULE_SET:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_vlan_rule(uint_param, uint_value);
		break;

	case RTL8367_IOCTL_STORM_UNICAST_UNK:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_storm_control_unicast_unknown(uint_value);
		break;
	case RTL8367_IOCTL_STORM_MULTICAST_UNK:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_storm_control_multicast_unknown(uint_value);
		break;
	case RTL8367_IOCTL_STORM_MULTICAST:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_storm_control_multicast(uint_value);
		break;
	case RTL8367_IOCTL_STORM_BROADCAST:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_storm_control_broadcast(uint_value);
		break;

	case RTL8367_IOCTL_JUMBO_FRAMES:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_jumbo_frames_accept(uint_value);
		break;

	case RTL8367_IOCTL_GREEN_ETHERNET:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_green_ethernet_mode(uint_value);
		break;

#if defined(CONFIG_RTL8367_IGMP_SNOOPING)
	case RTL8367_IOCTL_IGMP_SNOOPING:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_igmp_snooping_control(uint_value);
		break;
	case RTL8367_IOCTL_IGMP_RESET:
		reset_igmp_snooping_table();
		break;
#endif

	case RTL8367_IOCTL_LED_MODE_GROUP0:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_led_mode_group0(uint_value);
		break;
	case RTL8367_IOCTL_LED_MODE_GROUP1:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_led_mode_group1(uint_value);
		break;
	case RTL8367_IOCTL_LED_MODE_GROUP2:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_led_mode_group2(uint_value);
		break;

	case RTL8367_IOCTL_SPEED_PORT_WAN:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(WAN_PORT_X, uint_value);
		break;
	case RTL8367_IOCTL_SPEED_PORT_LAN1:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(LAN_PORT_1, uint_value);
		break;
	case RTL8367_IOCTL_SPEED_PORT_LAN2:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(LAN_PORT_2, uint_value);
		break;
	case RTL8367_IOCTL_SPEED_PORT_LAN3:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(LAN_PORT_3, uint_value);
		break;
	case RTL8367_IOCTL_SPEED_PORT_LAN4:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(LAN_PORT_4, uint_value);
		break;

	case RTL8367_IOCTL_RGMII_DELAY_RX:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_cpu_rgmii_delay_rx(uint_value);
		break;

	case RTL8367_IOCTL_RGMII_DELAY_TX:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_cpu_rgmii_delay_tx(uint_value);
		break;

#if defined(CONFIG_RTL8367_IGMP_SNOOPING)
	case RTL8367_IOCTL_MCAST_LUT_DUMP:
		dump_mcast_table();
		break;
#endif
	case RTL8367_IOCTL_ISOLATION_DUMP:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_dump_isolation(uint_value);
		break;

	default:
		ioctl_result = -ENOIOCTLCMD;
	}

	mutex_unlock(&asic_access_mutex);

	return ioctl_result;
}

////////////////////////////////////////////////////////////////////////////////////

static int rtl8367_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////

static int rtl8367_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////

static const struct file_operations rtl8367_fops =
{
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= rtl8367_ioctl,
	.open		= rtl8367_open,
	.release	= rtl8367_release,
};

