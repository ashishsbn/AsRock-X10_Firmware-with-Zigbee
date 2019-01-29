/**
 * Copyright (C) 2012-2013 Steven Barth <steven@midlink.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#pragma once
#include <stdbool.h>
#include <syslog.h>

#include "libubox/blobmsg.h"

#ifndef typeof
#define typeof __typeof
#endif

#ifndef container_of
#define container_of(ptr, type, member) (           \
    (type *)( (char *)ptr - offsetof(type,member) ))
#endif

#include "libubox/list.h"
#include "libubox/uloop.h"


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define _unused __attribute__((unused))
#define _packed __attribute__((packed))

#define ORBWEB_VERSION "v1.4"
#define	MAX_DEVICE_LIST	21
#define	MAX_SCENES_LIST	48
#define	ORBWEB_MQTT_TIMER	1000
#define	ORBWEB_MQTT_RECONNECTING_TIMER	100

#define ATTRID_POWER_BATTERYVOLTAGE 0x0020
#define ATTRID_ON_OFF 0x0000
#define ATTRID_LEVEL_CURRENT_LEVEL 0x0000
#define ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_X 0x0003
#define ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_Y 0x0004
#define ATTRID_LIGHTING_COLOR_CONTROL_COLOR_TEMPERATURE 0x0007

//#define ZLL_RESET_COUNT 12
#define ZLL_RESET_COUNT 5

#define MAX_MQTT_PAYLOAD 400
#define MAX_LENGTH_NAME 34
#define MAX_LENGTH_IEEE 20

/*********************************************************************
 * TYPEDEFS
 */
struct asrock_interface {
	int prefix_length;
};

struct config {
	bool enable;
	char *device_key;
	char *asrock_ssid;
	char *username;
	char *password;
} config;

struct ir {
	struct list_head head;
	bool enable;
	int channel_id;
	char name[34];
	char value[];
};

typedef enum _ZBDeviceTypeT {
	ZB_DEVICE_UNKNOWN = 0x0000,
	ZB_DEVICE_POWER_CONFIG = 0x0001,
	ZB_DEVICE_GROUPS = 0x0002,
	ZB_DEVICE_SCENES = 0x0004,
	ZB_DEVICE_ONOFF = 0x0008,
	ZB_DEVICE_LEVEL = 0x0010,
	ZB_DEVICE_COLOR = 0x0020,
	ZB_DEVICE_TEMP_MESU = 0x0040,
	ZB_DEVICE_HUMI_MESU = 0x0080,
	ZB_DEVICE_ACTIVE = 0x0100
} ZBDeviceTypeT;

/* from zstack */
typedef enum _GwStatusT {
	GW_STATUS_T__STATUS_SUCCESS = 0,
	GW_STATUS_T__STATUS_FAILURE = 1,
	GW_STATUS_T__STATUS_BUSY = 2,
	GW_STATUS_T__STATUS_INVALID_PARAMETER = 3,
	GW_STATUS_T__STATUS_TIMEOUT = 4
} GwStatusT;

typedef enum _ZstackReplyT {
	ZSTACK_REPLY_ZLL_RESET = 0,
	ZSTACK_REPLY_ZLL_CMD,
	ZSTACK_REPLY_PERMIT_JOIN,
	ZSTACK_REPLY_GROUP,
	ZSTACK_REPLY_ADD_SCENE,
	ZSTACK_REPLY_SCENES,
	ZSTACK_REPLY_OTHER
} ZstackReplyT;

typedef enum _ZstackGroupEventT {
	ZSTACK_GROUP_NONE = 0,
	ZSTACK_GROUP_ADD,
	ZSTACK_GROUP_REMOVE,
	ZSTACK_GROUP_REMOVE_ALL
} ZstackGroupEventT;

typedef enum _ZstackSceneEventT {
	ZSTACK_SCENE_NONE = 0,
	ZSTACK_SCENE_REMOVE,
	ZSTACK_SCENE_REMOVE_ALL,
	ZSTACK_SCENE_STORE,
	ZSTACK_SCENE_RECALL
} ZstackSceneEventT;

typedef enum _ZstackClusterT {
	ZSTACK_CLUSTER_POWER = 0x0001,
	ZSTACK_CLUSTER_ONOFF = 0x0006,
	ZSTACK_CLUSTER_LEVEL = 0x0008,
	ZSTACK_CLUSTER_COLOR = 0x0300,
	ZSTACK_CLUSTER_TEMP = 0x0402,
	ZSTACK_CLUSTER_HUMI = 0x0405,
	ZSTACK_CLUSTER_IAS = 0x0502
} ZstackClusterT;

