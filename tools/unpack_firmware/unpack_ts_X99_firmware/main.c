/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    
    Hamradio TS-990 and TS-890 USB firmware unpacker and packer.
    Dejan Nadarevic - 9A1AD
    Initial Relase : 26.02.2019
    
    
    compile : gcc main.c -w -O2 -std=gnu99 -Os -o /opt/TS990/kenwood_tool 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>

void dump(unsigned char *d, int len) {
    for (int i = 0; i < len; i++) printf("%02x ",d[i]);
    printf("\n");
    
}
static void
_strdelchr( char *s, size_t i, size_t *a, size_t *b)
{
  size_t        j;

  if( *a == *b)
    *a = i - 1;
  else
    for( j = *b + 1; j < i; j++)
      s[++(*a)] = s[j];
  *b = i;
}
int zstring_search_chr(char *token,char s){
    if (!token || s=='\0')
        return 0;

    for (;*token; token++)
        if (*token == s)
            return 1;

    return 0;
}

char *zstring_remove_chr(char *str,const char *bad) {
    char *src = str , *dst = str;

    /* validate input */
    if (!(str && bad))
        return NULL;

    while(*src)
        if(zstring_search_chr(bad,*src))
            src++;
        else
            *dst++ = *src++;  /* assign first, then incement */

    *dst='\0';
    return str;
}

#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))
#define FW_PACK 0
#define FW_UNPACK 28
void md5(uint8_t *initial_msg, size_t initial_len, uint8_t * hashOut) {
    uint32_t h0, h1, h2, h3;
    uint8_t *msg = NULL;
    uint32_t r[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                    5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};
    uint32_t k[] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1, 
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

    h0 = 0x67452301;
    h1 = 0xefcdab89;
    h2 = 0x98badcfe;
    h3 = 0x10325476;
    int new_len;
    for(new_len = initial_len*8 + 1; new_len%512!=448; new_len++);
    new_len /= 8;
    msg = calloc(new_len + 64, 1); 
    memcpy(msg, initial_msg, initial_len);
    msg[initial_len] = 128; 
    uint32_t bits_len = 8*initial_len; 
    memcpy(msg + new_len, &bits_len, 4);
    int offset;
    for(offset=0; offset<new_len; offset += (512/8)) {
        uint32_t *w = (uint32_t *) (msg + offset);
        uint32_t a = h0;
        uint32_t b = h1;
        uint32_t c = h2;
        uint32_t d = h3;
        // Main loop:
        uint32_t i;
        for(i = 0; i<64; i++) {       
            uint32_t f, g;
             if (i < 16) {
                f = (b & c) | ((~b) & d);
                g = i;
            } else if (i < 32) {
                f = (d & b) | ((~d) & c);
                g = (5*i + 1) % 16;
            } else if (i < 48) {
                f = b ^ c ^ d;
                g = (3*i + 5) % 16;          
            } else {
                f = c ^ (b | (~d));
                g = (7*i) % 16;
            }
            uint32_t temp = d;
            d = c;
            c = b;
            b = b + LEFTROTATE((a + f + k[i] + w[g]), r[i]);
            a = temp;
        }
        h0 += a;h1 += b;h2 += c;h3 += d;
    }
    memcpy(hashOut   , &h0,  4);
    memcpy(hashOut+ 4, &h1,  4);
    memcpy(hashOut+ 8, &h2,  4);
    memcpy(hashOut+12, &h3,  4);
    free(msg);
}

