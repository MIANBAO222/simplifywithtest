#include <stdio.h>
#include <stdlib.h>
#include "c_list.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <openssl/sha.h>   
#include <openssl/crypto.h>  // OPENSSL_cleanse  
#define granularity 1024//文件粒度

struct file_info//list
{
    char hash[21];
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
    int i;
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
        printf("cant open the file");
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
int copy_file_to_filebuf(const char *file,char filebuf[])
{
    FILE *fp;
    int i;
    if((fp=fopen(file,"rb"))==NULL)
    {
        printf("cant open the file");
        exit(0);
    }
    fseek(fp,0,SEEK_END); //定位到文件末
    int nFileLen = ftell(fp); //文件长度
    printf("wenjianchang%d\n", nFileLen);
    fseek(fp,0,SEEK_SET); //定位到文件头
    if(fread(filebuf,nFileLen,1,fp)==1){
            fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}
//计算哈希值
static int buf_sha_calculate(const char *buf,char *hashreturn,size_t length){//hashreturn hash/0
SHA_CTX stx;
SHA1_Update(&stx,buf,length);
unsigned char md[SHA_DIGEST_LENGTH];  
SHA1_Final(md,&stx);
strncpy(hashreturn,md,SHA_DIGEST_LENGTH);
hashreturn[21]='\0';
return 0;
}
int main()
{
    const char *file="filetest";
    c_list file_list;
    c_list_create(&file_list, NULL);
    struct file_info *new_one;
    new_one = (struct file_info*)malloc(sizeof(struct file_info));
    strcpy(new_one->hash, "12345678901234567890");
    new_one->shadow_mark = 1;
    new_one->size = 1024;
    //make a test list
    c_list_push_back(&file_list, new_one);
    new_one = (struct file_info*)malloc(sizeof(struct file_info));
    strcpy(new_one->hash, "abcdefglmnabcdefglmn");
    new_one->shadow_mark = 1;
    new_one->size = 1024;
    c_list_push_back(&file_list, new_one);
    save_file_info(file_list,file);

    //start test
    const char *buf="abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    size_t daxiao=52;//daxiao
    off_t juli=1020;//kaishi
    c_list lt;
    c_iterator iter, first, last;
    c_list_create(&lt, NULL);
    read_file_info(file,lt);//readfile
    last = c_list_end(&lt);
    first = c_list_begin(&lt);
    for(iter = first; !ITER_EQUAL(iter, last); ITER_INC(iter))
    {
        struct file_info *one=((struct file_info *)ITER_REF(iter));
        printf("%s\n", one->hash);
        printf("%d\n", one->shadow_mark);
        printf("%zu\n", one->size);
        char filebuf[granularity+1]={0};
        copy_file_to_filebuf(one->hash,filebuf);
        printf("%s\n", filebuf);
        if(one->size>juli){// 这个文件开始写入
            if(juli>0){//数格子，写入
                off_t nowjuli=juli;//开始写入位置
                juli=0;
                size_t nowdaxia=one->size-nowjuli;//可以写入的的大小
                if(nowdaxia<=daxiao){
                    daxiao=daxiao-nowdaxia;
                    char filebuf[granularity+1]={0};
                    filebuf[granularity]='\0';
                    if(copy_file_to_filebuf(one->hash,filebuf)!=0){};//读取，处理，写入
                }
                else{//nowdaxia>daxiao
                    char filebuf[granularity+1]={0};
                    filebuf[granularity]='\0';
                    if(copy_file_to_filebuf(one->hash,filebuf)!=0){};//读取，处理，写入
                }
            }
            else{//juli=0
                size_t nowdaxia=one->size;//可以写入的的大小
                    if(nowdaxia<=daxiao){
                        daxiao=daxiao-nowdaxia;
                        char filebuf[granularity+1]={0};
                        filebuf[granularity]='\0';
                        if(copy_file_to_filebuf(one->hash,filebuf)!=0){};//读取，处理，写入
                         }
                        else{//nowdaxia>daxiao
                            char filebuf[granularity+1]={0};
                            filebuf[granularity]='\0';
                            if(copy_file_to_filebuf(one->hash,filebuf)!=0){};//读取，处理，写入
                        }
                    }
        }
        else{//one->size<juli
            if(one->size!=granularity){//最后一个
                if(granularity<juli){
                    //把最后一个填充为0, 往尾部增加一个空文件
                    last = c_list_end(&lt);
                }
                else{
                    //先补齐juli个0,写入数据, 往尾部增加一个空文件
                    last = c_list_end(&lt);
                }
            }
            else//不是最后一个
            {
             juli=juli-one->size;
            }

        }
    }
    return 0;
}


