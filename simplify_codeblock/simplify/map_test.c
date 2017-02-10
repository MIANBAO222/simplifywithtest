#include <stdio.h>
#include <stdlib.h>
#include "../include/c_def.h"
#include "../include/c_vector.h"
#include "c_map.h"
#include <assert.h>
#define PATH_PMFS "/mnt/pmfs"
int main(){
// int i=1;
// int k=2;
// c_pair test;
// test=c_make_pair(i,k);
// printf("%d\n", test.first);
// printf("%d\n", test.second);
	char *path="/";
	char *pmfs=malloc(strlen(path)+strlen(PATH_PMFS));
	strcpy(pmfs,PATH_PMFS);
	strcat(pmfs,path);
	printf("pmfs--->%s\n", pmfs);
	free(pmfs);
}
