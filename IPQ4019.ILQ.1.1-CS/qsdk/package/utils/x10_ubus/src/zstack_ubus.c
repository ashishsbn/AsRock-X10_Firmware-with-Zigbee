 /*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <libubus.h>
#include <libubox/uloop.h>
#include <libubox/blobmsg_json.h>
#include <semaphore.h>
#include <sqlite3.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <curl/curl.h>

static struct ubus_context *ctx;
static struct ubus_subscriber test_event;
static struct blob_buf b;
#define QUEUE_MAX_LIMIT 60
#define MAX_LENGTH_NAME 34
static int queue_full;
static struct ubus_context *ubus = NULL;
static struct ubus_subscriber zstack;
static uint32_t objid_zstack = 0;
static struct blob_buf b;

int reset_count = 0;
bool zll_reset_get_device_rsp = false;
bool zll_procedure_done = false;
bool zll_send_twice = false;
char scan_resp_device_id[20] = "FFFFFFFFFFFFFFFF";

sem_t semO;
char db_path[256] = "/etc/orbweb.db";
char sql_endpoint_id[32];
char sql_manufacture[32];
char sql_cluster[32];
char sql_model[32];
static int sql_device_status;
static int trigger_scene;

#define MY_ENCODING "UTF-8"
#define POLLING_URL "app1.gizmosmart.io:8902/iot/tst_srvr/1.3/public/poll"


int semaphore_value = 0;

typedef enum _ZstackClusterT {
	ZSTACK_CLUSTER_POWER = 0x0001,
	ZSTACK_CLUSTER_ONOFF = 0x0006,
	ZSTACK_CLUSTER_LEVEL = 0x0008,
	ZSTACK_CLUSTER_COLOR = 0x0300,
	ZSTACK_CLUSTER_TEMP = 0x0402,
	ZSTACK_CLUSTER_HUMI = 0x0405,
	ZSTACK_CLUSTER_IAS = 0x0502
} ZstackClusterT;

typedef enum
{
	BINDING_MODE_BIND = 0,
	BINDING_MODE_UNBIND
} binding_mode_t;

typedef enum _ZstackStatusReplyT {
	ZSTACK_STATUS_REPLY_SUCCESS = 0,
	ZSTACK_STATUS_REPLY_FAIL,
	ZSTACK_STATUS_REPLY_BUSY,
	ZSTACK_STATUS_REPLY_TIMEOUT,
	ZSTACK_STATUS_REPLY_NOT_SUPPORT,
	ZSTACK_STATUS_REPLY_NO_DEVICE,
	ZSTACK_STATUS_REPLY_PROCESSING
} ZstackStatusReplyT;

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


struct config
{
	char mac[32];
	char ip[32];
	char netmask[32];
	char gateway_ip[32];
}cfg;


/*This function is callback of database query execution*/
static int callback(void *data, int argc, char **argv, char **azColName){
	/*
	   We should put this response in logs. 
	   int i;
	   fprintf(stderr, "%s: ", (const char*)data);

	   for(i = 0; i<argc; i++) {
	   printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	   }
	   printf("\n");
	 */
	return 0;
}

static int device_endpoint_callback(void *data, int argc, char **argv, char **azColName){
	strcpy(sql_endpoint_id,argv[0] ? argv[0]:"NULL");
	return 0;
}

static int device_status_callback(void *data, int argc, char **argv, char **azColName){
	char a = argv[0][0];
	sql_device_status = a - '0';
	return 0;
}

static int device_manufacture_callback(void *data, int argc, char **argv, char **azColName){
	strcpy(sql_manufacture, argv[0] ? argv[0]:"NULL");
	return 0;
}

static int device_cluster_callback(void *data, int argc, char **argv, char **azColName){
	strcpy(sql_cluster, argv[0] ? argv[0]:"NULL");
	return 0;
}

static int device_model_callback(void *data, int argc, char **argv, char **azColName){
	strcpy(sql_model, argv[0] ? argv[0]:"NULL");
	return 0;
}


static int IFTTT_check_callback (void *data, int argc, char **argv, char **azColName){
	trigger_scene = strtol(argv[0],NULL,10);
	return 0;
}


char* sql_retrieve_device_endpoint(char *device_id){
	char sql[256];
	int rc;
	sqlite3 *db;
	char *zErrMsg = 0;

	rc = sqlite3_open(db_path, &db);

	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return "NULL";
	} else {
		printf("Opened database successfully retriving Endpoint ID\n");
	}

	sprintf(sql,"select ENDPOINT from DEVICES where IEEEAddress='%s'; ",device_id);
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, device_endpoint_callback, 0, &zErrMsg);
	printf("ENDPOINT is :%s \n",sql_endpoint_id);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return "NULL";
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return sql_endpoint_id;
}

int sql_retrieve_device_status(char *device_id){
	char sql[256];
	int rc;
	sqlite3 *db;
	char *zErrMsg = 0;

	rc = sqlite3_open(db_path, &db);

	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return -1;
	} else {
		printf("Opened database successfully, retriving device status\n");
	}

	sprintf(sql,"select NEW from DEVICES where IEEEAddress='%s'; ",device_id);
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, device_status_callback, 0, &zErrMsg);
	printf("STATUS is :%d \n",sql_device_status);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return sql_device_status;
}

int sql_delete_device(char *device_id){
	char sql[256];
	int rc;
	sqlite3 *db;
	char *zErrMsg = 0;

	rc = sqlite3_open(db_path, &db);

	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return -1;
	} else {
		printf("Opened database successfully, deleting device.\n");
	}

	sprintf(sql,"DELETE from DEVICES where IEEEAddress='%s'; ",device_id);
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return 0;
}


int sql_new_device(char *IEEEAddress, char *endpoint,char *clusters,int device_type)
{
	char sql[256];
	int rc;
	sqlite3 *db;
	char *zErrMsg = 0;

	rc = sqlite3_open(db_path, &db);

	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return -1;
	} else {
		printf("Opened database successfully adding new device in DB\n");
	}

	sprintf(sql,"INSERT into DEVICES (IEEEAddress,ENDPOINT,CLUSTERS,TYPE,NEW) VALUES ('%s','%s','%s',%d,1); ",IEEEAddress,endpoint,clusters,device_type);
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return 0;
}


int sql_update_basic_device(char *IEEEAddress,char *name,char *manufacture,char *model){
	char sql[256];
	int rc;
	sqlite3 *db;
	char *zErrMsg = 0;

	rc = sqlite3_open(db_path, &db);

	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return -1;
	} else {
		printf("Opened database successfully, updating basic atrributes\n");
	}

	sprintf(sql,"UPDATE DEVICES set NAME = '%s',MANUFACTURE = '%s',MODEL = '%s' where IEEEAddress = '%s' ",name , manufacture,model,IEEEAddress);
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return 0;

}
char * sql_retrieve_device_manufacture_name(char *IEEEAddress){
	char sql[256];
	int rc;
	sqlite3 *db;
	char *zErrMsg = 0;

	rc = sqlite3_open(db_path, &db);

	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return "NULL";
	} else {
		printf("Opened database successfully, retriving manufacturer details\n");
	}

	sprintf(sql,"select MANUFACTURE from DEVICES where IEEEAddress='%s'; ",IEEEAddress);
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, device_manufacture_callback, 0, &zErrMsg);
	printf("MANUFACTURE is :%s \n",sql_manufacture);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return "NULL";
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return sql_manufacture;

}

char * sql_retrieve_device_clusters(char *IEEEAddress){
	char sql[256];
	int rc;
	sqlite3 *db;
	char *zErrMsg = 0;

	rc = sqlite3_open(db_path, &db);

	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return "NULL";
	} else {
		printf("Opened database successfully, retriving cluster details\n");
	}

	sprintf(sql,"select CLUSTERS from DEVICES where IEEEAddress='%s'; ",IEEEAddress);
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, device_cluster_callback, 0, &zErrMsg);
	printf("CLUSTER is :%s \n",sql_cluster);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return "NULL";
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return sql_cluster;

}

