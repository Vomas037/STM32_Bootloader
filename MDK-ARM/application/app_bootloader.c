#include "app_bootloader.h"
#include "stdlib.h"

typedef enum
{
    BOOTLOADER_STATUS_INIT,
    BOOTLOADER_STATUS_START,
    BOOTLOADER_STATUS_RUN,
    BOOTLOADER_STATUS_RX_DATA,
    BOOTLOADER_STATUS_CHECK_DATA,
    BOOTLOADER_STATUS_JUMP_APP
} Bootloader_status;

//预计接收
uint8_t app_rx_start_buff[APP_START_RX_BUFF_LEN] = {0};
uint16_t app_rx_start_len = 0;

//确认接收完毕标志
uint8_t rx_done_flag = 0;

//总的接收的长度
uint32_t app_rx_total_len = 0;

//当前状态
Bootloader_status boot_status = BOOTLOADER_STATUS_INIT;

/*按键中断回调函数*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == KEY1_Pin)
    {
        rx_done_flag = 1;
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
    //1.软件方式：等待两秒钟
    // if((HAL_GetTick() - last_rx_time > 2000) && (rx_flag == 1))
    // {
    //     boot_status = BOOTLOADER_STATUS_CHECK_DATA;
    //     rx_flag = 0;
    //     printf("receive data ok\r\n");
    // }
    //2.硬件方式
    if(rx_done_flag){
        boot_status = BOOTLOADER_STATUS_CHECK_DATA;
        printf("receive data ok\r\n");
    }
}

// 传输完成校验
void App_bootloader_check_data(void)
{
    if(app_rx_total_len == total_len)
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

// 跳转APP
void App_bootloader_jump_app(void)
{
    Bootloader_jump_to_app();
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
