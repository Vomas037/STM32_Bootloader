#ifndef __W25Q128_H
#define __W25Q128_H

#include "spi.h"
#include "gpio.h"

#define W25Q_PAGE_SIZE    256
#define W25Q_SECTOR_SIZE  4096
#define W25Q_BLOCK_SIZE   65536

#define W25Q128_READ_ID             0x9F
#define W25Q128_WRITE_ENABLE        0x06
#define W25Q128_WRITE_DISABLE       0x04
#define W25Q128_READ_STATUS_REG     0x05
#define W25Q128_READ_DATA           0x03
#define W25Q128_PAGE_PROGRAM        0x02
#define W25Q128_ERASE_SECTOR        0x20


#define W25Q128_DUMMY_BYTE          0xFF


void w25q128_start(void);
void w25q128_stop(void);
void w25q128_write_byte(uint8_t data);
uint8_t w25q128_read_byte(void);
void w25q128_read_id(uint8_t* mf_id, uint16_t* dev_id);
void w25q128_read_data(uint8_t block, uint8_t sector, uint8_t page, uint8_t addr, uint8_t* data, uint16_t len);
void w25q128_write_data(uint8_t block, uint8_t sector, uint8_t page, uint8_t addr, uint8_t* data, uint16_t len);
void w25q128_erase_sector(uint8_t block, uint8_t sector);


#endif
