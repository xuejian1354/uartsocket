/*
 * datahandler.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */

#include "datahandler.h"

static trbuf_t *http_parser_with_json_to_binary(trbuf_t *buf);
static trbuf_t *binary_package_to_http_with_json(trbuf_t *buf);


dhandler_t datahandlers[] = {
 	{"http-switch-bin", http_parser_with_json_to_binary, binary_package_to_http_with_json}
};


dhandler_t *get_datahandler(char *name)
{
	int i;
	for(i=0; i<sizeof(datahandlers); i++)
	{
		if(!strcmp(name, datahandlers[i].name))
		{
			return datahandlers+i;
		}
	}

	return NULL;
}


trbuf_t *http_parser_with_json_to_binary(trbuf_t *buf)
{
	trbuf_t *retbuf = calloc(1, sizeof(trbuf_t));
	retbuf->len = buf->len;
	memcpy(retbuf->data, buf->data, buf->len);

	return retbuf;
}

trbuf_t *binary_package_to_http_with_json(trbuf_t *buf)
{
	trbuf_t *retbuf = calloc(1, sizeof(trbuf_t));
	retbuf->len = buf->len;
	memcpy(retbuf->data, buf->data, buf->len);

	return retbuf;
}

