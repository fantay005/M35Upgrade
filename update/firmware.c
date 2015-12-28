#include <string.h>
#include <ctype.h>
#include "firmware.h"
#include "m35at.h"
#include "hardware.h"
#include "m35fs.h"
#include "m35ftp.h"
#include "m35sendsms.h"


struct Header {
	unsigned int headerCrc;
	unsigned int headerLength;
	unsigned int fileLength;
	unsigned int startAddr;
	unsigned int resolved[4];
	unsigned int crc[256];
} m_header;


#define FIRMWARE_FILE "firmware.bin"
#define BLOCK_SIZE 2048
#define HEADER_SIZE 32


unsigned int m_blockBuf[BLOCK_SIZE/sizeof(unsigned int)];

#include "libupdater.c"

static char notifySmsNum[20];

void notifyOperation(char *info)
{
	// 向 m_notifySMSNo 发送一条内容是 info 的短信
	if (strlen(notifySmsNum) != 0) {
		m35SendSms(notifySmsNum, info);
	}
}

int updateTimes() {
	int ret = 0;
	int i;
	for (i=0; i<sizeof(__mark->timesFlag)/sizeof(__mark->timesFlag[0]); ++i) {
		if (__mark->timesFlag[i] != 0xFFFFFFFF) {
			++ret;
		}
	}
	return ret;
}

void IncreaseUpdateTimes(void) {
	int i;
	for (i=0; i < sizeof(__mark->timesFlag)/sizeof(__mark->timesFlag[0]); i++) {
		if (__mark->timesFlag[i] == 0xFFFFFFFF) {
			FLASH_Unlock();
			FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);  
			FLASH_ProgramWord((u32)&__mark->timesFlag[i],0x00);
			FLASH_Lock();
			return;
		}
	}
}

bool isNeedDownloadFirmware(void)
{	
	if (__mark->activeFlag != __activeFlag) {
		dprintf("Don't need to upgrade -->\n");
		return false;
	}
	
	if (updateTimes() >= 5) {
		return false;
	}
	if (!isValidUpdateMark(__mark)) {
		dprintf("UnvalidUpdateMark.\n");
		notifyOperation("UnvalidUpdateMark.");
		return false;
	}

	strcpy(notifySmsNum, __mark->notifySmsNum);
	
	return true;
}

bool downloadFirmwareToRam(void)
{
	bool rc = false;
	char buf[120];
	int len;

	dprintf("downloadFirmware -->\n");
	dprintf("wait 10s to connect ftp\n");
	// 等待注册网络
	delayMs(10*1000);
	notifyOperation("UPGRADE FIRMWARE START");

	// 设置连接网络的参数
	if (!m35AtChat("AT+QIFGCNT=0\n", "OK", buf, 20000)) {
		eprintf("S1 QIFGCNT ERROR\n");
		notifyOperation("S1 QIFGCNT ERROR");
		return false;
	}

	if (!m35AtChat("AT+QICSGP=1,\"CMNET\"\n", "OK", buf, 20000)) {
		notifyOperation("S1 QICSGP ERROR");
		return false;
	}

	dprintf("connect ftp\n");
	// 连接FTP
	if (!m35FtpConnect(__mark->ftpHost, __mark->ftpPort, 100000)) {
		notifyOperation("S1 FTP CONNECT ERROR");
		return false;
	}
	dprintf("download file\n");

	// 下载文件保存到M35的内存
	if (!m35FtpDownload(__mark->remotePath, "/RAM/" FIRMWARE_FILE , &len)) {
		notifyOperation("S1 FTP DOWNLOAD ERROR");
		eprintf("S1 FTP DOWNLOAD ERROR\n");
	} else {
		dprintf("download succeed, len=%d\n", len);
		rc = true;
	}

	// 关闭FTP链接
	m35FtpClose();
	return rc;
}

static unsigned int calculateCrc32Stm32(void *buf, int len)
{
	unsigned int xbit, data;
	unsigned int polynomial = 0x04c11db7;
	unsigned int crc = 0xFFFFFFFF;    // init
	unsigned int *ptr = (unsigned int *)buf;
	len = len / 4;

    while (len--) {
        data = *ptr++;
        for (xbit = 0x80000000; xbit != 0; xbit = xbit>>1) {
            if (crc & 0x80000000) {
            	crc <<= 1;
            	crc ^= polynomial;
            } else {
                crc <<= 1;
            }

            if (data & xbit) {
                crc ^= polynomial;
            }
        }
    }
    return crc;
}

#define EraseTimeout             ((u32)0x00000FFF)
#define ProgramTimeout           ((u32)0x0000000F)

// 把一个BLOCK写入到内部FLASH并校验写入是否正确
static bool writeBlockDataToFlash(unsigned int *buf, int addr, int len)
{
	int t = 100;
	unsigned int *p = buf;
	unsigned int paddr = addr;
	int leftLen = len;
	int status;
	
	status = FLASH_ErasePage(addr);

	while ((status == FLASH_BUSY || status == FLASH_TIMEOUT) && t > 0) {
		status = FLASH_WaitForLastOperation(EraseTimeout);
	}

	if (status !=  FLASH_COMPLETE) return false;

	for (; leftLen > 0; leftLen -= 4, paddr += 4) {
		t = 100;
		status = FLASH_ProgramWord(paddr, *p++);
		while ((status == FLASH_BUSY || status == FLASH_TIMEOUT) && t > 0) {
			status = FLASH_WaitForLastOperation(ProgramTimeout);
		}
		if (status != FLASH_COMPLETE) return false;
	}

	if (0 != memcmp(buf, (void *)addr, len)) return false;

	return true;
}

