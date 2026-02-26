#ifndef __APP_BOOTLOADER_H
#define __APP_BOOTLOADER_H

#include "bootloader.h"

#define APP_START_RX_BUFF_LEN 64

//初始化打印日志
void App_bootloader_init(void);

//接收数据
void App_bootloader_rx_data(void);

//传输完成校验
void App_bootloader_check_data(void);

//跳转APP
void App_bootloader_jump_app(void);

void App_bootloader(void);
#endif

