// kenwood_unpack_TS-990_TS-890.cpp  : Defines the entry point for the console application.
// for Kenwood TS-990S and TS-890

#include "stdafx.h" 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int _tmain(int argc, _TCHAR* argv[])
{
	unsigned int  decrypt_size;
	unsigned char *buff;
	unsigned char *buffer;
	unsigned char *buf;
	unsigned char *addr;
	FILE *file_fd;
	FILE *fd;
	int fileLen;
	unsigned char md5hash[16] = {0,};
	unsigned char version[8] = {0,};

	char outfile[255] ;
	strcpy(outfile,"outfile.zip"); 
	if (argc  >	1 ) {
		printf("File to unpack : %s\n", (char*)argv[1]);
		if (argc == 2 || argc == 3 ) {
			file_fd = fopen((char*)argv[1], "rb");
			if (argc == 3) {
				strcpy(outfile, (char*)argv[2]);
				printf("File to store : %s\n", outfile);
				fd = fopen(outfile, "wb");
			} else {
				strcpy(outfile, "outfile.zip");
				printf("File to store : %s\n" , outfile);
				fd = fopen((const char*) outfile, "wb");
			}
		}
		fseek(file_fd, 0, SEEK_END);
		fileLen=ftell(file_fd);
		fseek(file_fd, 0, SEEK_SET);
		buffer=(unsigned char *)malloc(fileLen+1);
		if (!buffer){
			fprintf(stderr, "Memory error!");
			free (buffer);
			fclose (file_fd);
			fclose(fd);
			return 0;
		}
		fread(buffer, fileLen, 1, file_fd);
		if (fileLen != *(unsigned int*) buffer) {
			printf("wrong size of file ...exit...\n");
			free (buffer);
			fclose (file_fd);
			fclose(fd);
			return 0;
		}
		buf=(unsigned char *)malloc(fileLen);
		memcpy(buf , buffer + 28, fileLen - 28);
		buff = buf;
		addr = buffer;
		decrypt_size = fileLen - 28;
		*(unsigned long long *)md5hash		= *(unsigned long long *)(buffer +  4) ^ 0xCA1797D065893342LL;
		*(unsigned long long *)&md5hash[8]	= *(unsigned long long *)(buffer + 12) ^ 0x933360EC2EA5FC5ELL;
		*(unsigned long long *)version		= *(unsigned long long *)(buffer + 20) ^ 0x5C4F2A08F7C1C57ALL; 
		printf("version : %s\n", version);
		if ( fileLen != 28 ) {
			int n;
			unsigned short xor_val = 0x8E96u;
			for (n = 0; n < decrypt_size; ) {
				int i = 0;
				unsigned short xor = xor_val;
				for (i = 0; i < 0x1000; i+=2) {
					*(unsigned short *) buf  ^= xor;
					xor += 0x3247;
					buf++;
					buf++;
					n += 2;
					if (n  >= decrypt_size ) break;
				}
				xor_val += 0x3800;
			}
		}
		fwrite (buff , sizeof(char), fileLen - 28, fd);
		fclose (fd);
		fclose (file_fd);
		free(buff);
		free(buffer);
	} 
	printf ("Use kenwood_decryptor TS-XXX_VXXX.dat outfile.zip\n%s contain updatefile to be unziped\nxxx_xxxx.tar.gz shall under linux extracted.\n", outfile );
	
	return 0;
}