char * sql_retrieve_device_model(char *IEEEAddress){
	char sql[256];
	int rc;
	sqlite3 *db;
	char *zErrMsg = 0;

	rc = sqlite3_open(db_path, &db);

	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return "NULL";
	} else {
		printf("Opened database successfully, retriving model details\n");
	}

	sprintf(sql,"select MODEL from DEVICES where IEEEAddress='%s'; ",IEEEAddress);
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, device_model_callback, 0, &zErrMsg);
	printf("MODEL is :%s \n",sql_model);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return "NULL";
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return sql_model;

}

int sql_update_new_status(char *IEEEAddress, int status)
{
	char sql[256];
	int rc;
	sqlite3 *db;
	char *zErrMsg = 0;

	rc = sqlite3_open(db_path, &db);

	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return -1;
	} else {
		printf("Opened database successfully, updating device status\n");
	}

	sprintf(sql,"UPDATE DEVICES set NEW = '%d' where IEEEAddress = '%s' ",status,IEEEAddress);
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return 0;


}


int sql_check_IFTTT_rule(char *IEEEAddress)
{
	char sql[256];
	int rc;
	sqlite3 *db;
	char *zErrMsg = 0;
	trigger_scene = -1;
	rc = sqlite3_open(db_path, &db);

	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return -1;
	} else {
		printf("Opened database successfully, Looking for IFTTT rule\n");
	}

	sprintf(sql,"select exec from IFTTT where RULE_DEVICE_ID = '%s'",IEEEAddress);
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, IFTTT_check_callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return trigger_scene;


}

void create_table_if_doesnot_exist()
{

	char sql[256];
	int rc;
	sqlite3 *db;
	char *zErrMsg = 0;

	rc = sqlite3_open(db_path, &db);

	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return ;
	} else {
		printf("Opened database successfully, creating tables if doesn't exist\n");
	}

	sprintf(sql,"create table if not exists HUB (MODE INTEGER, ID INT,UNIQUE(ID))");
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return ;
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}

	sprintf(sql,"create table if not exists IFTTT (HA_NO INTEGER, RULE_DEVICE_ID TEXT, VALUE INTEGER , EXEC INTEGER, ACTION TEXT)");
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return ;
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}

	sprintf(sql,"CREATE TABLE GROUPS (GROUP_ID INTEGER, DEVICE_LIST TEXT)");
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return ;
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}

	sprintf(sql,"create table if not exists DEVICES (IEEEAddress TEXT PRIMARY KEY NOT NULL,NAME TEXT,MANUFACTURE TEXT,MODEL	TEXT,NEW INTEGER,ENDPOINT TEXT,CLUSTERS	TEXT,TYPE INTEGER )");
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return;
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}

	sprintf(sql,"insert or ignore into hub (MODE,ID) values (0,1)");
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return;
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}

	sqlite3_close(db);
	return ;

}


size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	return size * nmemb;
}

void sendStatusUpdate(char *post_data){

	CURLcode res;
	CURL *curl = curl_easy_init();
	if(curl) {
		printf("Polling Command\n");
		curl_easy_setopt(curl, CURLOPT_URL, POLLING_URL);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

}

void checkIFTTT(char *device_id){
	int scene_id;
	char command[128];
	char send_notification[1024];
	scene_id = sql_check_IFTTT_rule(device_id);
	printf("Calling scene :%d\n",scene_id);
	if(scene_id >= 0){
		sprintf(command,"ubus call zstack scenes '{\"action\":4,\"group_id\":%d,\"scene_id\":%d}'",scene_id,scene_id);
		system(command);
		sprintf(send_notification,"/etc/gizmo/scene_triggered %d",scene_id);
		system(send_notification);

	}
}

void send_device_status(char *device_id, char *value){
	int rc;
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	xmlChar *tmp;
	char model[32];
	int device_type;
	char status_type[16];

	int int_value = (int)strtoul(value, NULL, 0);
	//    if(value != NULL){
	sprintf(model, "%s", sql_retrieve_device_model(device_id));
	if(strncmp("DC",model,2)==0){
		device_type = 4;
	}else if(strncmp("PSMP",model,4)==0){
		device_type = 48;	
	}else if(strncmp("PSMD",model,4)==0){
		device_type = 53;
	}else if(strncmp("PSSP",model,4)==0){
		device_type = 24;
	}else if(strncmp("WS",model,2)==0){
		device_type = 5;
	}else if(strncmp("SV",model,2)==0){
		device_type = 39;
	}else if(strncmp("SD",model,2)==0){
		device_type = 11;
	}

	printf("Found device type:%d\n",device_type);
	if(device_type == 4 || device_type == 48 || device_type == 24 || device_type == 53){
		strcpy(status_type,"getDeviceStatus");
	}else if(device_type == 5 || device_type == 39 || device_type == 11){

		if((int_value & 1) != 1 )
			return;
		strcpy(status_type,"getRecord");
	}
	printf("executing command : %s for model : %s\n",status_type,model);
	/* Create a new XML buffer, to which the XML document will be
	 * written */
	buf = xmlBufferCreate();
	if (buf == NULL) {
		printf("testXmlwriterMemory: Error creating the xml buffer\n");
		return;
	}

	/* Create a new XmlWriter for memory, with no compression.
	 * Remark: there is no compression for this kind of xmlTextWriter */
	writer = xmlNewTextWriterMemory(buf, 0);
	if (writer == NULL) {
		printf("testXmlwriterMemory: Error creating the xml writer\n");
		return;
	}

	/* Start the document with the xml default for the version,
	 * encoding ISO 8859-1 and the default for the standalone
	 * declaration. */
	rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartDocument\n");
		return;
	}

	rc = xmlTextWriterStartElement(writer, BAD_CAST "p");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
		return;
	}

	/* Add an attribute with name "mac" to root "p" */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "mac");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
		return;
	}

	/* Add an attribute with name "v" (value of MAC address)  */
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
			BAD_CAST cfg.mac);
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
		return;
	}

	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
		return;
	}

	/* Start an element named "HEADER" as child of ORDER. */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "cmds");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
		return;
	}

	rc = xmlTextWriterStartElement(writer, BAD_CAST "referer");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
		return;
	}
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
			BAD_CAST "panel/notify");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
		return;
	}

	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
		return;
	}

	rc = xmlTextWriterStartElement(writer, BAD_CAST "cmd");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
		return;
	}
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "a",
			BAD_CAST status_type);
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
		return;
	}
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "id",
			BAD_CAST "813824618762384");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
		return;
	}

	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "ret",
			"%d", 1);
	if (rc < 0) {
		printf
			("testXmlwriterTree: Error at xmlTextWriterWriteFormatElement\n");
		return;
	}
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "m",
			"%s", "OK");
	if (rc < 0) {
		printf
			("testXmlwriterTree: Error at xmlTextWriterWriteFormatElement\n");
		return;
	}

	rc = xmlTextWriterStartElement(writer, BAD_CAST "x");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
		return;
	}



	if(strcmp(status_type,"getDeviceStatus")==0){
		rc = xmlTextWriterStartElement(writer, BAD_CAST "z");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}



		rc = xmlTextWriterStartElement(writer, BAD_CAST "area");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "1");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}


		rc = xmlTextWriterStartElement(writer, BAD_CAST "no");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "4");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}
		

		rc = xmlTextWriterStartElement(writer, BAD_CAST "ty");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "v",
				BAD_CAST "%d",device_type);
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}

		rc = xmlTextWriterStartElement(writer, BAD_CAST "su");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "1");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}


		rc = xmlTextWriterStartElement(writer, BAD_CAST "n");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST model);
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}


		rc = xmlTextWriterStartElement(writer, BAD_CAST "id");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST device_id);
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}


		rc = xmlTextWriterStartElement(writer, BAD_CAST "id2");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}

		rc = xmlTextWriterStartElement(writer, BAD_CAST "ver");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST model);
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}

		char buff[32];
		time_t now = time(NULL);
		strftime(buff, 32, "%Y-%m-%d %H:%M:%S", localtime(&now));

		rc = xmlTextWriterStartElement(writer, BAD_CAST "status_time");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST buff);
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}

		rc = xmlTextWriterStartElement(writer, BAD_CAST "status_rssi");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "9");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}

		rc = xmlTextWriterStartElement(writer, BAD_CAST "group");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "0x0");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}

		if(device_type == 4){
			char door_status[2];
			printf("checking door status\n");
			if(strcmp(value,"0x0015")==0 || strcmp(value,"0x0011")==0 || (int_value & 1) == 1){
				sprintf(door_status,"1");
				checkIFTTT(device_id);
			}else{
				sprintf(door_status,"0");
			}
			printf("Got door status: %s\n",door_status);


			rc = xmlTextWriterStartElement(writer, BAD_CAST "status_open");
			if (rc < 0) {
				printf
					("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
				return;
			}

			rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
					BAD_CAST door_status);
			if (rc < 0) {
				printf
					("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
				return;
			}

			rc = xmlTextWriterEndElement(writer);
			if (rc < 0) {
				printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
				return;
			}

		}else if(device_type == 48 || device_type == 24){
			
			char switch_status[2];
			printf("checking switch status\n");
			if(strcmp(value,"0x01")==0){
				sprintf(switch_status,"1");
			}else{
				sprintf(switch_status,"0");
			}
			printf("Got switch status: %s\n",switch_status);


			rc = xmlTextWriterStartElement(writer, BAD_CAST "status_switch");
			if (rc < 0) {
				printf
					("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
				return;
			}

			rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
					BAD_CAST switch_status);
			if (rc < 0) {
				printf
					("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
				return;
			}

			rc = xmlTextWriterEndElement(writer);
			if (rc < 0) {
				printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
				return;
			}


		}
		char temper_status[4];

		if((int_value & 4) == 4 ){
			sprintf(temper_status,"0x02");
		}else{
			sprintf(temper_status,"0x00");
		}


		rc = xmlTextWriterStartElement(writer, BAD_CAST "st");
		if (rc < 0) {
			printf      
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}


		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST temper_status);
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}



		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}
	}else if(strcmp(status_type,"getRecord")==0){

		checkIFTTT(device_id);
		rc = xmlTextWriterStartElement(writer, BAD_CAST "log");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}



		rc = xmlTextWriterStartElement(writer, BAD_CAST "record_id");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "1");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}

		char buff[32];
		time_t now = time(NULL);
		strftime(buff, 32, "%Y-%m-%d %H:%M:%S", localtime(&now));

		rc = xmlTextWriterStartElement(writer, BAD_CAST "log_time");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST buff);
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}

		rc = xmlTextWriterStartElement(writer, BAD_CAST "id");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST device_id);
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}

		rc = xmlTextWriterStartElement(writer, BAD_CAST "area");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "1");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}
		rc = xmlTextWriterStartElement(writer, BAD_CAST "zone");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "4");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}

		rc = xmlTextWriterStartElement(writer, BAD_CAST "user");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "1");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}

		rc = xmlTextWriterStartElement(writer, BAD_CAST "event");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "1");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}

		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) {
			printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
			return;
		}


	}
	rc = xmlTextWriterStartElement(writer, BAD_CAST "size");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
		return;
	}

	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
			BAD_CAST "1");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
		return;
	}

	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
		return;
	}

	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
		return;
	}

	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
		return;
	}

	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
		return;
	}

	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
		return;
	}
	xmlFreeTextWriter(writer);
	printf("Completed xml construction\n");
	printf("%s\n",buf->content);

	char *post_data;
	post_data = malloc(4096);
	post_data = memset(post_data,0,4096);
	CURL *curl = curl_easy_init();
	if(curl) {
		char *output = curl_easy_escape(curl, buf->content, 0);
		if(output) {
			sprintf(post_data,"strXML=%s",output);
			curl_free(output);
		}
	}
	sendStatusUpdate(post_data);
	curl_global_cleanup();
	xmlBufferFree(buf);
}


