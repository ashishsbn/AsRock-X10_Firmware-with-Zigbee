/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file subscribe_publish_sample.c
 * @brief simple MQTT publish and subscribe on the same topic
 *
 * This example takes the parameters from the aws_iot_config.h file and establishes a connection to the AWS IoT MQTT Platform.
 * It subscribes and publishes to the same topic - "sdkTest/sub"
 *
 * If all the certs are correct, you should see the messages received by the application in a loop.
 *
 * The application takes in the certificate path, host name , port and the number of times the publish should happen.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>  

#include <signal.h>
#include <memory.h>
#include <sys/time.h>
#include <limits.h>

#include <libubus.h>
#include <sqlite3.h>

#include "log.h"
// #include "aws_iot_version.h"
#include "aws_iot_mqtt_interface.h"
// #include "aws_iot_config.h"
#include "asrock_mqttd.h"

/**
 * @brief Default cert location
 */
char certDirectory[PATH_MAX + 1] = {0};

char HostAddress[255] = {0};

uint32_t tls_port = 8883;

//int32_t i = 0;
bool debug_subscribe = false;

static void sighandler(_unused int signal) {
	DEBUG("sighandler...");
	exit_ubus();
	close_database();
}

static void usage(void) {

	INFO("asrock_mqttd usage:");
	INFO("  --help       show usage information");
	INFO("  --version    show asrock_mqttd version");
	INFO("  --url        cloud URL configuration");
	INFO("  --port       cloud port configuration");
	INFO("  --cert       certificate root directory");
	INFO("  --debug      subscribe all topics for debuging");
}

int main(int argc, char** argv) {
	
	if( argc < 2 ) {
		usage();
		return 0;
	}

	int opt;
	struct option long_opts[] = {
		{ "help", 0, NULL, 'h' },
		{ "version", 0, NULL, 'v' },
		{ "url", 1, NULL, 'u' },
		{ "port", 1, NULL, 'p' },
		{ "cert", 1, NULL, 'c' },
		{ "debug", 0, NULL, 'd' },
		{ 0,0,0,0 }
	};

	while (-1 != (opt = getopt_long(argc, argv, "hvu:p:c:d", long_opts, NULL))) {
		switch (opt) {
			case EOF:
				break;
			case 'h':
				usage();
				return 0;
			case 'v':
				INFO("asrock_mqttd version %s", ORBWEB_VERSION);
				return 0;
			case 'u':
				strcpy(HostAddress, optarg);
#if DEBUG_VERBOSE
				DEBUG("url %s", HostAddress);
#endif
				break;
			case 'p':
				tls_port = atoi(optarg);
#if DEBUG_VERBOSE
				DEBUG("port %d", tls_port);
#endif
				break;
			case 'c':
				strcpy(certDirectory, optarg);
#if DEBUG_VERBOSE
				DEBUG("cert root directory %s", certDirectory);
#endif
				break;
			case 'd':
				debug_subscribe = true;
#if DEBUG_VERBOSE
				DEBUG("Using debug mode...");
#endif
				break;
			default:
				usage();
				return 1;
		}
	}

	uloop_init();

	init_ubus();

	if(open_database()!=SQLITE_OK)
		return 2;

	signal(SIGUSR1, SIG_IGN);
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);

	asrock_mqttd_run();

	exit_ubus();
	close_database();

	return 0;
}

