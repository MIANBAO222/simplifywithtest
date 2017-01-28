#include <stdio.h>
#include <stdlib.h>
#include "c_list.h"
#define granularity 1024;//文件粒度
struct file_info//list
{
    char hash[21];
    int shadow_mark;//shadow区mark=1,否则
    int size;
};
struct file_use//map
{
    char hash[21];
    int use_num;
};
int write_fileinfo_into_list(c_list file_list,const char* path){//读取文件信息表
	int fd;
	if((fd=open(path,O_RDONLY))>0){
        struct file_info *new_one;
        new_one=malloc(sizeof(struct file_info));
        while(read(fd,new_one,sizeof(new_one))==sizeof(new_one)){
             c_list_push_back(&file_list,*new_one);
            new_one=malloc(sizeof(struct file_info));
            }
            free(new_one);
        }
        else{
            printf("%s\n","open_error" );
            return -1;
        }
        return 0;
        close(fd);
}
int copy_fileinfo_from_list(const c_list file_list,fd){
    c_iterator first, last;
    first = c_list_begin(file_list);
    last = c_list_end(file_list);
    c_iterator iter;
    printf("list is :\n"); 
    for(iter = first;
          !ITER_EQUAL(iter, last); ITER_INC(iter))//猜测ITER_INC是迭代器
    {
        printf("%s\n","writeing_into_fileinfo..." );
        //printf("\t%d\n", *((int *)(ITER_REF(iter))));
        if(write(fd,ITER_REF(iter),sizeof(ITER_REF(iter)))==sizeof(ITER_REF(iter)))continue;
        else{
        	printf("%s\n","error_copy_fileinfo_from_list" );
        	return -1;
        }
    }
}
int count_if(c_list file_info){
    c_iterator begin, c_iterator last;
    c_iterator iter;
    first = c_list_begin(file_list);
    last = c_list_end(file_list);

    int count=0;
    for(iter = first;!ITER_EQUAL(iter, last); ITER_INC(iter))//猜测ITER_INC是迭代器
    {
    	cout++;
    }
    return cout;
}
int find_the_file(clist file_info,int pos,c_iterator* get_return){
	c_iterator first,last,iter;
    first = c_list_begin(file_list);
    last = c_list_end(file_list);
     for(iter = first;!ITER_EQUAL(iter, last); ITER_INC(iter))//猜测ITER_INC是迭代器
    {
    	cout++;
    	if(count==pos){
    		get_return=&iter;
    		return 0;
    	}
    }
    return 1;
}
static int file_sha_calculate(const char *path){//还需要一个buf_sha_calculate
	 SHA_CTX stx;
     unsigned char outmd[20];//注意这里的字符个数为20
     char buffer[1024];
     //char filename[32];
     int len=0;
     int i;
     int fp=0;
     memset(outmd,0,sizeof(outmd));
     //memset(filename,0,sizeof(filename));
     memset(buffer,0,sizeof(buffer));
     //printf("请输入文件名，用于计算SHA1值:");
     //scanf("%s",filename);
     fp=open(path,O_RDONLY);
     if(fp<0)
     {
         printf("Can't open file\n");
         return 0;
     } 
     SHA1_Init(&stx);
     while((len=fread(buffer,1,1024,fp))>0)
     {
         SHA1_Update(&stx,buffer,len);
         memset(buffer,0,sizeof(buffer));
     }
     SHA1_Final(outmd,&stx);
 
     for(i=0;i<20;i<i++)
     {
        printf("%02X",outmd[i]);
     }
     printf("\n");
    return 0;
}
static int buf_sha_calculate(const char *buf,char *hashreturn){//hashreturn hash/0
SHA_CTX stx;
SHA1_Update(&stx,buf,1024);
SHA1_Final(hashreturn,&stx);
hashreturn[21]='/0';
return 0;
}
/*
error list
-1 open_file_error
-2 list_init_eror
*/
static int pmfs_write(const char *path, const char *buf, size_t size,off_t offset)
{
	c_list lt;
	c_iterator pos,last,get_return;
	int shadow_mark;
	int file_true_size;
	int offset_num,offset_pos;
	int buf_pos;
	int buf_size;
	int list_size;
	c_list_create(&lt, NULL);
	int i=0;
	char temp_filename[23];
	int fd,fd2;
	char temp[1025]={0};
	char sha[21]={0};
	buf_size=sizeof(buf)/sizeof(char);
	int write_size=0;
	if((fd=open(path,O_RDWR|O_CREAT))>0){//open_file
		printf("%s\n","open_file_info_success");
	}
	else{
		printf("%s\n", "error_open_fileinfo");
		return -1;
	}
	if(write_fileinfo_into_list(lt,path)==0){//list_init
		printf("%s\n", "init_list_success");
	}
	else{
		printf("%s\n", "init_list_error");
		return -2;
	}
    last = c_list_end(&lt);
    if(ITER_INC(last)->shadow_mark=1||ITER_INC(last)->shadow_mark=0){
     printf("%s\n","shadow_mark_get_success" );
     shadow_mark=ITER_INC(last)->shadow_mark;
    }
    else{
    	printf("%s\n", "error_shadow_mark");
    	return -3;
    }
    list_size=count_if(lt);
    if(shadow_mark==0){
    	file_true_size=granularity*list_size;
    }
    else{
    	file_true_size=granularity*(list_size-1)+ITER_INC(last)->size;
    }
    while(buf_pos<size){   
     while(offset_num<=list_size){
    	offset_num=offset/granularity;
    	offset_pos=offset%granularity;
    	if(find_the_file(lt,offset_num,get_return)==0){
    		if((fd2=open(get_return->hash,O_RDONLY))>0){
    			if(read(fd2,temp,get_return->size)==get_return->size){
    				close(fd2);
    				if(buf_size>=buf_pos+1024){
    					write_size=1024;
    					strncpy(temp,buf+buf_pos,1024);
    				}
    				else {
    					write_size=buf_size-buf_pos;
    					strncpy(temp,buf+buf_pos,buf_size-buf_pos);
    				}
    				strncpy(temp_filename,get_return->size,20);
    				temp_filename[20]=i;
    				temp_filename[21]='/0';
    				i++;
    				buf_sha_calculate(temp,sha);//ceshe
    				if((fd2=open(sha,O_RDWR|O_CREAT))>0){
    					if(write(fd2,temp,1024)==1024){
    						buf_pos=buf_pos+write_size;
    						offset_num++;
    						offset_pos=0;
    						get_return->hash=sha;
    						get_return->shadow_mark=0;
    						get_return->size=write_size;
    					}
    					else{
    						printf("%s\n", "error_write_temp_file");
    					}
    				}
    				else{
    					printf("%s\n", "error_open_temp_file");
    				}
    			}
    			else{
    				printf("%s\n","error_read_fd2" );
    			}
    		}
    		else{
    			printf("%s\n", "error_open_fd2");
    		}
    	}
    	else{
    		printf("%s\n","error_not_get_the_file" );
    	}
    }
    while(offset>file_true_size){
    	struct file_info *new_item;
     new_item=malloc(sizeof(struct file_info));
     c_list_push_back(&lt,new_item);
    }
}

    close(fd);
	}
int main()
{
char array[]="abcdefghijklmnopqrsdtuvwxyz";
pmfs_write("./test",array, 10,0);
return 0;
}

