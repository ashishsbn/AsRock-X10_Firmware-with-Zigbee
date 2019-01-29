/*
 * (C) Copyright 2010
 * Jason Kridner <jkridner@beagleboard.org>
 *
 * Based on cmd_led.c patch from:
 * http://www.mail-archive.com/u-boot@lists.denx.de/msg06873.html
 * (C) Copyright 2008
 * Ulf Samuelsson <ulf.samuelsson@atmel.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <asm/arch-qcom-common/gpio.h>


int do_button (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i, match = 0;
	int isButtonPressed = 0;
	/* Validate arguments */
	if ((argc != 2)) {
		return CMD_RET_USAGE;
	}


	if ((strcmp("reset", argv[1]) == 0)) {
	    match = 1;
	    while (!isButtonPressed && !tstc())
	      {
		if (gpio_get_value(64) == 0) {
		  printf("reset button is pressed\n");
		    isButtonPressed = 1;
		    }
		udelay(500000);
	      }
	  }
	if ((strcmp("wps", argv[1]) == 0)) {
	    match = 1;
	    while (!isButtonPressed && !tstc())
	      {
		if (gpio_get_value(63) == 0) {
		  printf("wps button is pressed\n");
		    isButtonPressed = 1;
		    }
		udelay(500000);
	      }	    
	  }

	/* If we ran out of matches, print Usage */
	if (!match) {
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	button, 2, 1, do_button,
	"button\t- [reset|wps",
	"button [name]\n"
);