/*
   static struct ubus_method main_object_methods[] = {
   UBUS_METHOD("zb_table", handle_zb_table, zb_table_policy),
   UBUS_METHOD("zb_database", handle_zb_database, zb_database_policy),
   UBUS_METHOD("zb_zll_reset", handle_zb_zll_send_reset, zb_zll_send_reset_policy),
   };

   static struct ubus_object_type main_object_type =
   UBUS_OBJECT_TYPE("orbweb", main_object_methods);

   static struct ubus_object main_object = {
   .name = "orbweb",
   .type = &main_object_type,
   .methods = main_object_methods,
   .n_methods = ARRAY_SIZE(main_object_methods),
   };

 */


/**************************************************************************************************
 * @ obj This is the ubus object for pair event reply
 **************************************************************************************************/
enum {
	ZB_PAIREVENT_TYPE,
	ZB_PAIREVENT_STATUS,
	ZB_PAIREVENT_DEVICEID,
	__ZB_PAIREVENT_MAX
};

static const struct blobmsg_policy zb_pairevent_policy[__ZB_PAIREVENT_MAX] = {
	[ZB_PAIREVENT_TYPE] = { .name = "event_type", .type = BLOBMSG_TYPE_STRING },
	[ZB_PAIREVENT_STATUS] = { .name = "status", .type = BLOBMSG_TYPE_INT32 },
	[ZB_PAIREVENT_DEVICEID] = { .name = "device_id", .type = BLOBMSG_TYPE_STRING },
};

/**************************************************************************************************
 *
 * @fn          handle_zb_pairevent
 *
 * @brief       Handle pairevent(enabled/exited/unpaired) and publish to the cloud
 *
 **************************************************************************************************/
