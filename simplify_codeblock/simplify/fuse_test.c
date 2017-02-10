/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/

/** @file
 *
 * This file system mirrors the existing file system hierarchy of the
 * system, starting at the root file system. This is implemented by
 * just "passing through" all requests to the corresponding user-space
 * libc functions. Its performance is terrible.
 *
 * Compile with
 *
 *     gcc -Wall passthrough.c `pkg-config fuse3 --cflags --libs` -o passthrough
 *
 * ## Source code ##
 * \include passthrough.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "write_function.c"
#define FUSE_USE_VERSION 30
#define PATH_PMFS "/mnt/pmfs"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

static void *xmp_init(struct fuse_conn_info *conn,
		      struct fuse_config *cfg)
{
	(void) conn;
	cfg->use_ino = 1;
	return NULL;
}

static int xmp_getattr(const char *path, struct stat *stbuf,
		       struct fuse_file_info *fi)
{
	(void) fi;
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	res = lstat(pmfs, stbuf);
	if (res == -1){
		free(pmfs);
		return -errno;
	}
	free(pmfs);
	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	res = access(pmfs, mask);
	if (res == -1){
		free(pmfs);
		return -errno;
	}
	free(pmfs);
	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	res = readlink(pmfs, buf, size - 1);
	if (res == -1){
		free(pmfs);
		return -errno;
	}

	buf[res] = '\0';
	free(pmfs);
	return 0;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi,
		       enum fuse_readdir_flags flags)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;
	(void) flags;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	dp = opendir(pmfs);
	if (dp == NULL){
		free(pmfs);
		return -errno;
	}

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0, 0))
			break;
	}

	closedir(dp);
	free(pmfs);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(pmfs, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(pmfs, mode);
	else
		res = mknod(pmfs, mode, rdev);
	if (res == -1){
		free(pmfs);
		return -errno;
	}
	free(pmfs);
	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	res = mkdir(pmfs, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);

	res = unlink(pmfs);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);

	res = rmdir(pmfs);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to, unsigned int flags)
{
	int res;

	if (flags)
		return -EINVAL;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode,
		     struct fuse_file_info *fi)
{
	(void) fi;
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	res = chmod(pmfs, mode);
	if (res == -1){
		free(pmfs);
		return -errno;
	}
	free(pmfs);
	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid,
		     struct fuse_file_info *fi)
{
	(void) fi;
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	res = lchown(pmfs, uid, gid);
	if (res == -1){
		free(pmfs);
		return -errno;
	}
	free(pmfs);
	return 0;
}

static int xmp_truncate(const char *path, off_t size,
			struct fuse_file_info *fi)
{
	(void) fi;
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	res = truncate(pmfs, size);
	if (res == -1){
		free(pmfs);
		return -errno;
	}
	free(pmfs);
	return 0;
}

#ifdef HAVE_UTIMENSAT
static int xmp_utimens(const char *path, const struct timespec ts[2],
		       struct fuse_file_info *fi)
{
	(void) fi;
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, pmfs, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1){
		free(pmfs);
		return -errno;
	}
	free(pmfs);
	return 0;
}
#endif

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	res = open(pmfs, fi->flags);
	if (res == -1){
		free(pmfs);
		return -errno;
	}

	close(res);
	free(pmfs);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	(void) fi;
	fd = open(pmfs, O_RDONLY);
	if (fd == -1){
		free(pmfs);
		return -errno;
	}

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	free(pmfs);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	(void) fi;

	//
	unsigned char hashreturn[20]={0};
	char hashtostr[41];
	memset(hashtostr,'\0',sizeof(hashtostr));
	size_t daxiao=size;//daxiao
	off_t juli=offset;//kaishi
	c_list lt;
	c_iterator iter, first, last;
	c_list_create(&lt, NULL);
	read_file_info(path,pmfs,lt);//readfile
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
                    if(copy_file_to_filebuf(path,one->hash,one->size,filebuf)==0)
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
                    if(copy_file_to_filebuf(path,one->hash,one->size,filebuf)==0)
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
                    int openflag=copy_file_to_filebuf(path,one->hash,one->size,filebuf);
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
                    int openflag=copy_file_to_filebuf(path,one->hash,one->size,filebuf);
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
                int openflag=copy_file_to_filebuf(path,one->hash,one->size,filebuf);
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
    save_file_info(path,lt,pmfs);
    free_list(lt);
    free(pmfs);
    return buf_juli;









	//zhengchangieru
	// fd = open(pmfs, O_WRONLY);
	// if (fd == -1){
	// 	free(pmfs);
	// 	return -errno;
	// }

	// res = pwrite(fd, buf, size, offset);
	// if (res == -1)
	// 	res = -errno;

	// close(fd);
	// free(pmfs);
	// return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);

	res = statvfs(pmfs, stbuf);
	if (res == -1){
		free(pmfs);
		return -errno;
	}
	free(pmfs);
	return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_POSIX_FALLOCATE
static int xmp_fallocate(const char *path, int mode,
			off_t offset, off_t length, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

	if (mode)
		return -EOPNOTSUPP;
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	fd = open(pmfs, O_WRONLY);
	if (fd == -1){
		free(pmfs);
		return -errno;
	}

	res = -posix_fallocate(fd, offset, length);

	close(fd);
	free(pmfs);
	return res;
}
#endif

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	int res = lsetxattr(pmfs, name, value, size, flags);
	if (res == -1){
		free(pmfs);
		return -errno;
	}
	free(pmfs);
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	int res = lgetxattr(pmfs, name, value, size);
	if (res == -1){
		free(pmfs);
		return -errno;
	}
	free(pmfs);
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	int res = llistxattr(pmfs, list, size);
	if (res == -1){
		free(pmfs);
		return -errno;
	}
	free(pmfs);
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	int res = lremovexattr(pmfs, name);
	if (res == -1){
		free(pmfs);
		return -errno;
	}
	free(pmfs);
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
	.init           = xmp_init,
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
#ifdef HAVE_UTIMENSAT
	.utimens	= xmp_utimens,
#endif
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_POSIX_FALLOCATE
	.fallocate	= xmp_fallocate,
#endif
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
