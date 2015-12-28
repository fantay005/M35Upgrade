#include <stdbool.h>
#include "stm32f10x_systick.h"
#include "stm32f10x_iwdg.h"

static volatile unsigned int timeDelay = 0;

void sysTickConfiguration(void)   //1msÖÐ¶ÏÒ»´Î
{
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
  SysTick_SetReload(72000);
  SysTick_CounterCmd(SysTick_Counter_Enable);
  SysTick_ITConfig(ENABLE);
}

void setDelayMs(unsigned int ms)
{
	timeDelay = (ms == 0xFFFFFFFF) ? 0xFFFFFFFF : (ms + 1);
}

void SysTick_Handler(void)
{
	if(timeDelay > 0) timeDelay--;
}

void delayMs(unsigned int ms)
{
	setDelayMs(ms);
	while (timeDelay != 0) {
		if(timeDelay%1024 == 0)
			IWDG_ReloadCounter();//Î¹¹·
	}
}

bool isTimeout(void)
{
	return (timeDelay == 0);
}
