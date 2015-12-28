#include <string.h>
#include "m35at.h"
#include "m35.h"
#include "hardware.h"

bool m35AtChat(const char *cmd, const char *responsePrefix, char *result, unsigned int timeoutMs)
{
	int n = 0;
	bool cr = false;

	dprintf("m35AtChat-->\n");
	dprintf("command: %s", cmd);

	while(*cmd) m35SerialSendByte(*cmd++);
	if (responsePrefix == NULL || result == NULL) return true;

	//IWDG_ReloadCounter();//Î¹¹·
	setDelayMs(timeoutMs);
	while (true) {
		IWDG_ReloadCounter();//Î¹¹·
		if (isTimeout()) return false;
		if (m35SerialIsGotByte()) {
			result[n] = m35SerialGetByte();
//			putchar(result[n]);
			if (result[n] != responsePrefix[n]) {
				n = 0;
				continue;
			}
			n++;
			if (n == strlen(responsePrefix)) break;
		}
	}

	while (true) {
		IWDG_ReloadCounter();//Î¹¹·
		if (isTimeout()) return true;
		if (m35SerialIsGotByte()) {
			result[n] = m35SerialGetByte();
//			putchar(result[n]);
			if (cr) {
				if ('\n' == result[n]) {
					result[n] = 0;
					break;
				}
			}
			if (result[n] == '\r') {
				result[n] = 0;
				cr = true;
				continue;
			}
			n++;
		}
	}

	dprintf("    reply: %s\n", result);

	return true;
}

int m35ReceiveData(char *data, int len, int timeoutMs)
{
	int readLen = 0;

	setDelayMs(timeoutMs);
	while(readLen < len) {
		if (isTimeout()) return readLen;
		if (m35SerialIsGotByte()) {
			*data++ = m35SerialGetByte();
//			USART_SendData(USART1, *(data-1));
			readLen++;
		}
	}
	return readLen;
}


bool m35Init(void)
{
	int i;
	static const int bauds[] = {19200, 115200, 9600, 38400, 51200, 4800 };
	char buf[20];
	for (i = 0; i < 6; ++i) {
	printf("Init gsm baud: %d\n", bauds[i]); 
	usart2Configure(bauds[i]);
	m35AtChat("AT\r", "OK", buf, 500);
	m35AtChat("AT\r", "OK", buf, 500);
	m35AtChat("AT\r", "OK", buf, 500);
	if (m35AtChat("ATE0\r", "OK", buf, 1000)) {
		break;
		}
	}
	if (i >= 6) {
		printf("All baud error\n"); 
		return false;
	}
//	m35AtChat("AT\n", NULL, NULL, 0);
//	m35AtChat("AT\n", NULL, NULL, 0);
//	m35AtChat("AT\n", NULL, NULL, 0);
//	if (!m35AtChat("ATE0\n", "OK", buf, 1000)) return false;
	if (!m35AtChat("AT+IFC=2,2\n", "OK", buf, 1000)) return false;

	return true;
}