// 校验下载文件，并写入代码区
bool programFromRam(void)
{
	int readLen;
	int blockNum;
	int fileLeftLen;
	int block;
	unsigned int addr;
	bool rc = true;
	unsigned int crc;
	
	int fd = m35fOpen("RAM:" FIRMWARE_FILE, 2);
	if (fd < 0) {
		notifyOperation("S2 OPEN ERROR");
		return true;
	}
	IWDG_ReloadCounter();//喂狗
	dprintf("Read header ...\n");
	readLen = m35fRead(fd, &m_header, HEADER_SIZE);
	if (readLen != HEADER_SIZE) {
		eprintf("Read header return error\n");
		notifyOperation("S2 READ HEADER ERROR");
		goto __exit;
	}
	
	dprintf("headerCrc: %08X\n", m_header.headerCrc);
	dprintf("headerLength: %08X\n", m_header.headerLength);
	dprintf("fileLength: %08X\n", m_header.fileLength);
	dprintf("startAddr: %08X\n", m_header.startAddr);
	
	if (m_header.startAddr != 0x08010000) {
		notifyOperation("S2 STARTADDR ERROR");
		goto __exit;
	}
	
	if (m_header.fileLength > 1024*(512-64-4)) {
		notifyOperation("S2 FILELENGTH ERROR");
		goto __exit;
	}

	blockNum = (m_header.fileLength + BLOCK_SIZE - 1)/BLOCK_SIZE;
	addr = m_header.startAddr;

	if ((addr & (BLOCK_SIZE - 1)) != 0) {
		notifyOperation("S2 START ADDR ERROR");
		eprintf("S2 START ADDR ERROR\n");
		goto __exit;
	}

	if (m_header.headerLength != HEADER_SIZE + blockNum * 4) {
		notifyOperation("S2 HEADER SIZE ERROR");
		eprintf("S2 HEADER SIZE ERROR\n");
		goto __exit;
	}

	dprintf("Read header CRC ...\n");
	readLen = m35fRead(fd, m_header.crc, 4 * blockNum);
	if (readLen != 4 * blockNum) {
		notifyOperation("S2 READ BLOCK CRC ERROR");
		dprintf("S2 READ BLOCK CRC ERROR\n");
		goto __exit;
	}
	
	dprintf("Calc header ...\n");	
	if ((crc = calculateCrc32Stm32(&(m_header.headerLength), m_header.headerLength - 4)) != m_header.headerCrc) {
		notifyOperation("S2 HEADER CRC ERROR");
		eprintf("Header CRC error expect %08X but %08X\n", m_header.headerCrc, crc);
		goto __exit;
	}

	rc = false;

	fileLeftLen = m_header.fileLength;
//	rc = true;
	IWDG_ReloadCounter();//喂狗
	FLASH_Unlock();
	for(block = 0; block < blockNum; block++) {
		readLen = (fileLeftLen > BLOCK_SIZE) ? BLOCK_SIZE : fileLeftLen;
		
		dprintf("Read block %d, len: %d ...\n", block, readLen);
		if (readLen != m35fRead(fd, m_blockBuf, readLen)) {
			char buf[48];
			sprintf(buf, "S2 READ BLOCK %d ERROR", block);
			notifyOperation(buf);
			eprintf("Read block %d error\n", block);			
			if (block != 0) {			
			NVIC_GenerateSystemReset();
			}
			goto __exit;
		}

		if (readLen != BLOCK_SIZE) {
			unsigned char *p = (unsigned char *)m_blockBuf;
			memset(&p[readLen], 0, BLOCK_SIZE - readLen);
		}

		if ((crc = calculateCrc32Stm32(m_blockBuf, BLOCK_SIZE)) != m_header.crc[block]) {
			char buf[48];
			sprintf(buf, "S2 BLOCK CRC %d ERROR", block);
			notifyOperation(buf);
			eprintf("Block %d CRC error, expect %08X but %08X\n", block, m_header.crc[block], crc);
			
			if (block != 0) {			
			NVIC_GenerateSystemReset();
			}
			goto __exit;
		}

		if (!writeBlockDataToFlash(m_blockBuf, addr, readLen)) {
			char buf[48];
			sprintf(buf, "S2 WRITE %08X ERROR", addr);
			notifyOperation(buf);
			eprintf("Write data to flash error\n");			
			NVIC_GenerateSystemReset();
			goto __exit;
		}
		fileLeftLen -= readLen;
		addr += readLen;
	}
	FLASH_Lock();
	
	dprintf("programFromRam OK\n");
	dprintf("eraseupdatemark \n");
	eraseupdatemark();
	
	notifyOperation("Upgrade is ok");
	rc = true;

__exit:
	m35fClose(fd);
	return rc;
}

bool updateFirmware(void)
{
	return true;
}

