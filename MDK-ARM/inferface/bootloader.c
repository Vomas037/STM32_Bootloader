#include "bootloader.h"

uint8_t uart_rx_buff[BOOTLOADER_UART_RX_BUFF_LEN] = {0};
uint16_t uart_rx_len = 0;
volatile uint16_t total_len = 0; // 实际定义并初始化
volatile uint8_t rx_flag = 0;

uint32_t flash_write_offset = 0;
uint8_t last_byte_flag = 0;
uint8_t last_byte = 0;

volatile uint32_t last_rx_time = 0;


/**
 * 串口空闲中断回调
*/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
    {
        last_rx_time = HAL_GetTick();
        uart_rx_len = Size;
        total_len += Size;
        rx_flag = 1;
        HAL_FLASH_Unlock();

        uint16_t i = 0;

        // 1. 修正后的擦除逻辑：仅在页面起始地址处擦除
        //uint32_t current_addr = APP_START_ADDR + flash_write_offset;

        // 2. 预处理：如果有上次遗留的字节，先和本次第0个字节拼成 HalfWord 写入
        if (last_byte_flag)
        {
            uint16_t data16 = last_byte | (uart_rx_buff[0] << 8);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_START_ADDR + flash_write_offset, data16);
            flash_write_offset += 2;
            i = 1;              // 标记：本次 buffer 从索引 1 开始处理
            last_byte_flag = 0; // 遗留已处理
        }

        // 3. 核心循环：处理当前 buffer 中剩余的完整 HalfWord
        for (; i + 1 < uart_rx_len; i += 2)
        {
            uint16_t data16 = uart_rx_buff[i] | (uart_rx_buff[i + 1] << 8);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_START_ADDR + flash_write_offset, data16);
            flash_write_offset += 2;
        }

        // 4. 收尾：如果本次还剩下一个奇数字节，存起来等下次
        if (i < uart_rx_len)
        {
            last_byte = uart_rx_buff[i];
            last_byte_flag = 1;
        }

        HAL_FLASH_Lock();

        // 5. 重新开启接收
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rx_buff, BOOTLOADER_UART_RX_BUFF_LEN);
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    }
}

/**
 * 错误处理
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uint32_t isrflags = READ_REG(huart->Instance->SR);
        if (isrflags & UART_FLAG_ORE)
        {
            __HAL_UART_CLEAR_OREFLAG(huart);
        }
        // 发生错误后重启接收
        Bootloader_rx_app_init();
    }
}

/**
 * 擦除flash
 */
void Bootloader_erase_flash(uint32_t page_addr, uint16_t num_pages)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase_init;
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.Banks = FLASH_BANK_1;
    erase_init.PageAddress = page_addr;
    erase_init.NbPages = num_pages;
    uint32_t page_error;
    HAL_FLASHEx_Erase(&erase_init, &page_error);

    HAL_FLASH_Lock();
}

/**
 * 串口接收
 */
void Bootloader_rx_app_init(void)
{
    // 1. 清除可能存在的错误标志（如 ORE 溢出）
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);

    // 2. 开启 DMA 空闲中断接收
    // 该函数会同时开启 DMA 全满中断和串口空闲中断
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rx_buff, BOOTLOADER_UART_RX_BUFF_LEN);

    // 3. 关键：手动禁用 DMA 的半传输中断 (HT)
    // 这样 256 字节时不会因为 HT 触发回调，只有 IDLE 或 512 字节满时才触发
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
}

/**
 * 跳转到APP
 */

uint8_t Bootloader_jump_to_app(uint32_t start_addr)
{
    typedef void (*pFunc)(void);

    // 1.校验
    uint32_t app_stack_ptr = *(volatile uint32_t *)(start_addr);
    uint32_t app_reset_handle = *(volatile uint32_t *)(start_addr + 4);

    // 1.校验栈顶地址
    if ((app_stack_ptr & 0xFFFF0000) != STACK_ADDR)
    {
        printf("stack handle error\n");
        return 1;
    }

    // 2.校验复位中断地址
    if (app_reset_handle < start_addr || app_reset_handle > APP_END_ADDR)
    {
        printf("reset handle error\n");
        return 1;
    }

    // 注销bootloader程序
    __NVIC_DisableIRQ(USART1_IRQn);
    __NVIC_DisableIRQ(DMA1_Channel5_IRQn);

    // 关闭全局中断
    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // 注销hal库设置-注销外设的配置
    HAL_DeInit();

    // 设置堆栈指针
    __set_MSP(app_stack_ptr);

    // 重定向中断向量表
    SCB->VTOR = start_addr;

    // 跳转到APP复位中断
    pFunc jump_to_app = (pFunc)app_reset_handle;
    jump_to_app();

    return 0;
}
