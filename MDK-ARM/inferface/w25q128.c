#include "w25q128.h"

/**
 * 片选
 */
void w25q128_start(void)
{
    HAL_GPIO_WritePin(W25Q128_CS_GPIO_Port, W25Q128_CS_Pin, GPIO_PIN_RESET);
}

/**
 * 不片选
 */
void w25q128_stop(void)
{
    HAL_GPIO_WritePin(W25Q128_CS_GPIO_Port, W25Q128_CS_Pin, GPIO_PIN_SET);
}

/**
 * 写入一个字节
 */
void w25q128_write_byte(uint8_t data)
{
    HAL_SPI_Transmit(&hspi1, &data, 1, 100);
}

/**
 * 读取一个字节
 */
uint8_t w25q128_read_byte(void)
{
    uint8_t data;
    HAL_SPI_Receive(&hspi1, &data, 1, 100);
    return data;
}

/**
 * 交换一个字节数据
 */
uint8_t W25q128_Spi(uint8_t data)
{
    uint8_t ret;
    HAL_SPI_TransmitReceive(&hspi1, &data, &ret, 1, 100);
    return ret;
}

/**
 * 读取id（测试）
 */
void w25q128_read_id(uint8_t *mf_id, uint16_t *dev_id)
{
    w25q128_start();

    w25q128_write_byte(W25Q128_READ_ID);

    *mf_id = w25q128_read_byte();

    uint8_t high = w25q128_read_byte();
    uint8_t low = w25q128_read_byte();
    *dev_id = high << 8 | low;

    w25q128_stop();
}



/**
 * 等待忙状态
 */
static void w25q128_wait_busy(void)
{
    w25q128_start();

    while (1)
    {
        w25q128_write_byte(W25Q128_READ_STATUS_REG);
        uint8_t status = w25q128_read_byte();
        if ((status & 0x01) == 0)
        {
            break;
        }
    }

    w25q128_stop();
}

/**
 * 写使能
*/
static void w25q128_write_enable(void)
{
    w25q128_start();
    
    w25q128_write_byte(W25Q128_WRITE_ENABLE);

    w25q128_stop();
}

/**
 * 读取flash数据
 * addr 0x  00     0     0    00 -> 0xFF FF FF 
 *         block sector page addr
*/
void w25q128_read_data(uint8_t block, uint8_t sector, uint8_t page, uint8_t addr, uint8_t* data, uint16_t len)
{
    w25q128_wait_busy();

    w25q128_write_enable();

    w25q128_start();

    w25q128_write_byte(W25Q128_READ_DATA);
    uint32_t addr_24 = (block * W25Q_BLOCK_SIZE) + (sector * W25Q_SECTOR_SIZE) + (page * W25Q_PAGE_SIZE) + addr;
    w25q128_write_byte(addr_24 >> 16);
    w25q128_write_byte(addr_24 >> 8);
    w25q128_write_byte(addr_24);

    for(uint16_t i = 0; i < len; i++){
        data[i] = w25q128_read_byte();
    }

    w25q128_stop();
}

void w25q128_write_data(uint8_t block, uint8_t sector, uint8_t page, uint8_t addr, uint8_t* data, uint16_t len)
{
    w25q128_wait_busy();

    w25q128_write_enable();

    w25q128_start();

    w25q128_write_byte(W25Q128_PAGE_PROGRAM);
    uint32_t addr_24 = (block * W25Q_BLOCK_SIZE) + (sector * W25Q_SECTOR_SIZE) + (page * W25Q_PAGE_SIZE) + addr;
    w25q128_write_byte(addr_24 >> 16);
    w25q128_write_byte(addr_24 >> 8);
    w25q128_write_byte(addr_24);
    for(uint16_t i = 0; i < len; i++){
        w25q128_write_byte(data[i]);
    }

    w25q128_stop();    
}


/**
 * 擦除一扇内容
*/
void w25q128_erase_sector(uint8_t block, uint8_t sector)
{
    w25q128_wait_busy();

    w25q128_write_enable();

    w25q128_start();

    w25q128_write_byte(W25Q128_ERASE_SECTOR);
    uint32_t addr_24 = (block * W25Q_BLOCK_SIZE) + (sector * W25Q_SECTOR_SIZE);
    w25q128_write_byte(addr_24 >> 16);
    w25q128_write_byte(addr_24 >> 8);
    w25q128_write_byte(addr_24);
    

    w25q128_stop(); 
}
