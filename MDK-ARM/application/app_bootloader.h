#ifndef __APP_BOOTLOADER_H
#define __APP_BOOTLOADER_H

#include "bootloader.h"
#include "backup.h"
#include "usart.h"

typedef enum{
    BOOTLOADER_UPDATE,
    BOOTLOADER_NO_UPDATE,
    BOOTLOADER_RESET
} Bootloader_Update_Status;

typedef enum
{
    BOOTLOADER_STATUS_INIT,
    BOOTLOADER_STATUS_START,
    BOOTLOADER_STATUS_RUN,
    BOOTLOADER_STATUS_RX_DATA,
    BOOTLOADER_STATUS_CHECK_DATA,
    BOOTLOADER_STATUS_JUMP_APP
} Bootloader_Status;

#define APP_START_RX_BUFF_LEN 64

//魔术数字 更新标志位
#define UPDATE_FLAG_MAGIC  0x5A5A

//初始化打印日志
void App_bootloader_init(void);

//接收数据
void App_bootloader_rx_data(void);

//传输完成校验
void App_bootloader_check_data(void);

//跳转APP
void App_bootloader_jump_app(void);

void App_bootloader(void);

void App_Bootloader_CheckUpdate(void);

void App_bootloader_CheckReset(void);
#endif

