#include "backup.h"


/**
 *  使能BKP区域访问
 *  enable the power and backup interface clocks by setting the PWREN and BKPEN bits 
    in the RCC_APB1ENR register
    set the DBP bit the Power Control Register (PWR_CR) to enable access to the Backup 
    registers and RTC.
*/
void BKP_EnableAccess(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();     //使能PWR时钟
    __HAL_RCC_BKP_CLK_ENABLE();     //使能BKP时钟
    HAL_PWR_EnableBkUpAccess();
}

/**
 *  写入更新标志位
*/
void BKP_WriteFlag(uint16_t flag)
{
    BKP_EnableAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, flag);
}


/**
 * 读取更新标志位
 * 16位寄存器，一次读取半字
*/
uint16_t BKP_ReadFlag(void)
{
    BKP_EnableAccess();
    return (uint16_t)HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
}
