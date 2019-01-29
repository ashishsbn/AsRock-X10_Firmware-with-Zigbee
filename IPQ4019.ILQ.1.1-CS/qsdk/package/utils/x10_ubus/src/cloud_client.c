/**
 *
 *   Gizmo Smart Gateway
 *
 *   jinul_24@yahoo.in
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <curl/curl.h>
#include <sqlite3.h>

//#if defined(LIBXML_WRITER_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)

#define MY_ENCODING "UTF-8"
#define POLLING_URL "http://app1.gizmosmart.io:8902/iot/tst_srvr/1.3/public/poll"

void testXmlwriterFilename(const char *uri);
void testXmlwriterMemory(const char *file);
char* createPollingRequest(const char *mac);
void sendStatusUpdate(char *post_data);
void execute_command(char *command, char *request_id, char *device_id, char *value);
xmlChar *ConvertInput(const char *in, const char *encoding);
//size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);
static void fetch_command(char * command);
char HA_script[256] = "/etc/gizmo/add_rule"; 
char schedule_script[256] = "/etc/gizmo/add_schedule"; 
char delete_gateway[256] = "/etc/gizmo/delete_gateway";
char group_script[256] = "/etc/gizmo/add_group"; 
char db_path[256] = "/etc/orbweb.db";

int hub_mode;

struct config
{
	char mac[32];
	char ip[32];
	char netmask[32];
	char gateway_ip[32];
}cfg;

struct firmwareInfo
{
        char version[32];
        char url[256];
        char md5[40];
}
fmInfo;

int
main(int argc, char *argv[])
{
	/*
	 * this initialize the library and check potential ABI mismatches
	 * between the version it was compiled for and the actual shared
	 * library used.
	 */
	if(argc != 5){
		printf("Usage %s <MAC Address> <IP Address> <Net Mask> <Gateway Address>\n",argv[0]);
		exit(1);
	}
	//struct config cfg;
	strcpy(cfg.mac, argv[1]);
	strcpy(cfg.ip, argv[2]);
	strcpy(cfg.netmask, argv[3]);
	strcpy(cfg.gateway_ip, argv[4]);
	LIBXML_TEST_VERSION
		printf("MAC Address:%s\n",cfg.mac );
	char *post_data;
	post_data = createPollingRequest(argv[1]);
	while(1){    
		sendStatusUpdate(post_data);
		//execute_command("hello","world");
		sleep(1);
	}
	xmlCleanupParser();
	return 0;
}



xmlChar *
ConvertInput(const char *in, const char *encoding)
{
	xmlChar *out;
	int ret;
	int size;
	int out_size;
	int temp;
	xmlCharEncodingHandlerPtr handler;

	if (in == 0)
		return 0;

	handler = xmlFindCharEncodingHandler(encoding);

	if (!handler) {
		printf("ConvertInput: no encoding handler found for '%s'\n",
				encoding ? encoding : "");
		return 0;
	}

	size = (int) strlen(in) + 1;
	out_size = size * 2 - 1;
	out = (unsigned char *) xmlMalloc((size_t) out_size);

	if (out != 0) {
		temp = size - 1;
		ret = handler->input(out, &out_size, (const xmlChar *) in, &temp);
		if ((ret < 0) || (temp - size + 1)) {
			if (ret < 0) {
				printf("ConvertInput: conversion wasn't successful.\n");
			} else {
				printf
					("ConvertInput: conversion wasn't successful. converted: %i octets.\n",
					 temp);
			}

			xmlFree(out);
			out = 0;
		} else {
			out = (unsigned char *) xmlRealloc(out, out_size + 1);
			out[out_size] = 0;  /*null terminating out */
		}
	} else {
		printf("ConvertInput: no memory\n");
	}

	return out;
}

