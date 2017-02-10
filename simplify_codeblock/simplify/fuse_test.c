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
/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
 struct file_info//list
{
    char hash[41];
    int last_mark;//shadow区mark=1,否则就是shadow区
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


//fuse
static struct options {
	const char *filename;
	const char *contents;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};
static void *hello_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}
static int hello_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path+1, options.filename) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(options.contents);
	} else
		res = -ENOENT;

	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	filler(buf, options.filename, NULL, 0, 0);

	return 0;
}
static int hello_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path+1, options.filename) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(strcmp(path+1, options.filename) != 0)
		return -ENOENT;

	len = strlen(options.contents);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, options.contents + offset, size);
	} else
		size = 0;
	return size;
}
static int pmfs_write(const char *path, const char *buf, size_t size,off_t offset,struct fuse_file_info *fi)
{
	char *file=path;
	size_t daxiao=size;//daxiao
	off_t juli=offset;//kaishi
    	    unsigned char hashreturn[20]={0};
	    char hashtostr[41]={0};
	    hashtostr[41]='\0';
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
	    save_file_info(lt,file);
	    return 0;
}
static struct fuse_operations hello_oper = {
	.init           = hello_init,
	.getattr	= hello_getattr,
	.readdir	= hello_readdir,
	 .open		= hello_open,
	 .read		= hello_read,
	// .write              =pmfs_write,
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"hello\" file\n"
	       "                        (default: \"hello\")\n"
	       "    --contents=<s>      Contents \"hello\" file\n"
	       "                        (default \"Hello, World!\\n\")\n"
	       "\n");
}

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("hello");
	options.contents = strdup("Hello World!\n");

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0] = (char*) "";
	}

	return fuse_main(args.argc, args.argv, &hello_oper, NULL);
}