static int handle_zb_pairevent(struct blob_attr *msg) {

#if printf_VERBOSE
	printf("handle_zb_pairevent Start\n");
#endif
	struct blob_attr *tb[__ZB_PAIREVENT_MAX], *c;

	const char *event_type = "none";
	ZstackStatusReplyT zstack_status = ZSTACK_STATUS_REPLY_FAIL;
	char *device_id = "0000000000000000";
	int topic_type = 0; // 1: enabled, 2: exited, 3: unpaired

	blobmsg_parse(zb_pairevent_policy, __ZB_PAIREVENT_MAX, tb, blob_data(msg), blob_len(msg));

	if ((c = tb[ZB_PAIREVENT_TYPE])) {
		event_type = blobmsg_get_string(c);

		if(strcmp(event_type, "enabled")==0) {
			topic_type = 1;
		} else if(strcmp(event_type, "exited")==0) {
			topic_type = 2;
		} else if(strcmp(event_type, "unpaired")==0) {
			topic_type = 3;
		} else if(strcmp(event_type, "pairevent_cnf")==0) {
			topic_type = 0;
		}
#if printf_VERBOSE
		printf("event_type[%d]: %s\n", topic_type, event_type);
#endif
	}
	if ((c = tb[ZB_PAIREVENT_STATUS])) {
		zstack_status = blobmsg_get_u32(c);
#if printf_VERBOSE
		printf("zstack_status %d\n", zstack_status);
#endif
		if(zstack_status==ZSTACK_STATUS_REPLY_PROCESSING) {
			// Actually send into zstack, no need to reply
			goto END;
		}
	}

	blob_buf_init(&b, 0);

	if(topic_type==3) { // unpaired need parse device ID
		if ((c = tb[ZB_PAIREVENT_DEVICEID])) {
			device_id = strdup(blobmsg_get_string(c));
#if printf_VERBOSE
			printf("device_id %s", device_id);
#endif
			char endpoint_text[10];
			int new_device_status = 0;
			memset(endpoint_text, 0, sizeof(endpoint_text));

			sprintf(endpoint_text, "%s", sql_retrieve_device_endpoint(device_id));
			if(strlen(endpoint_text)==0) {
				// Since this is unpaired from zsatck, but the address is not existed in database, ingoring this
#if printf_VERBOSE
				printf("hide this unpaired event of %s from zstack...", device_id);
#endif				
				topic_type = 0;
			}

			new_device_status = sql_retrieve_device_status(device_id);
			if(new_device_status==1) {
#if printf_VERBOSE
				printf("hide this unpaired event of %s that is unfinished joining...", device_id);
#endif				
				topic_type = 0;
			}

			// Delete database
			sql_delete_device(device_id);
		}
	}


	switch(zstack_status) {
		case ZSTACK_STATUS_REPLY_SUCCESS:
			blobmsg_add_string(&b, "status", "success");
			break;
		case ZSTACK_STATUS_REPLY_NO_DEVICE:
			blobmsg_add_string(&b, "status", "no device");
			break;
		default:
			blobmsg_add_string(&b, "status", "failure");
			break;
	}

	printf("zstack_status %d, event_type %s\n", zstack_status, event_type);
	if((zstack_status!=ZSTACK_STATUS_REPLY_NO_DEVICE)&&(strcmp(event_type, "unpaired")!=0) ) {

		// enable permit_join would not release semaphore, since X10 would do this on disabling permit_join 
		if(sem_getvalue(&semO, &semaphore_value) != -1) {
			printf("orbweb sem_getvalue = %d\n", semaphore_value);
		}

		sem_post(&semO);

		if(sem_getvalue(&semO, &semaphore_value) != -1) {
			printf("orbweb sem_getvalue = %d\n", semaphore_value);
		}
	}

	// publish to cloud via MQTT
	if(topic_type!=0) {

		//MQTTMessageParams Msg = MQTTMessageParamsDefault;
		//Msg.qos = QOS_0;
		char cPayload[50];
		memset(cPayload, 0, sizeof(cPayload));
		sprintf(cPayload, "%s", blobmsg_format_json(b.head, true));
		printf("blobmsg json %s\n", cPayload);
		//Msg.pPayload = (void *) cPayload;

		/*
		   MQTTPublishParams Params = MQTTPublishParamsDefault;
		   char cTopic[100];
		   memset(cTopic, 0, sizeof(cTopic));
		   switch(topic_type) {
		   case 1:
		   sprintf(cTopic, "/asrock/x10/%s/zigbee/pairevent/enabled", config.asrock_ssid);
		   break;
		   case 2:
		   sprintf(cTopic, "/asrock/x10/%s/zigbee/pairevent/exited", config.asrock_ssid);
		   break;
		   case 3:
		   sprintf(cTopic, "/asrock/x10/%s/zigbee/pairevent/unpaired/0x%s", config.asrock_ssid, device_id);
		   break;
		   default:
		   return rc;
		   }

		   Params.pTopic = strdup(cTopic);

		   Msg.PayloadLen = strlen(cPayload) + 1;
		   Params.MessageParams = Msg;
		   printf("topic %s", Params.pTopic);
		   rc = aws_iot_mqtt_publish(&Params);
		 */
	}
END:
#if printf_VERBOSE
	printf("handle_zb_pairevent End\n");
#endif
	return 0;
}

enum {
	ZB_DEVICE_ANNOUNCE_ADDR,
	__ZB_DEVICE_ANNOUNCE_MAX
};

static const struct blobmsg_policy zb_device_announce_policy[__ZB_DEVICE_ANNOUNCE_MAX] = {
	[ZB_DEVICE_ANNOUNCE_ADDR] = { .name = "device_addr", .type = BLOBMSG_TYPE_STRING },
};

/**************************************************************************************************
 *
 * @fn          handle_zb_device_announce
 *
 * @brief       Handle device announce and publish to the cloud
 *
 **************************************************************************************************/
static int handle_zb_device_announce(struct blob_attr *msg) {

#if printf_VERBOSE
	printf("handle_zb_device_announce Start\n");
#endif
	struct blob_attr *tb[__ZB_DEVICE_ANNOUNCE_MAX], *c;

	char *device_id = "0000000000000000";
	char endpoint_text[10];
	bool publish_to_cloud = false;

	memset(endpoint_text, 0, sizeof(endpoint_text));

	blobmsg_parse(zb_device_announce_policy, __ZB_DEVICE_ANNOUNCE_MAX, tb, blob_data(msg), blob_len(msg));

	if ((c = tb[ZB_DEVICE_ANNOUNCE_ADDR])) {
		device_id = strdup(blobmsg_get_string(c));
#if printf_VERBOSE
		printf("device_id %s\n", device_id);
#endif
	}
	sprintf(endpoint_text, "%s", sql_retrieve_device_endpoint(device_id));
	if(strlen(endpoint_text)!=0) {
		publish_to_cloud = true;
	}


	if(publish_to_cloud) {

		blob_buf_init(&b, 0);
		blobmsg_add_string(&b, "status", "success");

		// publish to cloud via MQTT	
		char cPayload[50];
		memset(cPayload, 0, sizeof(cPayload));
		sprintf(cPayload, "%s", blobmsg_format_json(b.head, true));
		printf("blobmsg json %s\n", cPayload);
		/*
		   MQTTPublishParams Params = MQTTPublishParamsDefault;
		   char cTopic[100];
		   memset(cTopic, 0, sizeof(cTopic));
		   sprintf(cTopic, "/asrock/x10/%s/zigbee/event/0x%s/zdo/devannounce", config.asrock_ssid, device_id);

		   Params.pTopic = strdup(cTopic);

		   Msg.PayloadLen = strlen(cPayload) + 1;
		   Params.MessageParams = Msg;
		   printf("topic %s", Params.pTopic);
		   rc = aws_iot_mqtt_publish(&Params);
		 */
	} else {
#if printf_VERBOSE
		printf("This device is not in database, so not publishing device announcement MQTT...\n");
#endif
	}

#if printf_VERBOSE
	printf("handle_zb_device_announce End\n");
#endif
	return 0;
}



/**************************************************************************************************
 * @ obj This is the ubus object for zigbee new found device
 **************************************************************************************************/
enum {
	ZB_NEW_ADDR,
	ZB_NEW_ENDPOINT,
	ZB_NEW_CLUSTER,
	ZB_NEW_DEVICE_TYPE,
	__ZB_NEW_MAX
};

int ubus_call_zstack_basic_attribute(char *device_addr, int endpoint_id) {

#if printf_VERBOSE
	printf("ubus_call_zstack_basic_attribute Start\n");
#endif

#if printf_VERBOSE
	printf("device_addr %s, endpoint_id %d\n", device_addr, endpoint_id);
#endif	

	if(sem_getvalue(&semO, &semaphore_value) != -1) {
		printf("orbweb sem_getvalue = %d\n", semaphore_value);
	}

	if (sem_trywait(&semO) != 0) {
		queue_full = 1;
		printf("sem_trywait can't acquire, queue is full\n");
		goto END;
	}
	else {
		queue_full = 0;
	}

	if(sem_getvalue(&semO, &semaphore_value) != -1) {
		printf("orbweb sem_getvalue = %d\n", semaphore_value);
	}


	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "dev_addr", device_addr);
	blobmsg_add_u32(&b, "endpoint_id", endpoint_id);
	ubus_invoke(ubus, objid_zstack, "basic_attr", b.head, NULL, 0, 1000);
END:
#if printf_VERBOSE
	printf("ubus_call_zstack_basic_attribute End\n");
