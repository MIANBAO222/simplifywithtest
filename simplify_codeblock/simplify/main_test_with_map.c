//fuse
#define FUSE_USE_VERSION 30
#include "config.h"
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
#include "c_map.h"
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
    char tem[44];
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
    printf("length-->%d\n", length);
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
static int buf_sha_calculate(const char *buf,size_t length,unsigned char *hashreturn){//hashreturn hash/0
// SHA_CTX stx;
unsigned char md[SHA_DIGEST_LENGTH];  
// printf("buf-->%s\n", buf);
printf("strlen(buf)--->%d\n", length);
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


//保存文件引用表
void save_map(c_map  m)
{
    FILE *fp;
    if((fp=fopen("map_file","wb"))==NULL)
    {
        printf("canot open the file.");
        exit(0);
    }
    c_iterator iter, first, last;
    last = c_map_end(&m);
    first = c_map_begin(&m);
    for(iter = first; !ITER_EQUAL(iter, last); ITER_INC(iter))
    {
        struct file_use one;
        strcpy(one.hash,((c_pair *)ITER_REF(iter))->first);
        one.use_num=*(int*)((c_pair *)ITER_REF(iter))->second;
        printf("savingmap_key--->%s  map_values--->%d\n", one.hash,one.use_num);
        if(fwrite(&one,sizeof(struct file_use),1,fp)!=1)
    {
        printf("file write error\n");
    }
        
    }
    fclose(fp);
}

//读取文件引用表
void read_map(c_map m)
{
    FILE *fp;
    int i;
    if((fp=fopen("map_file","rb"))==NULL)
    {
        printf("can`t open the file");
        exit(0);
    }

    fseek(fp,0,SEEK_END); //定位到文件末
    int nFileLen = ftell(fp); //文件长度
    printf("文件长:%d\n", nFileLen);
    int tmpCount = 0;
    int tmpScount = sizeof(struct file_use);
    printf("大小:%d\n", tmpScount);
    tmpCount = nFileLen / tmpScount ;
    printf(" 节点数:%d\n", tmpScount);
    
    struct file_use buf;
    c_pair *p;
    if(tmpCount!=0)
    {
        printf("%s\n","有数据" );
        for(i=0; i<tmpCount; i++)
        {
            fseek(fp,tmpScount*i,tmpScount*i);
            if(fread(&buf,sizeof(struct file_use),1,fp)!=1) printf("file write error\n");
            printf("%s --end >> \n",buf.hash);
            printf("%d--end >> \n",buf.use_num);
            p=(struct c_pair*)malloc(sizeof(struct c_pair));
            char *key=(char *)malloc(sizeof(char)*41);
            int *num=(int *)malloc(sizeof(int));
            strncpy(key,buf.hash,41);
            *num = buf.use_num;
            *p=c_make_pair(key,num);
            printf("存储在c_pair p中的key值：%s\n", p->first);
            printf("存储在c_pair p中的value值：%d\n", *(int *)p->second);
            c_map_insert(&m, p);
        }
    }
    else
    {
        printf("%s\n","没数据" );
    }

    fclose(fp);
}
void free_map(c_pmap m)
{
    c_iterator iter = c_map_begin(m);
    c_iterator end = c_map_end(m);
    printf("free-->map is:\n");
    for(; !ITER_EQUAL(iter, end); ITER_INC(iter))
    {
        printf("key = %s, value = %d\n",((c_ppair)ITER_REF(iter))->first,*(int *)((c_ppair)ITER_REF(iter))->second);
        char *key=(((c_ppair)ITER_REF(iter))->first);
        int *num=*(int *)(((c_ppair)ITER_REF(iter))->second);
        c_pair *p=(c_ppair)ITER_REF(iter);
        printf("free-->key = %s, value = %d\n",key,num);
        free(key);
        free(num);
    }
}
//打印map
void print_map(c_pmap pt)
{
	c_iterator iter = c_map_begin(pt);
	c_iterator end = c_map_end(pt);
	printf("map is:\n");
	for(; !ITER_EQUAL(iter, end); ITER_INC(iter))
	{
		printf("key = %s, value = %d\n",
			((c_ppair)ITER_REF(iter))->first,
			*(int *)((c_ppair)ITER_REF(iter))->second);
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

    //文件引用表初始化
    c_list old_lt;
    c_list_create(&old_lt, NULL);
    read_file_info(file,old_lt);//readfile


    //start test
    unsigned char hashreturn[20]={0};
    char hashtostr[41]={0};
    hashtostr[41]='\0';
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
                        printf("buf_juli--->%d\n", buf_juli);
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
                        printf("buf_juli--->%d\n", buf_juli);
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
                    printf("daxiao--->%d\n", daxiao);
                    printf("buf_juli--->%d\n", buf_juli);
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
                        printf("nowjuli+nowdaxia--->%d\n", nowjuli+nowdaxia);
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
    
    //
    save_file_info(lt,file);
    

//文件引用表
    printf("文件引用表代码测试开始\n");
    c_list now_lt;
    c_list_create(&now_lt, NULL);
    read_file_info(file,now_lt);//readfile
    c_iterator iter1, first1, last1;//原list
    c_iterator iter2, first2, last2;//现list
    printf("\n");

    c_map map_init;//存储文件引用表的map初始化
    c_map_create(&map_init, char_comparer);//创建map
    c_pair *q;
   
    int *init_times;//引用次数
    
    int count=0;

    char **key;
    key=(char **)malloc(sizeof(char *));
    *key =(char *)malloc(sizeof(char)*41);

    last1 = c_list_end(&old_lt);
    first1 = c_list_begin(&old_lt);
    last2 = c_list_end(&now_lt);
    first2 = c_list_begin(&now_lt);

    for(iter1 = first1; !ITER_EQUAL(iter1, last1); ITER_INC(iter1))
    {       
         struct file_info *test1=((struct file_info *)ITER_REF(iter1));
         *key = &test1->hash;
        //将原链表里的文件引用信息装进文件引用表
         init_times=malloc(sizeof(int *));
         *init_times=1;
         q=malloc(sizeof(struct c_pair));
         *q = c_make_pair(*key,init_times);
         c_map_insert(&map_init, q);//插入
    }  
    print_map(&map_init);//第一次输出map
    save_map(map_init);

    c_map map;//文件引用表
    c_map_create(&map, char_comparer);//创建map
    printf("读取map_file文件内容开始\n");
    read_map(map);
    printf("读取map_file文件内容结束\n");

    iter1 = first1;
    int len=c_list_size(&old_lt);
    for(iter2 = first2; !ITER_EQUAL(iter2, last2); ITER_INC(iter2))//外层迭代比较
    {
        struct file_info *test2=((struct file_info *)ITER_REF(iter2));
        struct file_info *test1=((struct file_info *)ITER_REF(iter1));
        if(count<len)//判断原链表是否遍历完毕
        { 
            count=count+1;
            if(!strcmp(&test1->hash,&test2->hash))//文件未改动
            {
            //printf("无改动\n");
            }
            else //文件被修改过
            {
                 //printf("文件被修改过\n");
                 c_iterator target,end;
                 c_iterator map_end = c_map_end(&map);
                 target = c_map_find(&map, &test2->hash); // 根据新链表中节点key值查找map值          
                 if(!ITER_EQUAL(map_end, target)) 
                 {//若在map中找到则修改map中values+1
                 
                 *(int*)(((c_ppair)ITER_REF(target))->second)=*(int*)(((c_ppair)ITER_REF(target))->second)+1;
                 }
                 else  //若在map中找不到则新增map值
                 {  
                     *key = &test2->hash;
                     init_times=malloc(sizeof(int *));
                     *init_times=1;
                     q=malloc(sizeof(struct c_pair));
                     *q = c_make_pair(*key,init_times);
                     c_map_insert(&map, q);//插入
                 }

                 //printf("%s\n", &test1->hash);
                 target = c_map_find(&map, &test1->hash);// 根据原链表中节点key值查找map值                  
                 //修改map中values-1
                *(int*)(((c_ppair)ITER_REF(target))->second)=*(int*)(((c_ppair)ITER_REF(target))->second)-1;

                 //检测到values==0则从map中删除
                 if(*(int*)(((c_ppair)ITER_REF(target))->second)==0)
                 {
                 c_map_erase(&map, target);//删除特定元素
                 }
             }
             ITER_INC(iter1);
        }

        //新链表中的新节点，新增map值
         *key = &test2->hash;
         init_times=malloc(sizeof(int *));
         *init_times=1;
         q=malloc(sizeof(struct c_pair));
         *q = c_make_pair(*key,init_times);
         c_map_insert(&map, q);//插入
    }

    for(;!ITER_EQUAL(iter1, last1);ITER_INC(iter1))//若旧链表长于新链表
    {
         struct file_info *test1=((struct file_info *)ITER_REF(iter1));
         c_iterator target;
         target = c_map_find(&map, &test1->hash);
         //修改map中values-1
         *(int*)(((c_ppair)ITER_REF(target))->second)=*(int*)(((c_ppair)ITER_REF(target))->second)-1;

         //检测到values==0则从map中删除
         if(*(int*)(((c_ppair)ITER_REF(target))->second)==0)
         {
             c_map_erase(&map, target);//删除特定元素
         }
    }

    print_map(&map);//第二次输出map（经过处理）
    save_map(map);
    // free(q);
    // free(key);
    // free(init_times);
    free_map(&map);
    return 0;
}


