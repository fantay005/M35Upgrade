#include "stm32f10x_nvic.h"

extern const unsigned int __Vectors;
void nvicConfigure(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_SetVectorTable((u32)&__Vectors, 0x0);
	/* Configure the NVIC Preemption Priority Bits */
	//----------------------------------------------------------------------------
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	//----------------------------------------------------------------------------
	/* Enable the USART2 Interrupt */
 	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQChannel;
 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
 	NVIC_Init(&NVIC_InitStructure);
	//----------------------------------------------------------------------------
// 	/* Enable the RTC Interrupt */
// 	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQChannel;
// 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
// 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
// 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
// 	NVIC_Init(&NVIC_InitStructure);
	//----------------------------------------------------------------------------
	NVIC_SystemHandlerPriorityConfig(SystemHandler_SysTick, 0, 0);
	
}
