
/* Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifndef __GIGA_SPI_NAND_H
#define __GIGA__SPI_NAND_H

#include "mt29f_spinand.h"

void gigadevice_set_defaults(struct spi_device *spi_nand);

void gigadevice_read_cmd(struct spinand_cmd *cmd, u32 page_id);

void gigadevice_read_data(struct spinand_cmd *cmd, u16 column, u16 page_id);

void gigadevice_write_cmd(struct spinand_cmd *cmd, u32 column);

void gigadevice_write_data(struct spinand_cmd *cmd, u16 column, u16 page_id);

void gigadevice_erase_blk(struct spinand_cmd *cmd, u32 page_id);

int gigadevice_parse_id(struct spi_device *spi_nand, u8 *nand_id, u8 *id);

int gigadevice_verify_ecc(u8 status);

int dummy_verify_ecc(u8 status);

void macronix_read_data(struct spinand_cmd *cmd, u16 column, u16 page_id);

void macronix_write_data(struct spinand_cmd *cmd, u16 column, u16 page_id);

int macronix_parse_id(struct spi_device *spi_nand, u8 *nand_id, u8 *id);

int macronix_verify_ecc(u8 status);

void winbond_read_data(struct spinand_cmd *cmd, u16 column, u16 page_id);

void winbond_write_data(struct spinand_cmd *cmd, u16 column, u16 page_id);

int winbond_parse_id(struct spi_device *spi_nand, u8 *nand_id, u8 *id);

/* Macronix Specfic defines */
#define MACRONIX_NORM_RW_MASK	0x0F
#endif /* __GIGA_SPI_NAND_H */
