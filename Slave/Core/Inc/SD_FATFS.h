#ifndef SD_FATFS_H_
#define SD_FATFS_H_
#include "fatfs.h"

//#define BUF4KB
//#define BUF8KB
//#define BUF12KB
#define BUF16KB

#define F4KB 4096
#define DATASIZE 4080
struct  SD_Iterface{
	FATFS fs;
	FIL fil;
	UINT bw,br;
	uint8_t fresult;

	#ifdef BUF4KB
	char buffer1[F4KB];
	#endif
	#ifdef BUF8KB
	char buffer1[F4KB];
	char buffer2[F4KB];
	#endif
	#ifdef BUF12KB
	char buffer1[F4KB];
	char buffer2[F4KB];
	char buffer3[F4KB];
	#endif
	#ifdef BUF16KB
	char buffer1[F4KB];
	char buffer2[F4KB];
	char buffer3[F4KB];
	char buffer4[F4KB];
	#endif



};

void mountSDcard(void);
void writeDataPacked(const TCHAR* path,BYTE mode);
void openFile(const TCHAR* path,BYTE mode);
void closeFile(void);
void bufclear(void);

#endif /* SD_FATFS_H_ */
