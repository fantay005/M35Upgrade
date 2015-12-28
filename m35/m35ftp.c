#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "m35ftp.h"
#include "m35at.h"

bool m35FtpConnect(const char *host, int port, int timeoutMs)
{
	char buf[80];
	int len;
	sprintf(buf, "AT+QFTPOPEN=\"%s\",%d\n", host, port);
	if (!m35AtChat(buf, "+QFTPOPEN:", buf, timeoutMs)) return false;

	for (len = 10; len < strlen(buf); len++) {
		if (isdigit(buf[len]) || buf[len] == '-') break;
	}

	if (len >= strlen(buf)) return false;

	return (0 == atoi(&buf[len]));
}

bool m35FtpClose()
{
	char buf[80];
	int len;
	if (!m35AtChat("AT+QFTPCLOSE\n", "+QFTPCLOSE:", buf, 6000)) return false;

	for (len = 10; len < strlen(buf); len++) {
		if (isdigit(buf[len]) || buf[len] == '-') break;
	}

	if (len >= strlen(buf)) return false;

	return (0 == atoi(&buf[len]));
}

bool m35FtpDownload(const char *remoteFile, const char *localFile, int *fileLen)
{
	char buf[128];
	int len;

	sprintf(buf, "AT+QFTPCFG=4,\"%s\"\n", localFile);
	if (!m35AtChat(buf, "+QFTPCFG:", buf, 2000)) return false;
	for (len = 9; len < strlen(buf); len++) {
		if (isdigit(buf[len])) break;
	}
	if (len >= strlen(buf)) return false;
	if (0 != atoi(&buf[len])) return false;

	sprintf(buf, "AT+QFTPGET=\"%s\",%d\n", remoteFile, 524288);
	if (!m35AtChat(buf, "+QFTPGET:", buf, 1000000)) return false;
	for (len = 9; len < strlen(buf); len++) {
		if (isdigit(buf[len]) || buf[len] == '-') break;
	}
	if (len >= strlen(buf)) return false;
	if ((*fileLen = atoi(&buf[len])) <= 0) return false;

	return true;
}