#endif
	return 0;
}




static const struct blobmsg_policy zb_new_policy[__ZB_NEW_MAX] = {
	[ZB_NEW_ADDR] = { .name = "device_addr", .type = BLOBMSG_TYPE_STRING },
	[ZB_NEW_ENDPOINT] = { .name = "endpointid", .type = BLOBMSG_TYPE_STRING },
	[ZB_NEW_CLUSTER] = { .name = "clusters", .type = BLOBMSG_TYPE_STRING },
	[ZB_NEW_DEVICE_TYPE] = { .name = "device_type", .type = BLOBMSG_TYPE_INT32 },
};

/**************************************************************************************************
 *
 * @fn          handle_zb_new
 *
 * @brief       Handle new zigbee device and store into the database
 *
 **************************************************************************************************/
static int handle_zb_new(struct blob_attr *msg) {

#if printf_VERBOSE
	printf("handle_zb_new Start\n");
#endif
	struct blob_attr *tb[__ZB_NEW_MAX], *c;
	char *IEEEAddress = "0000000000000000";
	char *endpoint = "0x0000";
	int endpoint_id_int = 0;
	char *clusters = "";
	int device_type = ZB_DEVICE_UNKNOWN;

	blob_buf_init(&b, 0);
	printf("ZB NEW Package %s\n", blobmsg_format_json(msg, true));
	blobmsg_parse(zb_new_policy, __ZB_NEW_MAX, tb, blob_data(msg), blob_len(msg));

	if ((c = tb[ZB_NEW_ADDR])) {
		IEEEAddress = blobmsg_get_string(c);
#if printf_VERBOSE
		printf("DeviceAddr %s\n", IEEEAddress);
#endif
	}
	if ((c = tb[ZB_NEW_ENDPOINT])) {
		endpoint = blobmsg_get_string(c);
#if printf_VERBOSE
		printf("endpoint %s\n", endpoint);
#endif
	}
	if ((c = tb[ZB_NEW_CLUSTER])) {
		clusters = blobmsg_get_string(c);
#if printf_VERBOSE
		printf("clusters %s\n", clusters);
#endif
	}
	if ((c = tb[ZB_NEW_DEVICE_TYPE])) {
		device_type = blobmsg_get_u32(c);
#if printf_VERBOSE
		printf("device_type %d\n", device_type);
#endif
	}

	// Create New device
	sql_new_device(IEEEAddress, endpoint, clusters, device_type);

	// send basic attribute query to zstack
	endpoint_id_int = (int)strtoul(endpoint, NULL, 0); //atoi(endpoint);
#if printf_VERBOSE
	printf("endpoint_id_int %d\n", endpoint_id_int);
#endif
	ubus_call_zstack_basic_attribute(IEEEAddress, endpoint_id_int);

#if printf_VERBOSE
	printf("handle_zb_new End\n");
#endif
	return 0;
}


/**************************************************************************************************
 * @ obj This is the ubus object for zigbee basic attributes
 **************************************************************************************************/
enum {
	ZB_NEW_BASIC_ATTR_ADDR,
	ZB_NEW_BASIC_ATTR_STATUS,
	ZB_NEW_BASIC_ATTR_MANUNAME,
	ZB_NEW_BASIC_ATTR_MODEL,
	__ZB_NEW_BASIC_ATTR_MAX
};

static const struct blobmsg_policy zb_new_basic_attributes_policy[__ZB_NEW_BASIC_ATTR_MAX] = {
	[ZB_NEW_BASIC_ATTR_ADDR] = { .name = "device_addr", .type = BLOBMSG_TYPE_STRING },
	[ZB_NEW_BASIC_ATTR_STATUS] = { .name = "status", .type = BLOBMSG_TYPE_INT32 },
	[ZB_NEW_BASIC_ATTR_MANUNAME] = { .name = "manufacture", .type = BLOBMSG_TYPE_STRING },
	[ZB_NEW_BASIC_ATTR_MODEL] = { .name = "model", .type = BLOBMSG_TYPE_STRING },
};


int ubus_call_zstack_basic_attribute_cree(char *device_addr, int endpoint_id) {

#if printf_VERBOSE
	printf("ubus_call_zstack_basic_attribute_cree Start\n");
#endif

#if printf_VERBOSE
	printf("device_addr %s, endpoint_id %d\n", device_addr, endpoint_id);
#endif

	if(sem_getvalue(&semO, &semaphore_value) != -1) {
		printf("orbweb sem_getvalue = %d\n", semaphore_value);
	}

	if (sem_trywait(&semO) != 0) {
		queue_full = 1;
		printf("sem_trywait can't acquire, queue is full\n");
		goto END;
	}
	else {
		queue_full = 0;
	}

	if(sem_getvalue(&semO, &semaphore_value) != -1) {
		printf("orbweb sem_getvalue = %d\n", semaphore_value);
	}	

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "dev_addr", device_addr);
	blobmsg_add_u32(&b, "endpoint_id", endpoint_id);
	ubus_invoke(ubus, objid_zstack, "basic_attr_cree", b.head, NULL, 0, 1000);
END:
#if printf_VERBOSE
	printf("ubus_call_zstack_basic_attribute_cree End\n");
#endif
	return 0;
}

int ubus_call_zstack_binding_endpoints(binding_mode_t binding_mode, char *device_addr, int endpoint_id, int cluster_id) {

#if printf_VERBOSE
	printf("ubus_call_zstack_binding_endpoints Start\n");
#endif

#if printf_VERBOSE
	printf("Sending %s command: device_addr %s, endpoint_id %d, cluster_id %d\n", binding_mode == BINDING_MODE_BIND ? "bind" : "unbind", device_addr, endpoint_id, cluster_id);
#endif

	if(sem_getvalue(&semO, &semaphore_value) != -1) {
		printf("orbweb sem_getvalue = %d", semaphore_value);
	}

	if (sem_trywait(&semO) != 0) {
		queue_full = 1;
		printf("sem_trywait can't acquire, queue is full");
		goto END;
	}
	else {
		queue_full = 0;
	}

	if(sem_getvalue(&semO, &semaphore_value) != -1) {
		printf("orbweb sem_getvalue = %d", semaphore_value);
	}	

	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "mode", binding_mode);
	blobmsg_add_string(&b, "src_device_id", device_addr);
	blobmsg_add_u32(&b, "src_endpoint", endpoint_id);
	blobmsg_add_u32(&b, "cluster_id", cluster_id);
	ubus_invoke(ubus, objid_zstack, "bind", b.head, NULL, 0, 1000);
END:
#if printf_VERBOSE
	printf("ubus_call_zstack_binding_endpoints End");
#endif
	return 0;
}




/**************************************************************************************************
 *
 * @fn          handle_zb_new_basic_attributes
 *
 * @brief       Handle basic attributes of new zigbee device and publish to the cloud
 *
 **************************************************************************************************/
