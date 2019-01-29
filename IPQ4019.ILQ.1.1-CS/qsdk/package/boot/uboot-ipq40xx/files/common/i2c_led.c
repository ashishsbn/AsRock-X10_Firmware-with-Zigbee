#include "../include/i2c_led.h"
#include <common.h>
//PEGA<<BEGIN
int led_ctl(char R,char G,char B)
{
	uchar	chip;
	ulong	addr;
	uint	alen;
	int	count;
	int     chksum = 0;
	uchar LED[11]={0x05,0x00,0x12,0x00,0x22,0x11,0x7D,0x33,0xF0,0xF0};
	LED[4]=G;
	LED[5]=R;
	LED[7]=B;
	chksum = LED[0] + LED[1] + LED[2] + LED[3] + LED[4] + LED[5] + LED[7];
	LED[6]=chksum & 0xFF;
	/*
	 * Chip is always specified.
	 */
	chip = 80;
	/*
	 * Address is always specified.
	 */
	addr = 0xFF61;
	alen = 2;

	count = sizeof(LED)/sizeof(uchar);
	if (i2c_write(chip, 0xFF61, 2, (uchar*)&LED, count) != 0)
	  puts("Error writing the chip.\n");

	return (0);
}
//PEGA>>END
