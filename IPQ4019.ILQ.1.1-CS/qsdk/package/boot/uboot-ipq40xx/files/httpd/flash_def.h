#ifndef _FLASH_DEF_H_
#define _FLASH_DEF_H_


/*-----------------------------------------------------------------------
 * FLASH Info: contains chip specific data, per FLASH bank
 */
typedef struct {
	ulong	size;							/* total bank size in bytes		*/
	ulong	sector_size;					/* size of erase unit in bytes */
	ushort	sector_count;					/* number of erase units		*/
	ulong	flash_id;						/* combined device & manufacturer code	*/
  /*ulong	start[CFG_MAX_FLASH_SECT];*/		/* physical sector start addresses */
} flash_info_t;

#endif
