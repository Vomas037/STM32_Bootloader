#include "app_bootloader.h"
#include "stdlib.h"

// 预计接收
uint8_t app_rx_start_buff[APP_START_RX_BUFF_LEN] = {0};
uint16_t app_rx_start_len = 0;

// 确认接收完毕标志
uint8_t rx_done_flag = 0;

// 总的接收的长度
uint32_t app_rx_total_len = 0;

// 当前状态
Bootloader_Status boot_status = BOOTLOADER_STATUS_INIT;

// 更新状态
Bootloader_Update_Status boot_update_status = BOOTLOADER_NO_UPDATE;

/*按键中断回调函数*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == KEY1_Pin)
    {
        boot_update_status = BOOTLOADER_RESET;
        // printf("reset..\r\n");
    }
}

/**
 * 初始化打印日志
 */
void App_bootloader_init(void)
{
    printf("bootloader init\r\n");
    printf("wait for start...\r\n");
    boot_status = BOOTLOADER_STATUS_START;
}

void App_bootloader_start(void)
{
    memset(app_rx_start_buff, 0, APP_START_RX_BUFF_LEN);
    HAL_UARTEx_ReceiveToIdle(&huart1, app_rx_start_buff, APP_START_RX_BUFF_LEN, &app_rx_start_len, 0xffffff);
    if (app_rx_start_len > 0)
    {
        char *start_str = strstr((char *)app_rx_start_buff, "start:");
        if (start_str != NULL)
        {
            char *end_ptr;
            uint32_t parsed_len = strtoul(start_str + 6, &end_ptr, 10);
            // 校验：如果 endptr 依然指向 start_str + 6，说明后面根本没数字
            if (end_ptr == (start_str + 6))
            {
                printf("Invalid length format\r\n");
            }
            else
            {
                app_rx_total_len = parsed_len;
                printf("len:%d\r\n", app_rx_total_len);
                boot_status = BOOTLOADER_STATUS_RUN;
            }
        }
        else
        {
            printf("data error\r\n");
        }
    }
}

// 接收数据
void App_bootloader_rx_data(void)
{
    // 1.软件方式：等待两秒钟
    if ((HAL_GetTick() - last_rx_time > 2000) && (rx_flag == 1))
    {
        boot_status = BOOTLOADER_STATUS_CHECK_DATA;
        rx_flag = 0;
        printf("receive data ok\r\n");
    }
    // 2.硬件方式 外部按键中断
}

// 传输完成校验
void App_bootloader_check_data(void)
{
    if (app_rx_total_len == total_len)
    {
        boot_status = BOOTLOADER_STATUS_JUMP_APP;
        printf("check ok\r\n");
    }
    else
    {
        printf("check error\r\n");
        NVIC_SystemReset();
    }
}

void App_bootloader(void)
{
    switch (boot_status)
    {
    case BOOTLOADER_STATUS_INIT:
        App_bootloader_init();
        break;

    case BOOTLOADER_STATUS_START:
        App_bootloader_start();
        break;

    case BOOTLOADER_STATUS_RUN:
        Bootloader_erase_flash(APP_START_ADDR, 10);
        printf("flash erase ok\r\n");
        Bootloader_rx_app_init();
        printf("ready to receive app\r\n");
        rx_done_flag = 0;
        boot_status = BOOTLOADER_STATUS_RX_DATA;
        break;

    case BOOTLOADER_STATUS_RX_DATA:
        App_bootloader_rx_data();
        break;

    case BOOTLOADER_STATUS_CHECK_DATA:
        App_bootloader_check_data();
        break;

    case BOOTLOADER_STATUS_JUMP_APP:
        App_bootloader_jump_app();
        break;

    default:
        break;
    }
}

/**
 * 检查更新标志
 */
void App_Bootloader_CheckUpdate(void)
{
    uint16_t update_flag = 0;
    uint8_t is_soft_reset = 0;
    // 首先判断是不是冷启动
    // 通过判断软件复位标志位是否被置1
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) != RESET)
    {
        is_soft_reset = 1;
    }

    // 读取BKP寄存器获取标志位
    update_flag = BKP_ReadFlag();

    if (boot_update_status == BOOTLOADER_RESET)
    {
        printf("Strat reset..\r\n");
    }
    else
    {
        if (is_soft_reset)
        {
            if (update_flag == UPDATE_FLAG_MAGIC)
            {
                printf("Start updating...\r\n");
                boot_update_status = BOOTLOADER_UPDATE;
                BKP_WriteFlag(0x0000);
                __HAL_RCC_CLEAR_RESET_FLAGS(); // 清除复位标志位,防止意外重启再次进入
            }
            else
            {
                printf("Invaild flag, jump to app...\r\n");
                // boot_update_status = BOOTLOADER_NO_UPDATE;
                //  BKP_WriteFlag(UPDATE_FLAG_MAGIC);
                //  NVIC_SystemReset();
            }
        }
        else
        {
            printf("Cold boot, jump to app...\r\n");
            // boot_update_status = BOOTLOADER_NO_UPDATE;
            //  BKP_WriteFlag(UPDATE_FLAG_MAGIC);
            //  BKP_WriteFlag(0x0000);
            //  NVIC_SystemReset();
        }
    }
    App_bootloader_jump_app();
}

void App_bootloader_CheckReset(void)
{
    HAL_Delay(3000);
}

// 跳转APP
void App_bootloader_jump_app(void)
{
    if (boot_update_status == BOOTLOADER_RESET)
    {
        // 跳转到初始化程序
        Bootloader_jump_to_app(RESET_START_ADDR);
    }
    else
    {
        // 跳转到APP程序
        Bootloader_jump_to_app(APP_START_ADDR);
    }
}
