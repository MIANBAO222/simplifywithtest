#include <stdio.h>
#include <stdlib.h>
#include "../include/c_def.h"
#include "../include/c_vector.h"
#include "c_map.h"
#include <assert.h>

c_bool __c_rb_tree_verify(c_prb_tree thiz);

static int values[] = 
{ 
	8,	/* 0 */
	3,	/* 1 */
	8,	/* 2 */
	4,	/* 3 */
	3,	/* 4 */
	8,	/* 5 */
	5,	/* 6 */
	43,	/* 7 */
	994,	/* 8 */
	32,	/* 9 */
	6,	/* 10 */
	9,	/* 11 */
	3,	/* 12 */
	7,	/* 13 */
	7,	/* 14 */
	32,	/* 15 */
	8,	/* 16 */
	2,	/* 17 */
	343 	/* 18 */
};

static inline int int_comparer(void * x, void * y)
{
    return *(int *)(x) - *(int *)(y);
}

static int keys[] = 
{ 
	12,	/* 0 */	
	4,	/* 1 */	
	71,	/* 2 */
	2,	/* 3 */
	90,	/* 4 */
	99,	/* 5 */
	30,	/* 6 */
	61,	/* 7 */
	29,	/* 8 */
	84,	/* 9 */
	6,	/* 10 */
	97,	/* 11 */
	3,	/* 12 */
	8,	/* 13 */
	54,	/* 14 */
	78,	/* 15 */
	9,	/* 16 */
	16,	/* 17 */
	62,	/* 18 */
};
//没有段错误的写法
// static c_pair pairs[] =
// {
// 	{ &keys[0], &values[0] },
// 	{ &keys[1], &values[1] },
// 	{ &keys[2], &values[2] },
// 	{ &keys[3], &values[3] },
// 	{ &keys[4], &values[4] },
// 	{ &keys[5], &values[5] },
// 	{ &keys[6], &values[6] },
// 	{ &keys[7], &values[7] },
// 	{ &keys[8], &values[8] },
// 	{ &keys[9], &values[9] },
// 	{ &keys[10], &values[10] },
// 	{ &keys[11], &values[11] },
// 	{ &keys[12], &values[12] },
// 	{ &keys[13], &values[13] },
// 	{ &keys[14], &values[14] },
// 	{ &keys[15], &values[15] },
// 	{ &keys[16], &values[16] },
// 	{ &keys[17], &values[17] },
// 	{ &keys[18], &values[18] }
// };
//
//有段错误的写法
// static c_pair pairs[] =
// {
// 	{12,8},	
// 	{4,8}	,
// 	{71,8},	
// 	{2,8}	,
// 	{90,8},	
// 	{99,8},
// 	{30,8},	
// 	{61,8},
// 	{29,8},	
// 	{84,8},	
// 	{6,8}	,
// 	{97,8},	
// 	{3,8}	,
// 	{8,8}	,
// 	{54,8},	
// 	{78,8},	
// 	{9,8}	,
// 	{16,8},	
// 	{62,8}	
// };


static int print_map(c_pmap pt)
{
	c_iterator iter = c_map_begin(pt);
	c_iterator end = c_map_end(pt);
	printf("map is:\n");
	for(; !ITER_EQUAL(iter, end); ITER_INC(iter))
	{
		printf("key = %d, value = %d\n",
			*(int*)((c_ppair)ITER_REF(iter))->first,
			*(int*)((c_ppair)ITER_REF(iter))->second);
	}
	return 0;
}


// static int create_with_insert_unique(c_pmap thiz)
// {
// 	int i = 0;
// 	for(; i < sizeof(keys) / sizeof(int); ++ i)
// 	{
// 		c_map_insert(thiz, &pairs[i]);
// 		assert(__c_rb_tree_verify(thiz->_l));
// 	}

// 	return 0;
// }

// int t_map()
// {
// 	c_map map;

// 	c_map_create(&map, int_comparer);
// 	assert(__c_rb_tree_verify(map._l));
// 	printf("0. test create with insert unique\n");
// 	create_with_insert_unique(&map);
// 	print_map(&map);
// 	assert(__c_rb_tree_verify(map._l));
// }
int main(){
	// t_map();//启发
	// c_map map;
	// c_map_create(&map, NULL);
	// assert(__c_rb_tree_verify(map._l));
	// printf("MIANBAOwrite---->\n");
	// int *firstkey=malloc(sizeof(int));
	// int *secondkey=malloc(sizeof(int));
	// int *firstvalue=1;
	// int *secondvalue=1;
	// printf("%d\n",&firstvalue );
	// printf("%d\n",&secondvalue );
	// *firstkey=1;
	// *secondkey=2;
	// c_pair a=c_make_pair(firstkey,&firstvalue);
	// c_pair b=c_make_pair(secondkey,&secondvalue);
	// c_map_insert(&map, &a);
	// printf("map_size:%d\n",c_map_size(&map) );
	// print_map(&map);
	// c_map_insert(&map, &b);
	// printf("map_size:%d\n",c_map_size(&map) );
	// print_map(&map);
	c_pair *p;
	p=malloc(sizeof(struct c_pair));
	return 0;
}