char*
createPollingRequest(const char *mac)
{
	int rc;
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	xmlChar *tmp;

	/* Create a new XML buffer, to which the XML document will be
	 * written */
	buf = xmlBufferCreate();
	if (buf == NULL) {
		printf("testXmlwriterMemory: Error creating the xml buffer\n");
		return "";
	}

	/* Create a new XmlWriter for memory, with no compression.
	 * Remark: there is no compression for this kind of xmlTextWriter */
	writer = xmlNewTextWriterMemory(buf, 0);
	if (writer == NULL) {
		printf("testXmlwriterMemory: Error creating the xml writer\n");
		return "";
	}

	/* Start the document with the xml default for the version,
	 * encoding ISO 8859-1 and the default for the standalone
	 * declaration. */
	rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartDocument\n");
		return "";
	}

	rc = xmlTextWriterStartElement(writer, BAD_CAST "p");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
		return "";
	}

	/* Add an attribute with name "mac" to root "p" */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "mac");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
		return "";
	}

	/* Add an attribute with name "v" (value of MAC address)  */
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
			BAD_CAST mac);
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
		return "";
	}

	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
		return "";
	}

	/* Start an element named "HEADER" as child of ORDER. */
	rc = xmlTextWriterStartElement(writer, BAD_CAST "cmds");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
		return "";
	}

	rc = xmlTextWriterStartElement(writer, BAD_CAST "referer");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
		return "";
	}
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
			BAD_CAST "panel/poll");
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
		return "";
	}

	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
		return "";
	}


	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
		return "";
	}

	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		printf("testXmlwriterMemory: Error at xmlTextWriterEndElement\n");
		return "";
	}

	xmlFreeTextWriter(writer);

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

	curl_global_cleanup();
	xmlBufferFree(buf);
	return post_data;
}

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	if(size * nmemb > 0)
		fetch_command((char*)buffer);
	return size * nmemb;
}

void sendStatusUpdate(char *post_data){

	CURLcode res;
	CURL *curl = curl_easy_init();
	if(curl) {
		//printf("Polling Command\n");
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

static int hub_mode_callback(void *data, int argc, char **argv, char **azColName){
	char a = argv[0][0];
	hub_mode = a - '0';
	return 0;
}


int sql_get_mode()
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
		printf("Opened database successfully, retriving Hub Mode \n");
	}

	sprintf(sql,"select mode from hub where ID=1");
	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, hub_mode_callback, 0, &zErrMsg);
	printf("MODE is :%d \n",hub_mode);
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	} else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return hub_mode;



}

int sql_set_mode(int mode)
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
		printf("Opened database successfull , Changing hub mode to %d\n",mode);
	}

	sprintf(sql,"update hub set mode = %d where id = 1",mode);
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