static int handle_zb_new_basic_attributes(struct blob_attr *msg) {

#if printf_VERBOSE
	printf("handle_zb_new_basic_attributes Start");
#endif
	struct blob_attr *tb[__ZB_NEW_BASIC_ATTR_MAX], *c;
	char *IEEEAddress = "0000000000000000";
	char *manufacture = "";
	char *model = "";
	char name[MAX_LENGTH_NAME];
	char clusters[128];
	int device_status = 1;
	int new_device_status = 0;
	void *cluster_array;
	// Mark this, since reading battery is unnecessary
	/*bool power_config_cluster_support = false;
	  int i = 0;*/

	memset(name, 0, sizeof(name));
	memset(clusters, 0, sizeof(clusters));

#if printf_VERBOSE
	printf("ZB NEW Basic Attributes Package %s", blobmsg_format_json(msg, true));
#endif
	blobmsg_parse(zb_new_basic_attributes_policy, __ZB_NEW_BASIC_ATTR_MAX, tb, blob_data(msg), blob_len(msg));

	if ((c = tb[ZB_NEW_BASIC_ATTR_ADDR])) {
		IEEEAddress = blobmsg_get_string(c);
		printf("DeviceAddr %s\n", IEEEAddress);
	}
	if ((c = tb[ZB_NEW_BASIC_ATTR_STATUS])) {
		device_status = blobmsg_get_u32(c);
		printf("device_status %d\n", device_status);
	}
	if ((c = tb[ZB_NEW_BASIC_ATTR_MANUNAME])) {
		manufacture = strdup(blobmsg_get_string(c));
		printf("manufacture[%d] %s\n", strlen(manufacture), manufacture);
	}
	if ((c = tb[ZB_NEW_BASIC_ATTR_MODEL])) {
		model = strdup(blobmsg_get_string(c));
		printf("model[%d] %s\n", strlen(model), model);
	}

	if((strncmp(manufacture, "CREE", 4)==0) && (strlen(model)==0)) { // re-read ModelIdentifier
		char endpoint_text[10];
		int endpoint_int = 0;

		memset(endpoint_text, 0, sizeof(endpoint_text));
		sprintf(endpoint_text, "%s", sql_retrieve_device_endpoint(IEEEAddress));

		endpoint_int = (int)strtoul(endpoint_text, NULL, 0);

		// Only update manufacture name
		sql_update_basic_device(IEEEAddress, "", manufacture, "");

		printf("This is the type of Cree devices, endpoint %d\n", endpoint_int);
		ubus_call_zstack_basic_attribute_cree(IEEEAddress, endpoint_int);

	} else if((strncmp(manufacture, "ClimaxTechnology", 16)==0) 
			&& (strncmp(model, "RS_00.00.03.04TC", 16)==0)) {

		char endpoint_text[10];
		int endpoint_int = 0;

		memset(endpoint_text, 0, sizeof(endpoint_text));
		sprintf(endpoint_text, "%s", sql_retrieve_device_endpoint(IEEEAddress));

		endpoint_int = (int)strtoul(endpoint_text, NULL, 0);

		// Update the basic attributes of the device
		// Default-Name: "[manufacture] - [Device ID]"
		sprintf(name, "%s - %s", manufacture, IEEEAddress+12);
		printf("name %s\n", name);
		sql_update_basic_device(IEEEAddress, name, manufacture, model);

		printf("This is the TEMPERATURE SENSOR of ClimaxTechnology, needing to bind endpoints...\n");
		ubus_call_zstack_binding_endpoints(BINDING_MODE_BIND, IEEEAddress, endpoint_int, ZSTACK_CLUSTER_TEMP);
	} else if((strncmp(manufacture, "ClimaxTechnology", 16)==0)
			&& (strncmp(model, "LMHT", 4)==0)) {

		char endpoint_text[10];
		int endpoint_int = 0;

		memset(endpoint_text, 0, sizeof(endpoint_text));
		sprintf(endpoint_text, "%s", sql_retrieve_device_endpoint(IEEEAddress));

		endpoint_int = (int)strtoul(endpoint_text, NULL, 0);

		// Update the basic attributes of the device
		// Default-Name: "[manufacture] - [Device ID]"
		sprintf(name, "%s - %s", manufacture, IEEEAddress+12);
		printf("name %s\n", name);
		sql_update_basic_device(IEEEAddress, name, manufacture, model);

		printf("This is the TEMPERATURE SENSOR of ClimaxTechnology, needing to bind endpoints...\n");
		ubus_call_zstack_binding_endpoints(BINDING_MODE_BIND, IEEEAddress, endpoint_int, ZSTACK_CLUSTER_TEMP);

		sprintf(name, "%s - %s", manufacture, IEEEAddress+12);
		printf("name %s\n", name);

		// Update the basic attributes of the device
		sql_update_basic_device(IEEEAddress, name, manufacture, model);

		new_device_status = sql_retrieve_device_status(IEEEAddress);
		printf("new_device_status %d\n", new_device_status);

		if(new_device_status==1) {

			sprintf(clusters, "%s", sql_retrieve_device_clusters(IEEEAddress));
			printf("clusters %s\n", clusters);

			char *cluster = strtok(clusters, ",");
			blob_buf_init(&b, 0);
			blobmsg_add_string(&b, "name", name);
			blobmsg_add_string(&b, "mac", IEEEAddress);
			blobmsg_add_string(&b, "manufacture", manufacture);
			blobmsg_add_string(&b, "model", model);
			cluster_array = blobmsg_open_array(&b, "cluster");
			while (cluster != NULL) {
				//printf("cluster %s", cluster);
				blobmsg_add_string(&b, NULL, cluster);
				cluster = strtok(NULL, ",");
			}
			blobmsg_close_array(&b, cluster_array);

			char cPayload[200];
			memset(cPayload, 0, sizeof(cPayload));
			sprintf(cPayload, "%s", blobmsg_format_json(b.head, true));
			printf("blobmsg json %s\n", cPayload);

			FILE *f = fopen("/etc/gizmo/action/learn_mode_list", "a");
			if (f == NULL)
			{
				printf("Error opening file!\n");
				exit(1);
			}

			/* print some text */
			const char *text = "Write this to the file";
			fprintf(f, "%s\n", cPayload);
			fclose(f);

			sql_update_new_status(IEEEAddress, 0);
		}




	} else {

		if(strlen(manufacture)==0) { // for re-reading devices
			manufacture = sql_retrieve_device_manufacture_name(IEEEAddress);
			printf("set manufacture to %s\n", manufacture);
		} else if((strncmp(manufacture, "Securifi Ltd.", 13)==0) && (strlen(model)==0)) {
			// for Securifi Power Outlet
			model = strdup(manufacture);
			printf("copy %s to model %s\n", manufacture, model);
		}


		// We don't allow this two fields are all empty
		if((strlen(manufacture)!=0) || (strlen(model)!=0)) {

			// Default-Name: "[manufacture] - [Device ID]"
			sprintf(name, "%s - %s", manufacture, IEEEAddress+12);
			printf("name %s\n", name);

			// Update the basic attributes of the device
			sql_update_basic_device(IEEEAddress, name, manufacture, model);

			new_device_status = sql_retrieve_device_status(IEEEAddress);
			printf("new_device_status %d\n", new_device_status);

			if(new_device_status==1) {

				sprintf(clusters, "%s", sql_retrieve_device_clusters(IEEEAddress));
				printf("clusters %s\n", clusters);

				char *cluster = strtok(clusters, ",");
				blob_buf_init(&b, 0);
				blobmsg_add_string(&b, "name", name);
				blobmsg_add_string(&b, "mac", IEEEAddress);
				blobmsg_add_string(&b, "manufacture", manufacture);
				blobmsg_add_string(&b, "model", model);
				cluster_array = blobmsg_open_array(&b, "cluster");
				while (cluster != NULL) {
					//printf("cluster %s", cluster);
					blobmsg_add_string(&b, NULL, cluster);
					cluster = strtok(NULL, ",");
				}
				blobmsg_close_array(&b, cluster_array);

				char cPayload[200];
				memset(cPayload, 0, sizeof(cPayload));
				sprintf(cPayload, "%s", blobmsg_format_json(b.head, true));
				printf("blobmsg json %s\n", cPayload);

				FILE *f = fopen("/etc/gizmo/action/learn_mode_list", "a");
				if (f == NULL)
				{
					printf("Error opening file!\n");
					exit(1);
				}

				/* print some text */
				const char *text = "Write this to the file";
				fprintf(f, "%s\n", cPayload);
				fclose(f);

				sql_update_new_status(IEEEAddress, 0);
			}

		}
	}


free(model);
free(manufacture);
#if printf_VERBOSE
printf("handle_zb_new_basic_attributes End");
#endif
return 0;
}


