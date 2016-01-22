/*
 * strings_t.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#include "strings_t.h"

#ifdef __cplusplus
extern "C"
{
#endif

strings_t *strings_alloc(unsigned int size)
{
	if(size == 0)
	{
		return NULL;
	}

	strings_t *strs = (strings_t *)calloc(1, sizeof(strings_t));
	strs->size = size;
	strs->str = (char **)calloc(size, sizeof(char *));
	return strs;
}

int strings_add(strings_t *strs, char *str)
{
	int i;
	if(strs == NULL || strs->str == NULL || strs->size == 0)
	{
		return -1;
	}

	for(i=0; i<strs->size; i++)
	{
		if(*(strs->str+i) == NULL)
		{
			break;
		}
	}

	if(i == strs->size)
	{
		return -1;
	}

	int tlen = strlen(str);
	*(strs->str+i )= (char *)calloc(tlen+1, sizeof(char));;
	strncpy(*(strs->str+i), str, tlen);

	return 0;
}

void strings_free(strings_t *strs)
{
	int i = 0;

	if(strs == NULL)
	{
		return;
	}

	if(strs->str == NULL)
	{
		free(strs);
		return;
	}

	while(i < strs->size)
	{
		free(*(strs->str+i));
		i++;
	}

	free(strs->str);
	free(strs);
}

#ifdef __cplusplus
}
#endif