typedef enum _ZstackCmdOnOffT {
	ZSTACK_CMD_ONOFF_OFF = 0x00,
	ZSTACK_CMD_ONOFF_ON = 0x01,
	ZSTACK_CMD_ONOFF_TOGGLE = 0x02
} ZstackCmdOnOffT;

typedef enum _ZstackCmdLevelT {
	ZSTACK_CMD_LEVEL_MOVE_TO_LEVEL = 0x00,
	ZSTACK_CMD_LEVEL_MOVE = 0x01,
	ZSTACK_CMD_LEVEL_STEP = 0x02,
	ZSTACK_CMD_LEVEL_STOP = 0x03,
	ZSTACK_CMD_LEVEL_MOVE_TO_LEVEL_WITH_ONOFF = 0x04,
	ZSTACK_CMD_LEVEL_MOVE_WITH_ONOFF = 0x05,
	ZSTACK_CMD_LEVEL_STEP_WITH_ONOFF = 0x06,
	ZSTACK_CMD_LEVEL_STOP_WITH_ONOFF = 0x07
} ZstackCmdLevelT;

typedef enum _ZstackCmdColorT {
	ZSTACK_CMD_COLOR_MOVE_TO_COLOR = 0x07,
	ZSTACK_CMD_COLOR_MOVE_COLOR = 0x08,
	ZSTACK_CMD_COLOR_STEP_COLOR = 0x09,
	ZSTACK_CMD_COLOR_MOVE_TO_COLOR_TEMPERATURE = 0x0A
} ZstackCmdColorT;

typedef enum _ZstackCmdIAST {
	ZSTACK_CMD_IAS_START_WARNING = 0x00,
	ZSTACK_CMD_IAS_SQUAWK = 0x01
} ZstackCmdIAST;

typedef enum _ZstackZllCmdT {
	ZSTACK_CMD_ZLL_SCAN_REQ = 0x00,
	ZSTACK_CMD_ZLL_INDENTIFY_REQ = 0x01,
	ZSTACK_CMD_ZLL_RESET_FN_REQ = 0x02,
	ZSTACK_CMD_ZLL_RETURN_CHANNEL = 0x03
} ZstackZllCmdT;

typedef enum _ZstackStatusReplyT {
	ZSTACK_STATUS_REPLY_SUCCESS = 0,
	ZSTACK_STATUS_REPLY_FAIL,
	ZSTACK_STATUS_REPLY_BUSY,
	ZSTACK_STATUS_REPLY_TIMEOUT,
	ZSTACK_STATUS_REPLY_NOT_SUPPORT,
	ZSTACK_STATUS_REPLY_NO_DEVICE,
	ZSTACK_STATUS_REPLY_PROCESSING
} ZstackStatusReplyT;

typedef enum _ZstackSceneStatusT {
	ZSTACK_SCENE_STATUS_INIT = 0x00,
	ZSTACK_SCENE_STATUS_GROUP_SUCCESS = 0x01,
	ZSTACK_SCENE_STATUS_GROUP_FAIL = 0x02,
	ZSTACK_SCENE_STATUS_GROUP_BUSY = 0x03,
	ZSTACK_SCENE_STATUS_GROUP_TIMEOUT = 0x04,
	ZSTACK_SCENE_STATUS_GROUP_NOT_SUPPORT = 0x05,
	ZSTACK_SCENE_STATUS_GROUP_NO_DEVICE = 0x06,
	ZSTACK_SCENE_STATUS_SCENE_SUCCESS = 0x10,
	ZSTACK_SCENE_STATUS_SCENE_FAIL = 0x20,
	ZSTACK_SCENE_STATUS_SCENE_BUSY = 0x30,
	ZSTACK_SCENE_STATUS_SCENE_TIMEOUT = 0x40,
	ZSTACK_SCENE_STATUS_SCENE_NOT_SUPPORT = 0x50,
	ZSTACK_SCENE_STATUS_SCENE_NO_DEVICE = 0x60
} ZstackSceneStatusT;

typedef enum
{
	BINDING_MODE_BIND = 0,
	BINDING_MODE_UNBIND
} binding_mode_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern int queue_full;
extern int semaphore_value;
extern struct list_head irs;

