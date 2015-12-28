//说明：升级的一些标志和升级的参数都放在内部flash的最后一页里首地址是0x0807F800
//倒数第二页的用于存放升级失败的次数，次数大于3次时此次升级失败，擦除标志，地址是0x0807F000
//内部flash每一页的大小为2K字节
//0-64K存放bootloader程序65-508K存放应用程序

#include <stdio.h>
#include <string.h>

#include "hardware/hardware.h"
#include "firmware.h"
#include "m35.h"
#include "m35at.h"


void runApp(unsigned int addr);
void resetBoard(void);
void IncreaseUpdateTimes(void);

int main(void)
{
	bool rc;
	rccConfigure();
	nvicConfigure();
	gpioConfigure();
	usart1Configure();
	sysTickConfiguration();	
	IWDG_ReloadCounter();//喂狗

	if (isNeedDownloadFirmware()) {
		if (!m35Startup()) {
			dprintf("m35Startup return false\n");
			resetBoard();
		}
		if (!m35Init()) {
			dprintf("m35Init return false\n");
			resetBoard();
		}
		if (!downloadFirmwareToRam()) {
			dprintf("downloadFirmware return false\n");
			resetBoard();
		}

		rc = programFromRam();
		
		if (!m35Stop()) {
			dprintf("m35Stop return false\n");
			resetBoard();
		}
		
		if (!rc) {
			resetBoard();
		}
	}
	dprintf("runApp ......\n");
	runApp(0x08010000);
	while (1);
}


void resetBoard(void)
{
	IncreaseUpdateTimes();
	while (true) {
		NVIC_GenerateSystemReset();
//		NVIC_GenerateCoreReset();
	}
}


void runApp(unsigned int addr)
{
	unsigned int *p = (unsigned int *)addr;
	unsigned int sp = *p++;
	void (*func)(void) = (void (*)(void))*p;
	NVIC_DeInit();
	
	{
		__MSR_MSP(sp);
		func();
		while(1);
	}
}