int main(int argc, char* argv[])
{
	unsigned int  decrypt_size;
	unsigned char *buff;
	unsigned char *rbuff;
	unsigned char *wbuff;
    unsigned short xor, xor_val;
	char rfile[255] = {0,};
    char wfile[255] = {0,};
    FILE *rfd;
	FILE *wfd;
	int fileLen;
	unsigned char md5hash[16 + 8] = {0,};
    unsigned char md5Check[16] = {0,};
	unsigned char *version = md5hash + 16;
    unsigned char ver[10] = {0,};
    unsigned char op = 0;
    void usage(void)
    {
        printf("\n\nKenwood TS-990 and TS-890 Firmware packer/unpacker tool by 9A1AD\n\n"
            "Usage: \n"
            "./kenwood_tool -u TS-XXX_VXXX.dat for unpacking  (ex: TS-990_1.22.dat)\n" 
            "./kenwood_tool -v VX.XX -p tsXXX.zip for packing (ex: ts.990.zip and version V1.22)\n");
            
    }
    char data[101];
    int opt;

    memset(data, 0, 101);
    while ((opt = getopt(argc, argv, "hu:v:p:")) != -1) {
        switch (opt) {
        case 'h':
            usage();
            return 0;
        case 'u':
            if (optarg[0] == '-') { //not an optarg of ours...-h
                optind--;
                printf("option: -u. no tokens (another option follows)\n");
                break;
            }
            strncpy(rfile, optarg, strlen(optarg) );
            if (strstr(rfile, "TS-990")) {
                strcpy(wfile, "ts990.zip");
            } 
            if (strstr(rfile, "TS-890")) {
                strcpy(wfile, "ts890.zip");
            }
            printf("option: -u. tokens: %s writting to %s\n", rfile, wfile);
            op = FW_UNPACK;
            break;
        case 'p':
            strcpy(rfile, optarg);
            printf("option: -p tokens: %s\n", rfile);
            printf("...\n");
            if (data[0] != 'V') {
                strcpy (data, "V1.02");
                strcpy (ver,  "V1.02");
            }
            if (strstr(rfile, "ts990")) {
                sprintf(wfile, "TS-990_%s.dat", zstring_remove_chr(data,"."));
            } 
            if (strstr(rfile, "ts890")) {
                sprintf(wfile, "TS-890_%s.dat", zstring_remove_chr(data,"."));
            }
            printf("option: -p. tokens: %s writting to %s\n", rfile, wfile);
            op = FW_PACK;
            break;
        case 'v':
            if (optarg[0] == '-') { //not an optarg of ours...-h
                optind--;
                printf("option: -v. no tokens (another option follows)\n");
                
            }
            strncpy(ver, optarg, strlen(optarg) );
            strncpy(data, optarg, strlen(optarg) );
            printf("option: -v. tokens: %s \n", data);

            break;
            
        default:
            return 0;
        }
    }
	usage();
    rfd = fopen((char*)rfile, "rb");
    wfd = fopen(wfile, "wb");

    fseek(rfd, 0, SEEK_END);
    fileLen=ftell(rfd);
    fseek(rfd, 0, SEEK_SET);
    rbuff=(unsigned char *)malloc(fileLen + 29);
    wbuff=(unsigned char *)malloc(fileLen + 29);
    if (!rbuff)
    {
        fprintf(stderr, "Memory error!");
        fclose(rfd);
        fclose(wfd);
        return 0;
    }
    memset(rbuff, 0, fileLen + 29);
    memset(wbuff, 0, fileLen + 29);
    printf("\n\n\n");
    fread(rbuff, fileLen, 1, rfd);
    buff = wbuff;
    if (op == FW_UNPACK) {
        memcpy(wbuff, rbuff + op, fileLen);
    } else { 
        memcpy(wbuff + 28, rbuff + op, fileLen);
        wbuff += 28;
        md5(buff + 28, fileLen , buff + 4);
        if (ver[0] == 'V') {
            memset(buff + 20, 0x20,8);
            memcpy(buff + 20, ver,5);
        }
        *(unsigned int*) buff = fileLen + 28;
    }
    decrypt_size = fileLen;
    if ( fileLen != 28 ) {
        int i, n;
        xor_val = 0x8E96u;
        for (n = 0; n < decrypt_size; ) {
            xor = xor_val;
            for (i = 0; i < 0x1000; i+=2) {
                *(unsigned short *) wbuff  ^= xor;
                xor += 0x3247;
                wbuff+=2;
                n += 2;
                if (n  >= decrypt_size ) break;
            }
            xor_val += 0x3800;
        }
    }
    if (op == FW_UNPACK) {
        *(unsigned long long *)md5hash    = *(unsigned long long *)(rbuff +  4) ^ 0xCA1797D065893342;
        *(unsigned long long *)&md5hash[8]= *(unsigned long long *)(rbuff + 12) ^ 0x933360EC2EA5FC5E;
        *(unsigned long long *)version    = *(unsigned long long *)(rbuff + 20) ^ 0x5C4F2A08F7C1C57A; 
        printf("version : %s\n", version);
        md5(buff, fileLen - 28, md5Check);
        if (memcmp(md5Check, md5hash, 16) == 0) {
            printf("Firmware %s unpacked OK\n",wfile );
        } else {
            printf("Firmware %s unpacked NOK, check downloaded file\n", wfile);
        }
        dump(md5hash, 28);
        fwrite (buff , sizeof(char), fileLen - 28, wfd);
    } else if (op == FW_PACK) {
        *(unsigned long long *)(buff + 4)  ^= 0xCA1797D065893342;
        *(unsigned long long *)(buff + 12) ^= 0x933360EC2EA5FC5E;
        *(unsigned long long *)(buff + 20) ^= 0x5C4F2A08F7C1C57A;
        
        fwrite (buff , sizeof(char), fileLen + 28, wfd);
    }
    fclose(rfd);
    fclose(wfd);
    free(buff);
    free(rbuff);

	return 0;
}