/**************************************************************************************************
 * @ obj This is the ubus object for zigbee reading attributes
 **************************************************************************************************/
enum {
	ZB_READ_ATTR_ADDR,
	ZB_READ_ATTR_STATUS,
	ZB_READ_ATTR_CLUSTER,
	ZB_READ_ATTR_ATTR,
	ZB_READ_ATTR_VALUE,
	__ZB_READ_ATTR_MAX
};

static const struct blobmsg_policy zb_read_attribute_policy[__ZB_READ_ATTR_MAX] = {
	[ZB_READ_ATTR_ADDR] = { .name = "device_addr", .type = BLOBMSG_TYPE_STRING },
	[ZB_READ_ATTR_STATUS] = { .name = "status", .type = BLOBMSG_TYPE_INT32 },
	[ZB_READ_ATTR_CLUSTER] = { .name = "cluster", .type = BLOBMSG_TYPE_STRING },
	[ZB_READ_ATTR_ATTR] = { .name = "attribute", .type = BLOBMSG_TYPE_STRING },
	[ZB_READ_ATTR_VALUE] = { .name = "value", .type = BLOBMSG_TYPE_STRING },
};

/**************************************************************************************************
 *
 * @fn          handle_zb_read_attribute
 *
 * @brief       Handle basic attributes of new zigbee device and publish to the cloud
 *
 **************************************************************************************************/
static int handle_zb_read_attribute(struct blob_attr *msg) {

#if printf_VERBOSE
	printf("handle_zb_read_attribute Start");
#endif
	//int device_acquired = 0;
	struct blob_attr *tb[__ZB_READ_ATTR_MAX], *c;
	char *IEEEAddress = "0x0000000000000000";
	char *cluster = "";
	char *attribute = "";
	char *value = "";
	ZstackStatusReplyT device_status = ZSTACK_STATUS_REPLY_FAIL;

	char endpoint_text[10];
	int new_device_status = 0;
	bool publish_to_cloud = true;

	memset(endpoint_text, 0, sizeof(endpoint_text));

#if printf_VERBOSE
	printf("ZB Read Attribute Package %s", blobmsg_format_json(msg, true));
#endif
	blobmsg_parse(zb_read_attribute_policy, __ZB_READ_ATTR_MAX, tb, blob_data(msg), blob_len(msg));

	if ((c = tb[ZB_READ_ATTR_ADDR])) {
		IEEEAddress = strdup(blobmsg_get_string(c));
#if printf_VERBOSE
		printf("DeviceAddr %s", IEEEAddress);
#endif
	}
	if ((c = tb[ZB_READ_ATTR_STATUS])) {
		device_status = blobmsg_get_u32(c);
#if printf_VERBOSE
		printf("device_status %d", device_status);
#endif
	}
	if ((c = tb[ZB_READ_ATTR_CLUSTER])) {
		cluster = strdup(blobmsg_get_string(c));
#if printf_VERBOSE
		printf("cluster %s", cluster);
#endif
	}
	if ((c = tb[ZB_READ_ATTR_ATTR])) {
		attribute = strdup(blobmsg_get_string(c));
#if printf_VERBOSE
		printf("attribute %s", attribute);
#endif
	}
	if ((c = tb[ZB_READ_ATTR_VALUE])) {
		value = strdup(blobmsg_get_string(c));
#if printf_VERBOSE
		printf("value %s", value);
#endif
	}

	sprintf(endpoint_text, "%s", sql_retrieve_device_endpoint(IEEEAddress));
	if(strlen(endpoint_text)==0) {
		// Since this is unpaired from zsatck, but the address is not existed in database, ingoring this
#if printf_VERBOSE
		printf("hide this attribute event of %s from zstack...", IEEEAddress);
#endif				
		publish_to_cloud = false;
	}
	new_device_status = sql_retrieve_device_status(IEEEAddress);
	if(new_device_status==1) {
#if printf_VERBOSE
		printf("hide this attribute event of %s that is unfinished joining...", IEEEAddress);
#endif				
		publish_to_cloud = false;
	}


	if(publish_to_cloud) {

		// publish to cloud via MQTT
		// rc = publish_attribute_to_cloud(device_status, value, IEEEAddress, cluster, attribute);

		blob_buf_init(&b, 0);
		switch(device_status) {
			case ZSTACK_STATUS_REPLY_SUCCESS:
				blobmsg_add_string(&b, "status", "success");
				break;
			case ZSTACK_STATUS_REPLY_NO_DEVICE:
				blobmsg_add_string(&b, "status", "no device");
				break;
			case ZSTACK_STATUS_REPLY_BUSY:
				blobmsg_add_string(&b, "status", "busy");
				break;
			case ZSTACK_STATUS_REPLY_TIMEOUT:
				blobmsg_add_string(&b, "status", "timeout");
				break;
			case ZSTACK_STATUS_REPLY_NOT_SUPPORT:
				blobmsg_add_string(&b, "status", "not support");
				break;
			default:
				blobmsg_add_string(&b, "status", "failure");
				break;
		}
		blobmsg_add_string(&b, "value", value);

		char cPayload[200];
		memset(cPayload, 0, sizeof(cPayload));
		sprintf(cPayload, "%s", blobmsg_format_json(b.head, true));
		printf("blobmsg json %s\n", cPayload);

		send_device_status(IEEEAddress,value);

	}

#if printf_VERBOSE
	printf("handle_zb_read_attribute End");
#endif
	free(IEEEAddress);
	free(cluster);
	free(attribute);
	return 0;
}



/**************************************************************************************************
 * @ obj This is the ubus object for zigbee reading attributes
 **************************************************************************************************/
enum {
	ZB_READ_ATTR_CNF_ADDR,
	ZB_READ_ATTR_CNF_STATUS,
	ZB_READ_ATTR_CNF_CLUSTER,
	ZB_READ_ATTR_CNF_ATTR,
	__ZB_READ_ATTR_CNF_MAX
};

static const struct blobmsg_policy zb_read_attribute_cnf_policy[__ZB_READ_ATTR_CNF_MAX] = {
	[ZB_READ_ATTR_CNF_ADDR] = { .name = "device_addr", .type = BLOBMSG_TYPE_STRING },
	[ZB_READ_ATTR_CNF_STATUS] = { .name = "status", .type = BLOBMSG_TYPE_INT32 },
	[ZB_READ_ATTR_CNF_CLUSTER] = { .name = "cluster", .type = BLOBMSG_TYPE_STRING },
	[ZB_READ_ATTR_CNF_ATTR] = { .name = "attribute", .type = BLOBMSG_TYPE_STRING },
};

/**************************************************************************************************
 *
 * @fn          handle_zb_read_attr_cnf
 *
 * @brief       Handle reading attributes confirm
 *
 **************************************************************************************************/
