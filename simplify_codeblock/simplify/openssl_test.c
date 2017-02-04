//openssl
#include <openssl/sha.h>   
#include <openssl/crypto.h>  // OPENSSL_cleanse  
void printHash(unsigned char *md, int len)  
{  
    int i = 0;  
    for (i = 0; i < len; i++)  
    {  
        printf("%02x", md[i]);  
    }  
  
    printf("\n");  
} 
static int buf_sha_calculate(const char *buf,unsigned char *hashreturn){//hashreturn hash/0
SHA_CTX stx;
unsigned char md[SHA_DIGEST_LENGTH];  
SHA1((unsigned char *)buf, strlen(buf), md);  
printHash(md, SHA_DIGEST_LENGTH); 
SHA1_Init(&stx);  
SHA1_Update(&stx,buf,strlen(buf));
SHA1_Final(md,&stx);
OPENSSL_cleanse(&stx, sizeof(stx));  
printHash(md, SHA_DIGEST_LENGTH);  
// printf("md--->%x\n", md);
strncpy(hashreturn,md,SHA_DIGEST_LENGTH);
return 0;
}

int main(){
	unsigned char hashreturn[20]={0};
	const char *buf="abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    	size_t daxiao=52;//daxiao
	// printf("%s\n",hashreturn);
	buf_sha_calculate(buf,hashreturn);
	// printf("%s\n",hashreturn);
	printHash(hashreturn, SHA_DIGEST_LENGTH); 
}