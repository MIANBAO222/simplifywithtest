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
#include "c_map.h"
//openssl
#include <openssl/sha.h>   
#include <openssl/crypto.h>  // OPENSSL_cleanse  
#define granularity 1024//文件粒度

struct file_info//list
{
    char hash[41];
    int last_mark;//shadow区mark=1,否则就是shadow区
    size_t size;
};

struct file_use//map
{
    char hash[41];
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
        printf("savingfileinfo--->%s\n", one->hash);
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
        printf("NEWFILE");
        struct file_info *newfileinfo;
        newfileinfo = (struct file_info*)malloc(sizeof(struct file_info));
        strcpy(newfileinfo->hash,"");
        newfileinfo->last_mark = 1;
        newfileinfo->size = 0;
        c_list_push_back(&file_message, newfileinfo);
        return;
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
            if(fread(&buf,sizeof(struct file_info),1,fp)!=1) printf("file read error\n");
            printf("%s --end >> \n",buf.hash);
            printf("%d--end >> \n",buf.last_mark);
            printf("%zu --end >> \n",buf.size);
            struct file_info *message;
            message = (struct file_info*)malloc(sizeof(struct file_info));
            strcpy(message->hash,buf.hash);
            message->last_mark = buf.last_mark;
            message->size = buf.size;
            c_list_push_back(&file_message, message);
        }
    }
    else
    {
        printf("%s\n","没数据" );
    }

    fclose(fp);
}
//读取块文件
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
//保存块文件
void save_filebuf_to_file(const char *file,size_t length,char filebuf[])
{
    FILE *fp;
    if((fp=fopen(file,"rb"))!=NULL)//存在
    {
        printf("file ---->%s-----exist\n",file);
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
        printf("save file ---->---->%s-----error\n",file);
    }
    printf("save file ---->%s-----sucess\n",file);
    fclose(fp);
}
void save_temporary_filebuf_to_file(const char *file,size_t length,char filebuf[])
{
    FILE *fp;
        char tem[45];//44越界了
    strcpy(tem,file);
    strcat(tem,".tem");
    if((fp=fopen(tem,"rb"))!=NULL)//存在
    {
        printf("file ---->%s-----exist\n",tem);
        fclose(fp);
        return;
    }

    if((fp=fopen(tem,"wb"))==NULL)
    {
        printf("canot open the file.");
        exit(0);
    }
    printf("length-->%zu\n", length);
    if(fwrite(filebuf,length,1,fp)!=1)
    {
        printf("save file ---->---->%s-----error\n",tem);
    }
    printf("save file ---->%s-----sucess\n",tem);
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
    char hash[3];
    int i = 0;  
    for (i = 0; i < len; i++)  
    {  
        // printf("%02x", md[i]);  
        snprintf(hash, sizeof(hash), "%02x",md[i]);
        strncpy(str+2*i,hash,2);
    }  
}
//计算哈希值
static int buf_sha_calculate(const char *buf,size_t length,unsigned char *hashreturn){//hashreturn hash/0
// SHA_CTX stx;
unsigned char md[SHA_DIGEST_LENGTH];  
// printf("buf-->%s\n", buf);
printf("strlen(buf)--->%zu\n", length);
SHA1((unsigned char *)buf, length, md);  
strncpy(hashreturn,md,SHA_DIGEST_LENGTH);
return 0;
}
void insert_empty_node(c_list lt){
    printf("%s\n","扩展");
    struct file_info *add_last;
    add_last = (struct file_info*)malloc(sizeof(struct file_info));
    strcpy(add_last->hash, "");
    add_last->last_mark = 1;
    add_last->size = 0;
     c_list_push_back(&lt, add_last);
}
void free_list(c_list lt){
    c_iterator iter, first, last;
    last = c_list_end(&lt);
    first = c_list_begin(&lt);
    for(iter = first; !ITER_EQUAL(iter, last); ITER_INC(iter))
    {
        free(((struct file_info *)ITER_REF(iter)));
    }
}
//保存文件引用表
void save_map(c_list  lt)
{
    printf("%s\n","save_map_start" );
    FILE *fp;
    if((fp=fopen("map_file","wb"))==NULL)
    {
        printf("canot open the file.");
        return;
    }
    c_iterator iter, first, last;
    last = c_list_end(&lt);
    first = c_list_begin(&lt);
    for(iter = first; !ITER_EQUAL(iter, last); ITER_INC(iter))
    {
        struct file_use *init_map_file_info=((struct file_use *)ITER_REF(iter));
        printf("savingmap_key--->%s  map_values--->%d\n", init_map_file_info->hash,init_map_file_info->use_num);
       if(fwrite(init_map_file_info,sizeof(struct file_use),1,fp)!=1)
        {
            printf("file write error\n");
        }
    }
    fclose(fp);
}