static int handle_zb_read_attr_cnf(struct blob_attr *msg) {

#if printf_VERBOSE
	printf("handle_zb_read_attr_cnf Start");
#endif
	struct blob_attr *tb[__ZB_READ_ATTR_CNF_MAX], *c;
	char *IEEEAddress = "0x0000000000000000";
	char *cluster = "";
	char *attribute = "";
	ZstackStatusReplyT device_status = ZSTACK_STATUS_REPLY_FAIL;

#if printf_VERBOSE
	printf("ZB Read Attribute Confirm Package %s", blobmsg_format_json(msg, true));
#endif
	blobmsg_parse(zb_read_attribute_cnf_policy, __ZB_READ_ATTR_CNF_MAX, tb, blob_data(msg), blob_len(msg));

	if ((c = tb[ZB_READ_ATTR_ADDR])) {
		IEEEAddress = strdup(blobmsg_get_string(c));
#if printf_VERBOSE
		printf("DeviceAddr %s\n", IEEEAddress);
#endif
	}
	if ((c = tb[ZB_READ_ATTR_STATUS])) {
		device_status = blobmsg_get_u32(c);
#if printf_VERBOSE
		printf("device_status %d\n", device_status);
#endif
	}
	if ((c = tb[ZB_READ_ATTR_CLUSTER])) {
		cluster = strdup(blobmsg_get_string(c));
#if printf_VERBOSE
		printf("cluster %s\n", cluster);
#endif
	}
	if ((c = tb[ZB_READ_ATTR_ATTR])) {
		attribute = strdup(blobmsg_get_string(c));
#if printf_VERBOSE
		printf("attribute %s\n", attribute);
#endif
	}

	if(device_status!=ZSTACK_STATUS_REPLY_SUCCESS) {
#if printf_VERBOSE
		printf("Publish reading confirm errors\n");
#endif


		blob_buf_init(&b, 0);
		switch(device_status) {
			case ZSTACK_STATUS_REPLY_SUCCESS:
				blobmsg_add_string(&b, "status", "success");
				break;
			case ZSTACK_STATUS_REPLY_NO_DEVICE:
				blobmsg_add_string(&b, "status", "no device");
				break;
			case ZSTACK_STATUS_REPLY_BUSY:
				blobmsg_add_string(&b, "status", "busy");
				break;
			case ZSTACK_STATUS_REPLY_TIMEOUT:
				blobmsg_add_string(&b, "status", "timeout");
				break;
			case ZSTACK_STATUS_REPLY_NOT_SUPPORT:
				blobmsg_add_string(&b, "status", "not support");
				break;
			default:
				blobmsg_add_string(&b, "status", "failure");
				break;
		}

		char cPayload[200];
		memset(cPayload, 0, sizeof(cPayload));
		sprintf(cPayload, "%s", blobmsg_format_json(b.head, true));
		printf("blobmsg json %s\n", cPayload);



	} else {
#if printf_VERBOSE
		printf("Successfully sending reading attribute into Zstack...\n");
#endif
	}

	if(sem_getvalue(&semO, &semaphore_value) != -1) {
		printf("orbweb sem_getvalue = %d\n", semaphore_value);
	}

	sem_post(&semO);

	if(sem_getvalue(&semO, &semaphore_value) != -1) {
		printf("orbweb sem_getvalue = %d\n", semaphore_value);
	}

#if printf_VERBOSE
	printf("handle_zb_read_attr_cnf End\n");
#endif
	free(IEEEAddress);
	free(cluster);
	free(attribute);
	return 0;
}



static int zstack_notify(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {
	printf("--->Orbweb received notification of %s from zstack \n", method);

	if(strcmp(method, "pairevent")==0) {
		handle_zb_pairevent(msg);
	} else if(strcmp(method, "device_announce")==0) {
		handle_zb_device_announce(msg);
	} else if(strcmp(method, "new_found_device")==0) {
		handle_zb_new(msg);
	} else if(strcmp(method, "basic_attributes")==0) {
		handle_zb_new_basic_attributes(msg);
	} else if(strcmp(method, "read_attribute")==0) {
		handle_zb_read_attribute(msg);
	} else if(strcmp(method, "read_attr_cnf")==0) {
		handle_zb_read_attr_cnf(msg);
	} 


	/*
	   if(strcmp(method, "pairevent")==0) {
	   handle_zb_pairevent(msg);
	   } else if(strcmp(method, "new_found_device")==0) {
	   handle_zb_new(msg);
	   } else if(strcmp(method, "update_device")==0) {
	   handle_zb_update_device(msg);
	   } else if(strcmp(method, "basic_attributes")==0) {
	   handle_zb_new_basic_attributes(msg);
	   } else if(strcmp(method, "read_attribute")==0) {
	   handle_zb_read_attribute(msg);
	   } else if(strcmp(method, "read_attr_cnf")==0) {
	   handle_zb_read_attr_cnf(msg);
	   } else if(strcmp(method, "command_reply")==0) {
	   handle_zb_cmd(msg);
	   } else if(strcmp(method, "reset_reply")==0) {
	   handle_zb_zll_reset(msg);
	   } else if(strcmp(method, "refresh")==0) {
	   handle_zb_refresh(msg);
	   } else if(strcmp(method, "device_announce")==0) {
	   handle_zb_device_announce(msg);
	   } else if(strcmp(method, "scan_response")==0) {
	   handle_zb_scan_response(msg);
	   } else if(strcmp(method, "groupevent_reply")==0) {
	   handle_zb_groupevent(msg);
	   } else if(strcmp(method, "add_sceneevent_reply")==0) {
	   handle_zb_add_sceneevent(msg);
	   } else if(strcmp(method, "scenesevent_reply")==0) {
	   handle_zb_scenesevent(msg);
	   } else if(strcmp(method, "binding_reply")==0) {
	   handle_zb_binding(msg);
	   }
	 */
	return 0;
}

static void update_zstack(bool subscribe) {
	if (subscribe)
		ubus_subscribe(ubus, &zstack, objid_zstack);
}

enum {
	OBJ_ATTR_ID,
	OBJ_ATTR_PATH,
	OBJ_ATTR_MAX
};

static const struct blobmsg_policy obj_attrs[OBJ_ATTR_MAX] = {
	[OBJ_ATTR_ID] = { .name = "id", .type = BLOBMSG_TYPE_INT32 },
	[OBJ_ATTR_PATH] = { .name = "path", .type = BLOBMSG_TYPE_STRING },
};

static void handle_event(struct ubus_context *ctx, struct ubus_event_handler *ev, 
		const char *type, struct blob_attr *msg) {
#if printf_VERBOSE       
	printf("handle_event Start\n");
#endif
	struct blob_attr *tb[OBJ_ATTR_MAX];
	blobmsg_parse(obj_attrs, OBJ_ATTR_MAX, tb, blob_data(msg), blob_len(msg));

	if(!tb[OBJ_ATTR_ID] || !tb[OBJ_ATTR_PATH])
		return;

	if(strcmp(blobmsg_get_string(tb[OBJ_ATTR_PATH]), "zstack"))
		return;

	objid_zstack = blobmsg_get_u32(tb[OBJ_ATTR_ID]);
	printf("orbweb acquire zstack objid: %zu\n", objid_zstack);

	update_zstack(true);
#if printf_VERBOSE
	printf("handle_event End\n");
#endif
}

static struct ubus_event_handler event_handler = { .cb = handle_event };

int main(int argc, char **argv)
{
	const char *ubus_socket = NULL;
	int ch;

	if(argc != 2){
		printf("Usage %s <MAC Address>\n",argv[0]);
		exit(1);
	}

	create_table_if_doesnot_exist();

	strcpy(cfg.mac, argv[1]);

	while ((ch = getopt(argc, argv, "cs:")) != -1) {
		switch (ch) {
			case 's':
				ubus_socket = optarg;
				break;
			default:
				break;
		}
	}
	uloop_init();

	printf("orbweb init_ubus\n");

	if(sem_init(&semO, 0, QUEUE_MAX_LIMIT) == -1) {
		printf("sem_init\n");
	}

	if(sem_getvalue(&semO, &semaphore_value) != -1) {
		printf("orbweb sem_getvalue = %d\n", semaphore_value);
	}

	if(!(ubus = ubus_connect(NULL))) {
		printf("Failed to connect to ubus\n");
		return -1;
	}

	printf("zstack object %zu\n", objid_zstack);

	zstack.cb = zstack_notify;
	ubus_register_subscriber(ubus, &zstack);

	ubus_add_uloop(ubus);

	// ubus_add_object(ubus, &main_object);
	ubus_register_event_handler(ubus, &event_handler, "ubus.object.add");
	if(!ubus_lookup_id(ubus, "zstack", &objid_zstack)) {
		printf("Found zstack object %zu\n", objid_zstack);
		update_zstack(true);
	}
	uloop_run();
	return 0;
}
