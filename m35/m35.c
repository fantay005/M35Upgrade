#include "m35.h"
#include "stm32f10x_usart.h"
#include "hardware.h"
#include "ringbuffer.h"

static char m_buf[128];
static RingBuffer m_ringBuffer;

#if defined(AUDIO_DEVICE)
#  define RESET_GPIO_GROUP GPIOA
#  define RESET_GPIO GPIO_Pin_11

#elif defined(AUDIO_DEVICE_V2) || defined (__SPEAKER_V3__)
#  define RESET_GPIO_GROUP GPIOG
#  define RESET_GPIO GPIO_Pin_10

#elif defined(LED_DEVICE)
#  define RESET_GPIO_GROUP GPIOB
#  define RESET_GPIO GPIO_Pin_1

#endif

#if defined (__SPEAKER_V3__)
#define __gsmPowerSupplyOn()         GPIO_ResetBits(GPIOB, GPIO_Pin_0)
#define __gsmPowerSupplyOff()        GPIO_SetBits(GPIOB, GPIO_Pin_0)

#else
#define __gsmPowerSupplyOn()         GPIO_SetBits(GPIOB, GPIO_Pin_0)
#define __gsmPowerSupplyOff()        GPIO_ResetBits(GPIOB, GPIO_Pin_0)

#endif

bool m35Startup(void)
{
	dprintf("m35Startup -->\n");
	ringBufferInit(&m_ringBuffer, m_buf, sizeof(m_buf));

	
	__gsmPowerSupplyOff();                //PORTB.0  //关手机模块电源
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);  //RTS  -----I/O口全部拉低
	
	GPIO_ResetBits(RESET_GPIO_GROUP, RESET_GPIO);  //RADIO   RESET-----I/O口全部拉低
	
	GPIO_ResetBits(GPIOA, GPIO_Pin_2);   //USART2_TX  -----I/O口全部拉低
	GPIO_ResetBits(GPIOA, GPIO_Pin_3);   //USART2_RX-----I/O口全部拉低

	do {

  	delayMs(1000);
  	//RESET-----拉高 实现手机可靠复位
    GPIO_ResetBits(RESET_GPIO_GROUP, RESET_GPIO);
	GPIO_SetBits(GPIOA, GPIO_Pin_12);    //RTS  -----拉高
    __gsmPowerSupplyOn();     //PORTB.0  //开手机模块电源
    delayMs(1000);

    //RESET-----拉高 实现手机可靠复位
    GPIO_SetBits(RESET_GPIO_GROUP, RESET_GPIO);
	delayMs(2000);
    //RESET-----拉高 实现手机可靠复位
    GPIO_ResetBits(RESET_GPIO_GROUP, RESET_GPIO);
		
    delayMs(2000);
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == Bit_SET) {
    	dprintf("  start ok\n");
    	break;
    }
	
	dprintf("  start failed, try agin\n");

    delayMs(4000);

	} while (1);

	return true;
}

bool m35Stop(void)
{
	return true;
}


void USART2_IRQHandler(void)
{
	if (USART2->SR & USART_FLAG_RXNE) {
		unsigned char dat = USART2->DR;
		ringBufferAppendByte(&m_ringBuffer, dat);
//		USART1->DR = dat; 
	}
}

void m35SerialSendByte(char ch)
{
	if (ch == '\n') m35SerialSendByte('\r');

    USART_SendData(USART2, ch);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

void m35SerialSendbuff(unsigned char Num, unsigned char *ch)
{
	unsigned char i;
	for(i=0;i<Num;i++) {
		USART_SendData(USART2, *(ch+i));
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	}
}


bool m35SerialIsGotByte(void)
{
//	return (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != 0);
	return !(ringBufferIsEmpty(&m_ringBuffer));
}

char m35SerialGetByte(void)
{
//	return USART_ReceiveData(USART2);
	return ringBufferGetByte(&m_ringBuffer);
}

