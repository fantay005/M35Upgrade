#include "stm32f10x_usart.h"

void usart1Configure(void)
{
	USART_InitTypeDef USART_InitStructure;
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);       // Configure USART1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  // Enable USART1 Receive
	USART_Cmd(USART1, ENABLE);                    // Enable the USART1
	USART_SendData(USART1, 'X');
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void usart2Configure(int baud){
    USART_InitTypeDef USART_InitStructure;
	 	
	USART_InitStructure.USART_BaudRate = baud;
	//USART_InitStructure.USART_BaudRate = 9600;      //和手机模块通讯
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);       // Configure USART
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);  // Enable USART1 Receive
	USART_Cmd(USART2, ENABLE);

}
