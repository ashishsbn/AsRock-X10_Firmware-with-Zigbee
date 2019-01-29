/*====================================================================*
 *
 *   FILE:
 *     charlietalk.cpp
 *
 *   DESCRIPTION:
 *     QTI QMI connection manager;
 *
 *
 *   Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *
 *   *  Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   *  Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   *  Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 * system header files;
 *--------------------------------------------------------------------*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

/*====================================================================*
 * custom header files;
 *--------------------------------------------------------------------*/

#include "GobiConnectionMgmtAPI.h"
#include "Gobi3000Translation.h"
#include "StdAfx.h"

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define QMIWDS 1
#define QMIDMS 2
#define QMINAS 3
#define GOBI_DEVICENAME "/dev/qcqmi0";
#define EXTRACT_BYTE(value,position) \
        ((value & (0xFF << ( position * 8))) >> (position * 8))

// qmux buffer values
#define MUX_BUFFER_SIZE 22
#define PERIPHERAL_END_POINT_TYPE 0x10
#define PERIPHERAL_END_POINT_LENGTH 0x08
#define PERIPHERAL_END_POINT_VALUE 0x02 // HSUSB
#define PERIPHERAL_INTERFACE_NUM 0x04
#define MUX_ID_TYPE 0x11
#define MUX_ID_LENGTH 0x01
#define MUX_ID_VALUE 0x81
#define CLIENT_TYPE 0x13
#define CLIENT_LENGTH 0x04
#define CLIENT_VALUE 0x01 //tethered type

/*===========================================================================
METHOD:
   qmux_fill_buffer (Public Method)

DESCRIPTION:
   Fills the buffer values before calling WDSMuxBinding

RETURN VALUE:
   int
===========================================================================*/