/*********************************************************************
 * FUNCTIONS
 */

//#ifdef WITH_UBUS
int init_ubus(void);
int exit_ubus(void);

/* config.c */
int asrock_mqttd_reload(void);
void asrock_mqttd_run(void);
void asrock_main(void);

/* functions for calling zstack ubus */
int ubus_call_zstack_permit_join(bool enable);
int ubus_call_zstack_remove(char *device_addr);
int ubus_call_zstack_read_attribute(char *device_addr, int cluster, int attribute);
int ubus_call_zstack_execute_command(char *device_addr, ZstackClusterT cluster, int command, char *value);
int ubus_call_zstack_zll_reset(bool enable, int count, uint32_t command);
int ubus_call_zstack_rename(char *device_addr, char *name);
int ubus_call_zstack_group(ZstackGroupEventT action, char *device_addr, int group_id, int endpoint_id);
int ubus_call_zstack_add_scene(char *device_addr, int endpoint_id, int group_id, int scene_id, int device_type, int onoff, int level, int color);
int ubus_call_zstack_scenes(ZstackSceneEventT action, int group_id, int scene_id);

/* functions for handling scenes*/
int handle_cloud_add_scene_new(struct blob_attr *msg);
int handle_cloud_remove_all_scenes(void);

/* functions for accessing database */
int open_database(void);
int close_database(void);
int sql_new_device(char *IEEEAddress, char *endpoint, char *clusters, int device_type);
int sql_insert_device(char *IEEEAddress, char *name, char *manufacture, char *model, int new, char *endpoint, char *clusters, int device_type);
int sql_update_device(char *IEEEAddress, char *name, char *manufacture, char *model, char *endpoint, char *clusters, int device_type);
int sql_update_basic_device(char *IEEEAddress, char *name, char *manufacture, char *model);
int sql_update_new_status(char *IEEEAddress, int new);
int sql_update_device_type(char *IEEEAddress, int device_type);
int sql_update_device_name(char *IEEEAddress, char *name);
int sql_delete_device(char *IEEEAddress);
int sql_retrieve_device(char *IEEEAddress);
int sql_retrieve_device_type(char *IEEEAddress);
int sql_retrieve_dump_devices(char **ieeeaddr, char **name, char **manufacture, char **model);
int sql_retrieve_new_device(char *IEEEAddress, char *name, char *manufacture, char *model, char *clusters);
int sql_retrieve_all_devices(char **ieeeaddr, char **manufacture, char **model, char **name);
int sql_retrieve_all_devices_with_status(char **ieeeaddr);
char *sql_retrieve_device_clusters(char *IEEEAddress);
char *sql_retrieve_device_endpoint(char *IEEEAddress);
char *sql_retrieve_device_manufacture_name(char *IEEEAddress);
char *sql_retrieve_device_model_name(char *IEEEAddress);
char *sql_retrieve_device_device_name(char *IEEEAddress);
int sql_retrieve_device_status(char *IEEEAddress);
int sql_retrieve_next_device(char *ieeeaddr, char *return_ieeeAddr, char *return_endpoint);
int sql_check_gropu_id(int group_id);
int sql_new_group(int group_id, char *name, int total, int id);
int sql_retrieve_all_groups(int *group_id, char **name, int *id);
int sql_retrieve_gropu_id_from_cloud(int group_id);
int sql_retrieve_scene_name(int group_id, char *scene_name);
int sql_retrieve_next_group_id(int group_id);
int sql_create_scene_table(int group_id);
int sql_new_scene_device(int group_id, char *IEEEAddress, char *onoff, char *level, char *color, int device_type);
int sql_retrieve_n_group_devices(int group_id);
int sql_retrieve_next_group_device(char *ieeeaddr, int group_id, char *return_ieeeAddr);
int sql_retrieve_group_device_type(char *ieeeaddr, int group_id);
int sql_retrieve_group_onoff(char *ieeeaddr, int group_id, char *onoff);
int sql_retrieve_group_level(char *ieeeaddr, int group_id, char *level);
int sql_retrieve_group_color(char *ieeeaddr, int group_id, char *color);
int sql_retrieve_group_all_devices(int group_id, char **ieeeaddr, char **onoff, char **level, char **color, int *type);
int sql_delete_group_scene(int group_id);
//#endif
