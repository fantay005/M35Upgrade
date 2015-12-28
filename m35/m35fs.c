#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "hardware.h"
#include "m35fs.h"
#include "m35at.h"


int m35fOpen(const char *path, int mode)
{

	char buf[64];
	int len;

	sprintf(buf, "AT+QFOPEN=\"%s\",%d\n", path, mode);
	if (!m35AtChat(buf, "+QFOPEN:", buf, 3000)) return -1;

	for (len = 8; len < strlen(buf); len++) {
		if (isdigit(buf[len])) break;
	}

	if (len >= strlen(buf)) return -1;

	return atoi(&buf[len]);
}

int m35fRead(int fd, void *buffer, int byteToRead)
{
	char buf[64];
	int len;
	int dataLen;
	sprintf(buf, "AT+QFREAD=%d,%d\n", fd, byteToRead);
	if (!m35AtChat(buf, "CONNECT", buf, 3000)) return -1;
	for (len = 7; len < strlen(buf); len++) {
		if (isdigit(buf[len])) break;
	}

	if (len >= strlen(buf)) return -1;
	dataLen = atoi(&buf[len]);

	dataLen = m35ReceiveData(buffer, dataLen, 100*dataLen);
	if (6 != (len = m35ReceiveData(buf, 6, 1200))) {
		dprintf("%s error here %d, len = %d\n", __func__, __LINE__, len);
		return -1;
	}
	if (strncmp(buf, "\r\nOK\r\n", 6) != 0) {
		dprintf("%s error here %d", __func__, __LINE__);
		return -1;
	}

	return dataLen;
}

int m35fWrite(int fd, void *buffer, int byteToWrite, int *byteWriten)
{
	return 0;
}

int m35fClose(int fd)
{
	char buf[64];
	sprintf(buf, "AT+QFCLOSE=%d\n", fd);
	m35AtChat(buf, NULL, NULL, 0);
	return 0;
}

int m35fSeek(int fd, int offset, int seekMode)
{
	char buf[64];
	sprintf(buf, "AT+QFSEEK=%d,%d,%d\n", fd, offset, seekMode);
	if (!m35AtChat(buf, "OK", buf, 3000)) return -1;
	return 0;
}

