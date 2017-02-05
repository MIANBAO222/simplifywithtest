//openssl
#include <openssl/sha.h>   
#include <openssl/crypto.h>  // OPENSSL_cleanse
//stl
#include <stdio.h>
#include <stdlib.h>
#include "c_list.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>  
void printHash(unsigned char *md, int len)  
{  
    int i = 0;  
    for (i = 0; i < len; i++)  
    {  
        printf("%02x", md[i]);  
    }  
  
    printf("\n");  
} 
void saveHash(unsigned char *md, int len,char *str)
{
	unsigned char hash[3];
	// strncpy(hashreturn,md,SHA_DIGEST_LENGTH);
        int i = 0;  
    for (i = 0; i < len; i++)  
    {  
        // printf("%02x", md[i]);  
        snprintf(hash, sizeof(hash), "%02x",md[i]);
        strncpy(str+2*i,hash,2);
    }  
}
static int buf_sha_calculate(const char *buf,unsigned char *hashreturn){//hashreturn hash/0
// SHA_CTX stx;
unsigned char md[SHA_DIGEST_LENGTH];  
SHA1((unsigned char *)buf, strlen(buf), md);  
// printHash(md, SHA_DIGEST_LENGTH); 
// SHA1_Init(&stx);  
// SHA1_Update(&stx,buf,strlen(buf));
// SHA1_Final(md,&stx);
// OPENSSL_cleanse(&stx, sizeof(stx));  
// printHash(md, SHA_DIGEST_LENGTH);  
strncpy(hashreturn,md,SHA_DIGEST_LENGTH);
return 0;
}

int main(){
	unsigned char hashreturn[20]={0};
	char str[41];
	const char *buf="abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
	buf_sha_calculate(buf,hashreturn);
	saveHash(hashreturn, SHA_DIGEST_LENGTH,str); 
	printHash(hashreturn, SHA_DIGEST_LENGTH);
	str[41]='\0';
	printf("%s\n", str);
}