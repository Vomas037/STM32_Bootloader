#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#include "usart.h"

#define BOOTLOADER_UART_RX_BUFF_LEN 512

//程序写入的起始位置
#define APP_START_ADDR  0x08004000
#define STACK_ADDR 0x20000000
#define APP_END_ADDR 0x08040000

extern volatile uint16_t total_len; 
extern volatile uint8_t rx_flag;
extern volatile uint32_t last_rx_time;
/**
 * 串口接收
*/
void Bootloader_rx_app_init(void);
uint8_t Bootloader_jump_to_app(void);
void Bootloader_erase_flash(uint32_t page_addr, uint16_t num_pages);

#endif


