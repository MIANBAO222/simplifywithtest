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
#define granularity 1024//文件粒度
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
strncpy(hashreturn,md,SHA_DIGEST_LENGTH);
return 0;
}
int copy_file_to_filebuf(const char *file,size_t length,char filebuf[])
{
    FILE *fp;
    if((fp=fopen(file,"rb"))==NULL)
    {
        printf("nofile -->%s\n",file);
        return 1;
    }
    fseek(fp,0,SEEK_END); //定位到文件末
    int nFileLen = ftell(fp); //文件长度
    printf("wenjianchang%d\n", nFileLen);
    if(nFileLen!=length)
    {
    return 1;//超长
    }
    fseek(fp,0,SEEK_SET); //定位到文件头
    if(fread(filebuf,nFileLen,1,fp)==1)
    {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
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
  	char hashtostr[41]={0};
	hashtostr[41]='\0';
	char filebuf[granularity]={0};
	if(copy_file_to_filebuf("485ee3f34c9a0ebba14edaae702ae5433c28b542.tem",1024,filebuf)==0)
            {
                buf_sha_calculate(filebuf,hashreturn);//计算
                saveHash(hashreturn, SHA_DIGEST_LENGTH,hashtostr); //转换为40位16进制哈希值
                printf("%s\n",hashtostr );
            }
}