#ifndef __M35AT_H__
#define __M35AT_H__

#include <stdbool.h>

bool m35AtChat(const char *cmd, const char *responsePrefix, char *result, unsigned int timeoutMs);
int m35ReceiveData(char *data, int len, int timeoutMs);

bool m35Init(void);

#endif

