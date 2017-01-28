#include <stdio.h>
#include <stdlib.h>
#include "c_list.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
static inline int int_comparer(void * x, void * y)
{
    return *(int *)(x) - *(int *)(y);
}
static void print_list2(c_iterator first, c_iterator last)
{
    c_iterator iter;
    printf("list is :\n"); 
    for(iter = first;
          !ITER_EQUAL(iter, last); ITER_INC(iter))
    {
        printf("\t%d\n", *((int *)(ITER_REF(iter))));
    }
} 
static void print_list(c_list * p)
{
    c_iterator first, last;
    first = c_list_begin(p);
    last = c_list_end(p);
    print_list2(first, last);
}

int main(){
	 int ary[] = { 0,0,0,0,0,0 };
    int ary2[] = { 2,87,1,9,2,5,0,54,1,7,23,0,43,2,9 };
    c_list lt, lt2;
    int i;
    c_list_create(&lt, NULL);
    __c_list(&lt2, NULL);
    for(i = 0; i < (sizeof(ary) / sizeof(int)); ++ i)
    {
        c_list_push_back(&lt, &i);
    }
    for(i = 0; i < (sizeof(ary2) /sizeof(int)); ++ i)
    {
        c_list_push_back(&lt2, &ary2[i]);
    }
    
    print_list(&lt);
    print_list(&lt2);
    
    __c_tsil(&lt);
    __c_tsil(&lt2);
}