#ifndef __M35_H__
#define __M35_H__

#include <stdbool.h>

bool m35Startup(void);
bool m35Stop(void);
void m35SerialSendByte(char ch);
bool m35SerialIsGotByte(void);
char m35SerialGetByte(void);
void m35SerialSendbuff(unsigned char Num, unsigned char *ch);

#endif
