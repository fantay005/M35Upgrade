#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <stdbool.h>
#include <stdio.h>

#include "fsmc_nor.h"

extern void rccConfigure(void);
extern void nvicConfigure(void);
extern void gpioConfigure(void);
extern void usart1Configure(void);
extern void usart2Configure(int baud);
extern void sysTickConfiguration(void);
extern void delayMs(unsigned int ms);
void setDelayMs(unsigned int ms);
bool isTimeout(void);
extern void iwdgConfigure(void);

inline void hardwareInit(void)
{
	rccConfigure();
	nvicConfigure();
	gpioConfigure();
	usart1Configure();
	sysTickConfiguration();
	
	FSMC_NOR_Init();
	
	
}

#ifdef __DEBUG__
#define dprintf(fmt, args ...) printf(fmt, ## args)
#else
#define dprintf(fmt, args ...)
#endif

#define eprintf(fmt, args ...) printf(fmt, ## args)

#endif
