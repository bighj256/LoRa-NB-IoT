#include "led.h"

/**
 * @brief       LED初始化
 * @param       无
 * @retval      无
 */
void led_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef gpio;

    gpio.GPIO_Pin = GPIO_Pin_13;            // 连接 LED 的引脚
    gpio.GPIO_Speed = GPIO_Speed_2MHz;      // 输出速度
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;      // 开漏输出
    /* 先设置关闭电平，再启用输出，避免初始化时短暂点亮。 */
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
    GPIO_Init(GPIOC, &gpio);
}

/**
 * @brief       LED0闪烁（无限循环）
 * @param       无
 * @retval      无
 */
void led0_toggle(void)
{
    while(1)
    {
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
        delay_ms(200);
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
        delay_ms(200);
    }
}

void led_set(uint8_t on)
{
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, on ? Bit_RESET : Bit_SET);
}
