//fuse
#define FUSE_USE_VERSION 30
#include <config.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
//stl
#include <stdio.h>
#include <stdlib.h>
#include "c_list.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
//openssl
#include <openssl/sha.h>   
#include <openssl/crypto.h>  // OPENSSL_cleanse  
#define granularity 1024//文件粒度

struct file_info//list
{
    char hash[41];
    int shadow_mark;//shadow区mark=1,否则就是shadow区
    size_t size;
};

struct file_use//map
{
    char hash[21];
    int use_num;
};

//保存文件块表
void save_file_info(c_list  lt,const char *file)
{
    FILE *fp;
    if((fp=fopen(file,"wb"))==NULL)
    {
        printf("canot open the file.");
        exit(0);
    }
    c_iterator iter, first, last;
    last = c_list_end(&lt);
    first = c_list_begin(&lt);
    for(iter = first; !ITER_EQUAL(iter, last); ITER_INC(iter))
    {
        struct file_info *one=((struct file_info *)ITER_REF(iter));
        printf("%s\n", one->hash);
           if(fwrite(one,sizeof(struct file_info),1,fp)!=1)
    {
        printf("file write error\n");
    }

    }

    fclose(fp);
}
//读取文件块表
void read_file_info(const char *file,c_list file_message)
{
    FILE *fp;
    int i;
    if((fp=fopen(file,"rb"))==NULL)
    {
        printf("can`t open the file");
        exit(0);
    }

    fseek(fp,0,SEEK_END); //定位到文件末
    int nFileLen = ftell(fp); //文件长度
    printf("wenjianchang%d\n", nFileLen);
    int tmpCount = 0;
    int tmpScount = sizeof(struct file_info);
    printf("daxiao%d\n", tmpScount);
    tmpCount = nFileLen / tmpScount ;//计算有多少个粒度文件

    
    struct file_info buf;
    if(tmpCount!=0)
    {
        printf("%s\n","you shuju" );
        for(i=0; i<tmpCount; i++)
        {
            fseek(fp,tmpScount*i,tmpScount*i);
            if(fread(&buf,sizeof(struct file_info),1,fp)!=1) printf("file write error\n");
            printf("%s --end >> \n",buf.hash);
            printf("%d--end >> \n",buf.shadow_mark);
            printf("%zu --end >> \n",buf.size);
            struct file_info *message;
            message = (struct file_info*)malloc(sizeof(struct file_info));
            strcpy(message->hash,buf.hash);
            message->shadow_mark = buf.shadow_mark;
            message->size = buf.size;
            c_list_push_back(&file_message, message);
        }
    }
    else
    {
        printf("%s\n","meishuju" );
    }

    fclose(fp);
}
//读取块文件
int copy_file_to_filebuf(const char *file,size_t length,char filebuf[])
{
    FILE *fp;
    if((fp=fopen(file,"rb"))==NULL)
    {
        printf("can`t open -->%s\n",file);
        exit(0);
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
//保存块文件
void save_filebuf_to_file(const char *file,size_t length,char filebuf[])
{
    FILE *fp;
    if((fp=fopen(file,"rb"))!=NULL)//存在
    {
        fclose(fp);
        return;
    }
    if((fp=fopen(file,"wb"))==NULL)
    {
        printf("canot open the file.");
        exit(0);
    }
    if(fwrite(filebuf,length,1,fp)!=1)
    {
        printf("file write error---->%s\n",file);
    }
    fclose(fp);
}
//输出哈希值
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
//计算哈希值
static int buf_sha_calculate(const char *buf,unsigned char *hashreturn){//hashreturn hash/0
// SHA_CTX stx;
unsigned char md[SHA_DIGEST_LENGTH];  
SHA1((unsigned char *)buf, strlen(buf), md);  
strncpy(hashreturn,md,SHA_DIGEST_LENGTH);
return 0;
}
int main()
{
    const char *file="filetest";
    c_list file_list;
    c_list_create(&file_list, NULL);
    struct file_info *new_one;
    new_one = (struct file_info*)malloc(sizeof(struct file_info));
    strcpy(new_one->hash, "bd2146915b54954c8e080792655ff4746abed595");
    new_one->shadow_mark = 1;
    new_one->size = granularity;
    //make a test list
    c_list_push_back(&file_list, new_one);
    new_one = (struct file_info*)malloc(sizeof(struct file_info));
    strcpy(new_one->hash, "7e078c40616dc706f55a6f348a624a6d5384dec7");
    new_one->shadow_mark = 1;
    new_one->size = granularity;
    c_list_push_back(&file_list, new_one);
    save_file_info(file_list,file);

    //start test
    unsigned char hashreturn[20]={0};
    char hashtostr[41]={0};
    hashtostr[41]='\0';
    const char *buf="abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    size_t daxiao=52;//daxiao
    off_t juli=1020;//kaishi
    c_list lt;
    c_iterator iter, first, last;
    c_list_create(&lt, NULL);
    read_file_info(file,lt);//readfile
    last = c_list_end(&lt);
    first = c_list_begin(&lt);
    off_t buf_juli=0;
    for(iter = first; !ITER_EQUAL(iter, last); ITER_INC(iter))
    {
        struct file_info *one=((struct file_info *)ITER_REF(iter));
        printf("%s\n", one->hash);
        printf("%d\n", one->shadow_mark);
        printf("%zu\n", one->size);
        if(one->shadow_mark==1)
        {
            if(granularity>juli)// 这个文件写入
            {
                off_t nowjuli=juli;//开始写入位置
                juli=0;
                size_t nowdaxia=granularity-nowjuli;//可以写入的的大小
                if(nowdaxia<=daxiao-buf_juli)
                {
                    char filebuf[granularity]={0};
                    if(copy_file_to_filebuf(one->hash,one->size,filebuf)==0)
                    {
                        strncpy(filebuf+nowjuli,buf+buf_juli,nowdaxia);//写入filebuf
                        buf_juli=buf_juli+nowdaxia;
                        buf_sha_calculate(filebuf,hashreturn);//计算
                        saveHash(hashreturn, SHA_DIGEST_LENGTH,hashtostr); //转换为40位16进制哈希值
                        strcpy(one->hash, hashtostr);
                        save_filebuf_to_file(hashtostr,one->size,filebuf);//先保存临时文件
                    }
                }
                else//nowdaxia>daxiao-buf_juli
                {
                    char filebuf[granularity]={0};
                    if(copy_file_to_filebuf(one->hash,one->size,filebuf)==0)
                    {
                        strncpy(filebuf+nowjuli,buf+buf_juli,daxiao);//写入filebuf
                        buf_juli=buf_juli+daxiao;
                        buf_sha_calculate(filebuf,hashreturn);//计算
                        saveHash(hashreturn, SHA_DIGEST_LENGTH,hashtostr); //转换为40位16进制哈希值
                        strcpy(one->hash, hashtostr);
                        save_filebuf_to_file(hashtostr,one->size,filebuf);//先保存临时文件
                        break;//写完了
                    }
                }
            }
            else//nowdaxia>daxiao
            {
                juli=juli-granularity;
            }
        }
        else//最后一个
        {
            if(granularity>=juli)// 这个文件写入
            {
                off_t nowjuli=juli;//开始写入位置
                juli=0;
                size_t nowdaxia=granularity-nowjuli;//可以写入的的大小
                if(nowdaxia<=daxiao-buf_juli)
                {
                    char filebuf[granularity]={0};
                    if(copy_file_to_filebuf(one->hash,one->size,filebuf)==0)
                    {
                        strncpy(filebuf+nowjuli,buf+buf_juli,nowdaxia);//写入filebuf
                        buf_juli=buf_juli+nowdaxia;
                        buf_sha_calculate(filebuf,hashreturn);//计算
                        saveHash(hashreturn, SHA_DIGEST_LENGTH,hashtostr); //转换为40位16进制哈希值
                        strcpy(one->hash, hashtostr);
                        save_filebuf_to_file(hashtostr,one->size,filebuf);//先保存临时文件
                        //扩展
                        struct file_info *add_last;
                        add_last = (struct file_info*)malloc(sizeof(struct file_info));
                        strcpy(add_last->hash, '\0');
                        add_last->shadow_mark = 0;
                        add_last->size = 0;
                        c_list_push_back(&lt, add_last);
                        last = c_list_end(&lt);
                    }
                }
            }
            else//granularity<juli
            {
                char filebuf[granularity]={0};
                if(copy_file_to_filebuf(one->hash,one->size,filebuf)==0)
                {
                    memset(filebuf+one->size, '\0', granularity-one->size); 
                    buf_sha_calculate(filebuf,hashreturn);//计算
                    saveHash(hashreturn, SHA_DIGEST_LENGTH,hashtostr); //转换为40位16进制哈希值
                    strcpy(one->hash, hashtostr);
                    one->size=granularity;
                    one->shadow_mark=1;
                    save_filebuf_to_file(hashtostr,one->size,filebuf);//先保存临时文件
                    //扩展
                    struct file_info *add_last;
                    add_last = (struct file_info*)malloc(sizeof(struct file_info));
                    strcpy(add_last->hash, '\0');
                    add_last->shadow_mark = 0;
                    add_last->size = 0;
                    c_list_push_back(&lt, add_last);
                    last = c_list_end(&lt);
                }
                juli=juli-granularity;
            }
        }
        
    }
    return 0;
}


