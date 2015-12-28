#ifndef __M35FTP_H__
#define __M35FTP_H__

#include <stdbool.h>

bool m35FtpConnect(const char *host, int port, int timeoutMs);
bool m35FtpDownload(const char *remoteFile, const char *localFile, int *fileLen);
bool m35FtpClose(void);

#endif