//读取文件引用表
void read_map(c_list lt,c_map m)
{
    FILE *fp;
    int i;
    if((fp=fopen("map_file","rb"))==NULL)
    {
        printf("can`t open the file");
        return;
    }
    fseek(fp,0,SEEK_END); //定位到文件末
    int nFileLen = ftell(fp); //文件长度
    printf("文件长:%d\n", nFileLen);
    int tmpCount = 0;
    int tmpScount = sizeof(struct file_use);
    printf("大小:%d\n", tmpScount);
    tmpCount = nFileLen / tmpScount ;
    printf(" 节点数:%d\n", tmpCount);
    
    struct file_use buf;
    struct file_use *message;
    c_pair *pair;
    if(tmpCount!=0)
    {
        printf("%s\n","有数据" );
        for(i=0; i<tmpCount; i++)
        {
            printf("节点%d\n",i+1 );
            fseek(fp,tmpScount*i,tmpScount*i);
            if(fread(&buf,sizeof(struct file_use),1,fp)!=1) printf("file write error\n");
            printf("%s --end >> \n",buf.hash);
            printf("%d--end >> \n",buf.use_num);
            message = (struct file_use*)malloc(sizeof(struct file_use));
            strcpy(message->hash,buf.hash);
            message->use_num = buf.use_num;
            c_list_push_back(&lt, message);
            pair=(struct c_pair*)malloc(sizeof(struct c_pair));
            *pair=c_make_pair(&message->hash,&message->use_num);
            printf("读取--->key值：%s\n", pair->first);
            printf("读取--->value值：%d\n", *(int *)pair->second);
            c_map_insert(&m, pair);
        }
    }
    else
    {
        printf("%s\n","没数据" );
    }
    fclose(fp);
}
//打印map
void print_map(c_pmap pt)
{
    c_iterator iter = c_map_begin(pt);
    c_iterator end = c_map_end(pt);
    printf("map is:\n");
    for(; !ITER_EQUAL(iter, end); ITER_INC(iter))
    {
        printf("key = %s   value=%d\n",
            ((c_ppair)ITER_REF(iter))->first, *(int *)((c_ppair)ITER_REF(iter))->second);
    }
}

