#ifndef __M35FS_H__
#define __M35FS_H__

#define M35_FOPEN_READ 0x01
#define M35_FOPEN_WRITE 0x02
#define M35_FOPEN_TUCAT 0x04
#define M35_FOPEN_EXIST 0x08

// open() and creat() return the new file descriptor, or -1 if an error occurred
int m35fOpen(const char *path, int mode);
int m35fRead(int fd, void *buffer, int byteToRead);
int m35fWrite(int fd, void *buffer, int byteToWrite, int *byteWriten);
int m35fClose(int fd);

#define M35_FSEEK_BEGIN 0
#define M35_FSEEK_CURRENT 1
#define M35_FSEEK_END 2
int m35fSeek(int fd, int offset, int seekMode);

#endif


