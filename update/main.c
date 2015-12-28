//˵����������һЩ��־�������Ĳ����������ڲ�flash�����һҳ���׵�ַ��0x0807F800
//�����ڶ�ҳ�����ڴ������ʧ�ܵĴ�������������3��ʱ�˴�����ʧ�ܣ�������־����ַ��0x0807F000
//�ڲ�flashÿһҳ�Ĵ�СΪ2K�ֽ�
//0-64K���bootloader����65-508K���Ӧ�ó���

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
	IWDG_ReloadCounter();//ι��

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