static inline int char_comparer(void * x, void * y)
{
    //return *(int *)(x) - *(int *)(y);
    return strcmp((char *)(x) , (char *)(y));
}
int main()
{
    const char *file="filetest";
    c_list file_list;
    c_list_create(&file_list, NULL);
    struct file_info *new_one;
    new_one = (struct file_info*)malloc(sizeof(struct file_info));
    strcpy(new_one->hash, "bd2146915b54954c8e080792655ff4746abed595");
    new_one->last_mark=0;
    new_one->size = granularity;
    //make a test list
    c_list_push_back(&file_list, new_one);
    new_one = (struct file_info*)malloc(sizeof(struct file_info));
    strcpy(new_one->hash, "403361df2e96de4e9e694e0e888a442df3b54e82");
    new_one->last_mark=1;
    new_one->size = granularity;
    c_list_push_back(&file_list, new_one);
    save_file_info(file_list,file);
    free_list(file_list);
    //start test
    unsigned char hashreturn[20]={0};
    char hashtostr[41];
    memset(hashtostr,'\0',sizeof(hashtostr));
    const char *buf="abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    size_t daxiao=52;//daxiao
    off_t juli=2044;//kaishi
    c_list lt;
    c_iterator iter, first, last;
    c_list_create(&lt, NULL);
    read_file_info(file,lt);//readfile
    last = c_list_end(&lt);
    first = c_list_begin(&lt);
    off_t buf_juli=0;
    int i=1;
    for(iter = first; !ITER_EQUAL(iter, last); ITER_INC(iter))
    {
        printf("NO.%dFILE\n", i);
        i++;
        struct file_info *one=((struct file_info *)ITER_REF(iter));
        printf("%s\n", one->hash);
        printf("%d\n", one->last_mark);
        printf("%zu\n", one->size);
        if(one->last_mark==0)
        {
            if(granularity>juli)// 这个文件写入
            {
                printf("granularity>juli--->%ld\n", juli);
                off_t nowjuli=juli;//开始写入位置
                juli=0;
                size_t nowdaxia=granularity-nowjuli;//可以写入的的大小
                if(nowdaxia<=daxiao-buf_juli)
                {
                    char filebuf[granularity]={0};
                    if(copy_file_to_filebuf(one->hash,one->size,filebuf)==0)
                    {
                        memcpy(filebuf+nowjuli,buf+buf_juli,nowdaxia);//写入filebuf
                        buf_juli=buf_juli+nowdaxia;
                        printf("buf_juli--->%ld\n", buf_juli);
                        buf_sha_calculate(filebuf,granularity,hashreturn);//计算
                        saveHash(hashreturn, SHA_DIGEST_LENGTH,hashtostr); //转换为40位16进制哈希值
                        printf("%s\n",hashtostr );
                        strcpy(one->hash, hashtostr);
                        save_temporary_filebuf_to_file(hashtostr,one->size,filebuf);//先保存临时文件
                    }
                }
                else//nowdaxia>daxiao-buf_juli
                {
                    char filebuf[granularity]={0};
                    if(copy_file_to_filebuf(one->hash,one->size,filebuf)==0)
                    {
                        memcpy(filebuf+nowjuli,buf+buf_juli,daxiao-buf_juli);//写入filebuf
                        printf("buf_juli--->%ld\n", buf_juli);
                        buf_sha_calculate(filebuf,granularity,hashreturn);//计算
                        saveHash(hashreturn, SHA_DIGEST_LENGTH,hashtostr); //转换为40位16进制哈希值
                        printf("%s\n",hashtostr );
                        strcpy(one->hash, hashtostr);
                        buf_juli=buf_juli+daxiao-buf_juli;
                        save_temporary_filebuf_to_file(hashtostr,one->size,filebuf);//先保存临时文件
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
        // if(strcmp(one->hash,"")==0)//扩展出来的空文件
            printf("lastfile+juli--->%ld\n", juli);
            if(granularity>=juli)// 这个文件写入
            {
                printf("granularity>juli=--->%ld\n", juli);
                off_t nowjuli=juli;//开始写入位置
                juli=0;
                size_t nowdaxia=granularity-nowjuli;//可以写入的的大小
                if(nowdaxia<daxiao-buf_juli)
                {
                    printf("daxiao--->%zu\n", daxiao);
                    printf("buf_juli--->%ld\n", buf_juli);
                    char filebuf[granularity]={0};
                    int openflag=copy_file_to_filebuf(one->hash,one->size,filebuf);
                    if(openflag==0|strcmp(one->hash,"")==0)
                    {
                        if(one->size<nowjuli)
                        {
                            memset(filebuf+one->size, '\0', nowjuli-one->size); 
                        }
                        memcpy(filebuf+nowjuli,buf+buf_juli,nowdaxia);//写入filebuf
                        buf_juli=buf_juli+nowdaxia;
                        buf_sha_calculate(filebuf,nowjuli+nowdaxia,hashreturn);//计算
                        saveHash(hashreturn, SHA_DIGEST_LENGTH,hashtostr); //转换为40位16进制哈希值
                        printf("%s\n",hashtostr );
                        strcpy(one->hash, hashtostr);
                        one->size=nowjuli+nowdaxia;
                        one->last_mark=0;
                        save_temporary_filebuf_to_file(hashtostr,one->size,filebuf);//先保存临时文件
                        //扩展
                        insert_empty_node(lt);
                        last = c_list_end(&lt);
                    }
                    else//既不是扩展的空文件也没找到文件
                    {
                        printf("NOFOUND!!!!!ERROR!!!!!!!!--->%s\n", one->hash);
                    }
                }
                else//nowdaxia>=daxiao-buf_juli
                {
                    char filebuf[granularity]={0};
                    int openflag=copy_file_to_filebuf(one->hash,one->size,filebuf);
                    if(openflag==0|strcmp(one->hash,"")==0)
                    {
                        if(one->size<nowjuli)
                        {
                            memset(filebuf+one->size, '\0', nowjuli-one->size); 
                        }
                        memcpy(filebuf+nowjuli,buf+buf_juli,daxiao-buf_juli);//写入filebuf
                        printf("nowjuli+nowdaxia--->%lu\n", nowjuli+nowdaxia);
                        buf_sha_calculate(filebuf,nowjuli+daxiao-buf_juli,hashreturn);//计算
                        saveHash(hashreturn, SHA_DIGEST_LENGTH,hashtostr); //转换为40位16进制哈希值
                        printf("%s\n",hashtostr );
                        strcpy(one->hash, hashtostr);
                        one->size=nowjuli+daxiao-buf_juli;
                        buf_juli=buf_juli+daxiao-buf_juli;
                        save_temporary_filebuf_to_file(hashtostr,one->size,filebuf);//先保存临时文件
                        break;//写完了
                    }
                    else//既不是扩展的空文件也没找到文件
                    {
                        printf("NOFOUND!!!!!ERROR!!!!!!!!--->%s\n", one->hash);
                    }
                }
            }
            else//granularity<juli
            {
                char filebuf[granularity]={0};
                int openflag=copy_file_to_filebuf(one->hash,one->size,filebuf);
                if(openflag==0|strcmp(one->hash,"")==0)
                {
                    memset(filebuf+one->size, '\0', granularity-one->size); 
                    buf_sha_calculate(filebuf,granularity,hashreturn);//计算
                    saveHash(hashreturn, SHA_DIGEST_LENGTH,hashtostr); //转换为40位16进制哈希值
                    printf("%s\n",hashtostr );
                    strcpy(one->hash, hashtostr);
                    one->size=granularity;
                    one->last_mark=0;
                    save_temporary_filebuf_to_file(hashtostr,one->size,filebuf);//先保存临时文件
                    //扩展
                     insert_empty_node(lt);
                     last = c_list_end(&lt);
                }
                else//既不是扩展的空文件也没找到文件
                {
                    printf("NOFOUND!!!!!ERROR!!!!!!!!--->%s\n", one->hash);
                }
                juli=juli-granularity;
            }
        }
    }
    //
    printf("文件引用表测试\n");
    c_list lt_old;
    c_list file_use_list;
    c_list_create(&lt_old, NULL);
    c_list_create(&file_use_list, NULL);
    read_file_info(file,lt_old);//readfile
    c_map map_init;//存储文件引用表的map初始化
    c_map_create(&map_init, char_comparer);//创建map
    c_pair *init_pair;
    struct file_use *init_node;
    c_iterator iter_old, first_old, last_old;//原list
    //模拟初始化 value为1
    first_old = c_list_begin(&lt_old);
    last_old = c_list_end(&lt_old);
    for(iter_old = first_old; !ITER_EQUAL(iter_old, last_old); ITER_INC(iter_old))
    { 
    struct file_info *init_map_file_info=((struct file_info *)ITER_REF(iter_old));
    init_node=malloc(sizeof(struct file_use));
    strcpy(init_node->hash,init_map_file_info->hash);
    init_node->use_num=1;
    printf("init_key--->%s  init_value--->%d\n", init_node->hash,init_node->use_num);
    c_list_push_back(&file_use_list, init_node);
    init_pair=malloc(sizeof(struct c_pair));
    *init_pair = c_make_pair(&init_node->hash,&init_node->use_num);
    c_map_insert(&map_init, init_pair);//插入
    }
    print_map(&map_init);
    save_map(file_use_list);
    c_map_destroy(&map_init);
    free_list(file_use_list);
    //start_test_map
    c_list new_file_use;
    c_list_create(&new_file_use,NULL);
    c_map map_new;
    c_map_create(&map_new, char_comparer);//创建map

    read_map(new_file_use,map_new);
     print_map(&map_new);

    //  c_iterator iter1, first1, last1;//原list
    //  last1 = c_list_end(&new_file_use);
    // first1 = c_list_begin(&new_file_use);
    // for(iter1 = first1; !ITER_EQUAL(iter1, last1); ITER_INC(iter1))
    // {
    //     struct file_use *file_node=((struct file_use *)ITER_REF(iter1));
    //     printf("file_key--->%s  file_value--->%d\n", file_node->hash,init_node->use_num);
    // }
    // save_file_info(lt,file);
    // free_list(lt);
    return 0;
}