static void
fetch_command(char * response) {
	xmlDocPtr doc;
	xmlNsPtr ns;
	xmlNodePtr cur;
	char command[64];
	char request_id[32];
#ifdef LIBXML_SAX1_ENABLED
	/*
	 * build an XML tree from a the file;
	 */
	printf("Command Payload: %s\n",response);
	doc = xmlReadMemory(response, strlen(response), "noname.xml", NULL, 0);
	//doc = xmlParseFile(filename);
	if (doc == NULL) return;
#else
	/*
	 * the library has been compiled without some of the old interfaces
	 */
	return;
#endif /* LIBXML_SAX1_ENABLED */

	/*
	 * Check the document is of the right kind
	 */
	// Ignore Blalnk Space
	xmlKeepBlanksDefault(0);
	cur = xmlDocGetRootElement(doc);
	while ( cur && xmlIsBlankNode ( cur ) ) {
		cur = cur -> next;
	}

	cur = cur->xmlChildrenNode;
		printf("XML NODE ->%s\n",cur->name);
	cur = cur -> next;
	
	if ( cur == 0 ) {
		xmlFreeDoc(doc);
		printf("DYING\n");
		return;
	}

		printf("XML NODE ->%s\n",cur->name);

	char device_id[32];
	char value[16];
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		printf("XML NODE ->%s\n",cur->name);
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "cmd")))
		{
			strcpy(command,cur->properties->children->content);
			cur->properties = cur->properties->next;
			strcpy(request_id,cur->properties->children->content);
			printf("Command Found : %s with Request_Id : %s \n", command,request_id);
			if(strcmp(command,"switchDevicePSS") == 0){
				strcpy(device_id,cur->xmlChildrenNode->properties->children->content);
				printf("Device id:%s\n",device_id);
				strcpy(value,cur->xmlChildrenNode->next->properties->children->content);
				printf("Change Value :%s\n",value);
			}else if(strcmp(command,"delDevice")==0){
				strcpy(device_id,cur->xmlChildrenNode->properties->children->content);
				printf("Device id:%s\n",device_id);
			}else if(strcmp(command,"fwUpgrade")==0){/*Updated by Rajbir Singh*/

                                strcpy(fmInfo.version,cur->xmlChildrenNode->next->properties->children->content);
                                printf("Firmware version:%s\n",fmInfo.version);

                                strcpy(fmInfo.url,cur->xmlChildrenNode->next->next->properties->children->content);
                                printf("Firmware URL:%s\n",fmInfo.url);

                                strcpy(fmInfo.md5,cur->xmlChildrenNode->next->next->next->properties->children->content);
                                printf("Firmwrae md5:%s\n",fmInfo.md5);
                        }else if(strcmp(command,"setHA") == 0){
				char setHA_rule_command[1024];
				printf("Rule --- %s\n",cur->xmlChildrenNode->properties->children->content);
				printf("Exec --- %s\n",cur->xmlChildrenNode->xmlChildrenNode->name);
				if(strcmp(cur->xmlChildrenNode->properties->next->children->content,"scheduling") == 0 || strcmp(cur->xmlChildrenNode->properties->next->children->content,"delete_schedule") == 0){
					sprintf(setHA_rule_command,"%s \"%s\" \"%s\" \"%s\" \"%s\"",
						schedule_script,
						cur->xmlChildrenNode->properties->children->content,
						cur->xmlChildrenNode->properties->next->children->content,
						cur->xmlChildrenNode->xmlChildrenNode->properties->children->content,
						cur->xmlChildrenNode->xmlChildrenNode->next->properties->children->content);
				}else{
					sprintf(setHA_rule_command,"%s \"%s\" \"%s\" \"%s\" \"%s\"",
						HA_script,
						cur->xmlChildrenNode->properties->children->content,
						cur->xmlChildrenNode->properties->next->children->content,
						cur->xmlChildrenNode->xmlChildrenNode->properties->children->content,
						cur->xmlChildrenNode->xmlChildrenNode->next->properties->children->content);
				}
				printf("Executing command:%s\n",setHA_rule_command);
				system(setHA_rule_command);
			}else if(strcmp(command,"applyScene") == 0){
				char applyScene_command[1024];
				char send_notification[1024];
				int scene_no = atoi(cur->xmlChildrenNode->properties->children->content);
				sprintf(applyScene_command,"ubus call zstack scenes \'\{ \"action\":4,\"group_id\":%d,\"scene_id\":%d\}\'",scene_no-1,scene_no-1);
				sprintf(send_notification,"/etc/gizmo/scene_triggered %d",scene_no-1);
				printf("Executing command:%s\n",applyScene_command);
				system(applyScene_command);
				system(send_notification);
			}else if (strcmp(command,"setMode") == 0){
				printf("Changing Hub mode to %s",cur->xmlChildrenNode->next->properties->children->content);	
				sql_set_mode(atoi(cur->xmlChildrenNode->next->properties->children->content));
			}else if (strcmp(command,"setDeviceGroup") == 0){
				char group_number[16];
				char group_command[2048];
				strcpy(group_number,cur->xmlChildrenNode->properties->children->content);
				sprintf(group_command,"%s \"%s\" \"%s\" \"addGroup\" ",group_script,group_number,cur->xmlChildrenNode->next->properties->children->content);
				printf("Executing command:%s\n",group_command);
				system(group_command);
			}else if(strcmp(command,"delGateway") == 0){
				system(delete_gateway);
			}else if(strcmp(command,"switchGroupPSS") == 0){
				char applyScene_command[1024];
				sprintf(applyScene_command,"/etc/gizmo/call_group \"%s\" \"%s\"",cur->xmlChildrenNode->properties->children->content,cur->xmlChildrenNode->next->properties->children->content);
				system(applyScene_command);
			}
		}
		if(cur != NULL)
			cur = cur->next;
	}
	xmlFreeDoc(doc);
	if(command != NULL && request_id != NULL ){
		execute_command(command,request_id,device_id,value);
	}else{
		printf("No command found.\n");
		return;
	}
}

