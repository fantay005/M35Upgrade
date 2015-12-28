#ifndef __M35SENDSMS_H__
#define __M35SENDSMS_H__

#include <stdbool.h>


bool m35SendSms(const char *SMSPhoneNumber,  char *senddata);
int gsmEncode7bit(void);
bool send_7bit_sms(void);
unsigned char TransferAimaAddr(unsigned char *ptype);
void memoryset(unsigned char *p, unsigned char v, int len);
bool m35WaitReply(const char *responsePrefix, char *result, unsigned int timeoutMs);

#endif
