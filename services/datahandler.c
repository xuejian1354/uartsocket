/*
 * datahandler.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */

#include "datahandler.h"
#include "http_parser.h"
#include <debug/dlog.h>
#include <cJSON.h>

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

void dump_url (const char *url, const struct http_parser_url *u)
{
  unsigned int i;

  printf("\tfield_set: 0x%x, port: %u\n", u->field_set, u->port);
  for (i = 0; i < UF_MAX; i++) {
    if ((u->field_set & (1 << i)) == 0) {
      printf("\tfield_data[%u]: unset\n", i);
      continue;
    }

    printf("\tfield_data[%u]: off: %u, len: %u, part: %.*s\n",
           i,
           u->field_data[i].off,
           u->field_data[i].len,
           u->field_data[i].len,
           url + u->field_data[i].off);
  }
}

int message_begin_callback (http_parser* parser)
{
	return 0;
}

int url_callback (http_parser *parser, const char *at, size_t length)
{
	char *data = calloc(1, length+1);
	memcpy(data, at, length);
	parser->data = data;
}

int headers_complete_callback (http_parser* parser)
{
	return 0;
}

int message_complete_callback (http_parser* parser)
{
	return 0;
}

trbuf_t *http_parser_with_json_to_binary(trbuf_t *buf)
{
	http_parser_settings settings;
	settings.on_message_begin = message_begin_callback;
	settings.on_url = url_callback;
	settings.on_headers_complete = headers_complete_callback;
	settings.on_message_complete = message_complete_callback;
	http_parser *parser = malloc(sizeof(http_parser));
	http_parser_init(parser, HTTP_REQUEST);
	int nparsed = http_parser_execute(parser, &settings, buf->data, buf->len);

	int i;
	char *qdata = parser->data;
	int qlen = strlen(parser->data);
	int istart = -1;

	char fieldstr[4][16] = {0};
	int fieldpos;

	for(i=0; i<qlen; i++)
	{
		if(!strncmp(qdata+i, "opt=", 4))
		{
			fieldpos = 0;
			i += 4;
			istart = i;
		}
		else if(!strncmp(qdata+i, "devid=", 6))
		{
			fieldpos = 1;
			i += 6;
			istart = i;
		}
		else if(!strncmp(qdata+i, "type=", 5))
		{
			fieldpos = 2;
			i += 5;
			istart = i;
		}
		else if(!strncmp(qdata+i, "ctrl=", 5))
		{
			fieldpos = 3;
			i += 5;
			istart = i;
		}
		else if(*(qdata+i) == '&' || (i==qlen-1 && i++))
		{
			if(istart >= 0)
			{
				memcpy(fieldstr[fieldpos], qdata+istart, i-istart);
			}
			istart = -1;
		}
	}

	cJSON *pRoot = cJSON_CreateObject();
	for(i=0; i<4; i++)
	{
		if(strlen(fieldstr[i]) > 0)
		{
			switch(i)
			{
			case 0:
				cJSON_AddStringToObject(pRoot, "opt", fieldstr[0]);
				break;

			case 1:
				cJSON_AddStringToObject(pRoot, "devid", fieldstr[1]);
				break;

			case 2:
				printf("type=%s\n", fieldstr[i]);
				cJSON_AddStringToObject(pRoot, "type", fieldstr[2]);
				break;

			case 3:
				cJSON_AddStringToObject(pRoot, "ctrl", fieldstr[3]);
				break;
			}
		}
	}

	free(parser->data);


	char *frame = cJSON_Print(pRoot);


	trbuf_t *retbuf = calloc(1, sizeof(trbuf_t));
	retbuf->len = buf->len;
	retbuf->data = calloc(1, buf->len);
	//memcpy(retbuf->data, buf->data, buf->len);

	strcpy(retbuf->data, frame);
	cJSON_Delete(pRoot);

	return retbuf;
}

trbuf_t *binary_package_to_http_with_json(trbuf_t *buf)
{
	trbuf_t *retbuf = calloc(1, sizeof(trbuf_t));
	retbuf->len = buf->len;
	retbuf->data = calloc(1, buf->len);
	memcpy(retbuf->data, buf->data, buf->len);

	return retbuf;
}

