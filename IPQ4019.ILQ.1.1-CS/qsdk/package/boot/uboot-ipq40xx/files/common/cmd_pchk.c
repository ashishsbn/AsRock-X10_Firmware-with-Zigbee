#include <common.h>
#include <config.h>
#include <command.h>
#include <u-boot/md5.h>
typedef struct _PEGA_IMAGE_HEADER_
{
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  uint32_t len;
}PEGA_IMG_HEADER;

typedef struct _TLV_
{
  uint16_t tag;
  uint16_t len;
}TLV_ITEM;

typedef struct _TLV_DATA_
{
  TLV_ITEM item;
  char* data;
}TLV_ITEM_DATA;

int dumpimage(char* pBuff,PEGA_IMG_HEADER* hdr,off_t size){
  TLV_ITEM_DATA itemdata;
  char* pAddr = NULL;
  char* pHeadEndAddr = NULL;
  char* pItemData = NULL;
  char* tmp = NULL;
  char checkfilecmd[100];
  char md5sum[100];
  char calcmd5sum[33];
  char digest[16];
  char newAddr[9];
  off_t len;
  unsigned long addr,length=0;
  unsigned int i;
  u8 output[16];
  pAddr = pBuff;
  memset(calcmd5sum,0,sizeof(calcmd5sum)/sizeof(char));
  /*check magic number*/
  memcpy(hdr,pAddr,sizeof(PEGA_IMG_HEADER));
  pHeadEndAddr = pAddr + hdr->len;
  pAddr += sizeof(PEGA_IMG_HEADER);
  if (hdr->magic != 0x50454741){
    printf("Not correct image\n");
    return 1;
  }
  printf("MAGIC: %x\n",hdr->magic);
  printf("Version: %d\n",hdr->version);
  printf("Reserved: %d\n",hdr->reserved);
  printf("Head Len: %d\n",hdr->len);
  /* get TLV value */
  while(pAddr < pHeadEndAddr){
    pItemData = NULL;
    memcpy(&itemdata.item,pAddr,sizeof(TLV_ITEM));
    pAddr += sizeof(TLV_ITEM);
    printf("TAG : %d\n",itemdata.item.tag);
    printf("LEN : %d\n",itemdata.item.len);
    len = itemdata.item.len;
    pItemData = malloc(sizeof(char)*(len+1));
    memset(pItemData,0,len+1);
    memcpy(pItemData,pAddr,len);
    pAddr += len;
    if (len == 1)
      printf("Data : 0x%x\n",pItemData[0]);
    else if (len > 1)
      printf("Data : [%s]\n",pItemData);
    else
      printf("Data : ERROR\n");
    /* get md5sum */
    if (itemdata.item.tag == 4){
      memset(md5sum,0,sizeof(md5sum));
      strncpy(md5sum,pItemData,itemdata.item.len);
      printf("MD5 : %s\n",md5sum);
    }
    if (pItemData)
      free(pItemData);
  }

   if ((tmp = getenv ("filesize")) != NULL) {
     length = simple_strtoul (tmp, NULL, 16)-hdr->len;
   }

   addr = &pHeadEndAddr[0];
   md5_wd((unsigned char *) addr, length, output, CHUNKSZ_MD5);
   printf("md5 for %08lx ... %08lx ==> \n", addr, addr + length - 1);
   for (i = 0; i < 16; ++i){
     snprintf(&calcmd5sum[i*2],16*2,"%02x", output[i]);
   }
   printf("calcMD5 : %s\n",calcmd5sum);
     
   if (memcmp(calcmd5sum,md5sum,16*2) == 0){
     printf("Check passed\n");
     memcpy(pBuff,addr,length);
   }
   else{
     printf("Check failed\n");
     return 1;
   }
  return 0;
}


int do_pcheck (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  ulong		addr;
  PEGA_IMG_HEADER hdr;
  if (argc > 1) {
    addr = simple_strtoul(argv[1], NULL, 16);
  }
  return dumpimage(addr,&hdr,0);
}

U_BOOT_CMD(
	pchk, 2, 1, do_pcheck,
	"pchk\t- [addr]",
	"pchk [addr]\n"
);