void execute_command(char *command, char *request_id, char *device_id, char *value){
	int rc;
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	xmlChar *tmp;
	if(command == NULL)
		return;

	printf("executing command\n");
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
			BAD_CAST "panel/cgi");
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
			BAD_CAST command);
	if (rc < 0) {
		printf
			("testXmlwriterMemory: Error at xmlTextWriterWriteAttribute\n");
		return;
	}
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "id",
			BAD_CAST request_id);
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

	if(strcmp(command,"getNet")==0){

		rc = xmlTextWriterStartElement(writer, BAD_CAST "x");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterStartElement(writer, BAD_CAST "lanType");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "dhcp");
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


		rc = xmlTextWriterStartElement(writer, BAD_CAST "ip");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST cfg.ip);
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


		rc = xmlTextWriterStartElement(writer, BAD_CAST "netmask");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST cfg.netmask);
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

		rc = xmlTextWriterStartElement(writer, BAD_CAST "gw");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST cfg.gateway_ip);
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


		rc = xmlTextWriterStartElement(writer, BAD_CAST "dns1");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "8.8.8.8");
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


		rc = xmlTextWriterStartElement(writer, BAD_CAST "dns2");
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


		rc = xmlTextWriterStartElement(writer, BAD_CAST "dns_flush");
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
		
		rc = xmlTextWriterStartElement(writer, BAD_CAST "type");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}

		rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "v",
				BAD_CAST "asrock");
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
	else if(strcmp(command,"switchDevicePSS") == 0){
		char run_command[128];
		sprintf(run_command,"ubus call zstack control '{\"dev_addr\":\"%s\",\"endpoint_id\":1,\"cluster_id\":6,\"command_id\":%d,\"value\":\"0\"}'",device_id,atoi(value));
		printf("%s\n",run_command);
		int ret_val = system(run_command);
	
	}
	else if(strcmp(command,"delDevice") == 0){
		char run_command[128];
		sprintf(run_command,"ubus call zstack remove '{\"dev_addr\":\"%s\"}'",device_id);
		int ret_val = system(run_command);
	
	}/*sysUpgrade modified by Rajbir Singh on 8-aug-2018 at 12:38pm */
        else if(strcmp(command,"fwUpgrade")==0){
                char run_command[256];
                sprintf(run_command,"/etc/gizmo/fwUpgrade.sh %s %s",fmInfo.url,fmInfo.md5);
		printf("sysUpgrade %s\n",run_command);
                int ret_val = system(run_command);
        }
	else if(strcmp(command,"setHA") == 0){
		
	}
	else if(strcmp(command,"applyScene") == 0){

	}
	else if(strcmp(command,"setMode") == 0){

	}
	else if(strcmp(command,"switchGroupPSS") == 0){

	}
	else if(strcmp(command,"setSchedule") == 0){

	}
	else if(strcmp(command, "getAreaMode") == 0){
		rc = xmlTextWriterStartElement(writer, BAD_CAST "x");
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


		rc = xmlTextWriterStartElement(writer, BAD_CAST "no");
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

		rc = xmlTextWriterStartElement(writer, BAD_CAST "mode");
		if (rc < 0) {
			printf
				("testXmlwriterMemory: Error at xmlTextWriterStartElement\n");
			return;
		}
		sql_get_mode();
		rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "v",
				"%d",hub_mode);
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

	}else if(strcmp(command,"setDeviceGroup") == 0){


	}else if(strcmp(command,"delGateway") == 0){

	}else{

		xmlFreeTextWriter(writer);
		xmlBufferFree(buf);
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