ULONG qmux_fill_buffer(ULONG *buffer_length, UINT8 *buffer, size_t buffer_size)
{
   if(buffer_size < MUX_BUFFER_SIZE)
      return eGOBI_ERR_BUFFER_SZ;

   *buffer_length = MUX_BUFFER_SIZE;
   *((UINT8  *)(buffer)) = PERIPHERAL_END_POINT_TYPE;
   *((UINT16 *)(buffer + 1)) = PERIPHERAL_END_POINT_LENGTH;
   *((UINT32 *)(buffer + 3)) = PERIPHERAL_END_POINT_VALUE;
   *((UINT32 *)(buffer + 7)) = PERIPHERAL_INTERFACE_NUM;
   *((UINT8  *)(buffer + 11)) = MUX_ID_TYPE;
   *((UINT16 *)(buffer + 12)) = MUX_ID_LENGTH;
   *((UINT8  *)(buffer + 14)) = MUX_ID_VALUE;
   *((UINT8  *)(buffer + 15)) = CLIENT_TYPE;
   *((UINT16 *)(buffer + 16)) = CLIENT_LENGTH;
   *((UINT32 *)(buffer + 18)) = CLIENT_VALUE;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   configure_usb_interface (Public Method)

DESCRIPTION:
   This function configures usb0 interface using uci command

PARAMETERS:
   option_name  [ I ] - option name of the interface
   option_value [ I ] - option value of the interface
   is_list      [ I ] - flag to denote whether option should be added as list

RETURN VALUE:
   int
===========================================================================*/

int configure_usb_interface(char *option_name, char *option_value, int is_list)
{
   char command[1024] = { 0 };
   if(!is_list)
      sprintf(command,"uci set network.usb.%s='%s'",option_name,option_value);
   else
      sprintf(command,"uci add_list network.usb.%s='%s'",option_name,option_value);
   system(command);
   system("uci commit");
   return 0;
}

/*====================================================================*
 *
 *   int main (int argc, char * argv []);
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char * argv [])

{
   GOBIHANDLE handle;
   const char * device = GOBI_DEVICENAME;
   char command[1024];
   BYTE inputMessage [1024], outputMessage[1024];
   ULONG inputLength = 1024, outputLength = 1024;
   ULONG ipAddress = 0, subnetMask = 0, gateway = 0, dns1 = 0, dns2 = 0;
   ULONG timeout = 300000, status = 0;
   ULONG services_count = 3;
   ULONG services [] =
   {
      QMIWDS,
      QMIDMS,
      QMINAS
   };

   // Connecting to Gobi device
   status = GobiConnect(device, &services_count, services, &handle);
   if(status != eGOBI_ERR_NONE)
   {
      TRACE("\nError while connecting to device : %lu\n", status);
      return -1;
   }

   //Fill up qmux buffer
   status = qmux_fill_buffer(&inputLength, inputMessage, sizeof(inputMessage));
   if(status != eGOBI_ERR_NONE)
   {
      TRACE("\nError while filling up qmux buffer : %lu\n", status);
      return -1;
   }

   // Binding the qmux port
   status = WDSMuxBinding(handle, timeout, inputLength, &inputMessage[0], &outputLength, outputMessage);
   if(status != eGOBI_ERR_NONE)
   {
      TRACE("\nError while binding up qmux port : %lu\n", status);
      return -1;
   }

   // Starting Network Interface
   outputLength = sizeof(outputMessage);
   status = WDSStartNetworkInterface (handle, timeout, 0, 0, &outputLength, outputMessage);
   if(status != eGOBI_ERR_NONE)
   {
      TRACE("\nError while starting network interface : %lu\n", status);
      return -1;
   }

   //Initialize usb0 interface
   system("uci set network.usb='interface'");
   configure_usb_interface((char *)"ifname", (char *)"usb0",0);
   configure_usb_interface((char *)"proto", (char *)"static",0);

   //Get IP Address
   outputLength = sizeof(outputMessage);
   status = PackGetIPAddress(&inputLength, inputMessage);
   if(status != eGOBI_ERR_NONE)
   {
      TRACE("\nError while getting IP Address : %lu\n", status);
      return -1;
   }

   //Get Current WDS Settings
   status = WDSGetCurrentSettings(handle, timeout, inputLength, &inputMessage[0], &outputLength, outputMessage);
   if(status != eGOBI_ERR_NONE)
   {
      TRACE("\nError while getting current settings : %lu\n", status);
      return -1;
   }

   //Parse IP Address
   status = ParseGetIPAddress(outputLength, outputMessage, &ipAddress);
   if(status != eGOBI_ERR_NONE)
   {
      TRACE("\nError while parsing IP Address : %lu\n", status);
      return -1;
   }
   sprintf(command,"%lu.%lu.%lu.%lu", EXTRACT_BYTE(ipAddress,3), EXTRACT_BYTE(ipAddress,2), \
   EXTRACT_BYTE(ipAddress,1), EXTRACT_BYTE(ipAddress, 0));
   configure_usb_interface((char *)"ipaddr", command, 0);
   printf("\nIPAddress : %s", command);
   memset(command, 0, sizeof(command));

   //Parse Subnet mask
   status = ParseGetSubnetMask(outputLength, outputMessage, &subnetMask);
   if(status != eGOBI_ERR_NONE)
   {
      TRACE("\nError while parsing Subnet Mask : %lu\n", status);
      return -1;
   }
   sprintf(command,"%lu.%lu.%lu.%lu", EXTRACT_BYTE(subnetMask,3), EXTRACT_BYTE(subnetMask,2), \
   EXTRACT_BYTE(subnetMask,1), EXTRACT_BYTE(subnetMask,0));
   configure_usb_interface((char *)"netmask", command, 0);
   printf("\nSubnetMask : %s", command);
   memset(command, 0, sizeof(command));

   //Parse Gateway
   status = ParseGetGateway(outputLength, outputMessage, &gateway);
   if(status != eGOBI_ERR_NONE)
   {
      TRACE("\nError while parsing Gateway : %lu\n", status);
      return -1;
   }
   sprintf(command,"%lu.%lu.%lu.%lu", EXTRACT_BYTE(gateway,3), EXTRACT_BYTE(gateway,2), \
   EXTRACT_BYTE(gateway,1), EXTRACT_BYTE(gateway,0));
   configure_usb_interface((char *)"gateway", command, 0);
   printf("\nGateway : %s", command);
   memset(command, 0, sizeof(command));

   //Parse primary and secondary DNS
   status = ParseGetDNSSettings(outputLength, outputMessage, &dns1, &dns2);
   if(status != eGOBI_ERR_NONE)
   {
      TRACE("\nError while parsing DNS Settings : %lu\n", status);
      return -1;
   }
   sprintf(command,"%lu.%lu.%lu.%lu", EXTRACT_BYTE(dns2,3), EXTRACT_BYTE(dns2,2), \
   EXTRACT_BYTE(dns2,1), EXTRACT_BYTE(dns2,0));
   configure_usb_interface((char *)"dns", command, 1);
   printf("\nSecondary dns : %s", command);
   memset(command, 0,sizeof(command));
   sprintf(command,"%lu.%lu.%lu.%lu", EXTRACT_BYTE(dns1,3), EXTRACT_BYTE(dns1,2), \
   EXTRACT_BYTE(dns1,1), EXTRACT_BYTE(dns1,0));
   configure_usb_interface((char *)"dns", command, 1);
   printf("\nPrimary dns : %s\n", command);
   memset(command, 0, sizeof(command));

   while (1)
   {

   }
}
