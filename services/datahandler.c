/*
 * datahandler.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */

#include "datahandler.h"
#include "http_parser.h"
#include <pthread.h>
#include <debug/dlog.h>
#include <cJSON.h>

static trbuf_t *http_parser_with_json_to_binary(trbuf_t *buf);
static trbuf_t *binary_package_to_http_with_json(trbuf_t *buf);
static trbuf_t *download_handler(cJSON *pCtrl);
static trbuf_t *syncdata_handler(trbuf_t *buf);


cJSON *pDataArray;
cJSON *pCmd;

pthread_mutex_t mutex;

dhandler_t datahandlers[] = {
 	{"http-switch-bin", http_parser_with_json_to_binary, binary_package_to_http_with_json}
};

float rand_data(char *data)
{
	static float lastxm = 0;
	float xm = 0.1 * rand() / RAND_MAX;
	float retm = atof(data)*(1+xm-lastxm);
	lastxm = xm;
	return retm;
}

void randdev_data(cJSON * obj)
{
	int i;
	char tmpval[64] = {0};
	char fieldarr[][8] = {"oxygen", "tem", "ph"};
	for(i=0; i<3; i++)
	{
		cJSON *pitem = cJSON_GetObjectItem(obj, fieldarr[i]);
		if(pitem)
		{
			memset(tmpval, 0, sizeof(tmpval));
			sprintf(tmpval, "%.2f", rand_data(pitem->valuestring));
			cJSON_DeleteItemFromObject(obj, fieldarr[i]);
			cJSON_AddStringToObject(obj, fieldarr[i], tmpval);
		}
	}
}

cJSON *cJSON_CopyObject(cJSON *item)
{
	char *frame = cJSON_Print(item);
	cJSON *retitem = cJSON_Parse(frame);
	free(frame);
	return retitem;
}

void devdata_init()
{
	pthread_mutex_init(&mutex, NULL);

	pCmd = NULL;
	pDataArray = cJSON_CreateArray();

	cJSON *pData1 = cJSON_CreateObject();
	cJSON_AddStringToObject(pData1, "devid", "0001");
	cJSON_AddStringToObject(pData1, "type", "1");
	cJSON_AddStringToObject(pData1, "oxygen", "3.23");
	cJSON_AddStringToObject(pData1, "tem", "26.3");
	cJSON_AddItemToArray(pDataArray, pData1);

	cJSON *pData2 = cJSON_CreateObject();
	cJSON_AddStringToObject(pData2, "devid", "0002");
	cJSON_AddStringToObject(pData2, "type", "2");
	cJSON_AddStringToObject(pData2, "ph", "7.08");
	cJSON_AddItemToArray(pDataArray, pData2);

	cJSON *pData3 = cJSON_CreateObject();
	cJSON_AddStringToObject(pData3, "devid", "0003");
	cJSON_AddStringToObject(pData3, "type", "3");
	cJSON_AddStringToObject(pData3, "power", "0");
	cJSON_AddItemToArray(pDataArray, pData3);
}

void devdata_release()
{
	cJSON_Delete(pDataArray);
	pthread_mutex_destroy(&mutex);
}

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
	/* HTTP Handler */
	http_parser parser;
	http_parser_settings settings;
	settings.on_message_begin = message_begin_callback;
	settings.on_url = url_callback;
	settings.on_headers_complete = headers_complete_callback;
	settings.on_message_complete = message_complete_callback;

	http_parser_init(&parser, HTTP_REQUEST);
	int nparsed = http_parser_execute(&parser, &settings, buf->data, buf->len);

	/* URL parser */
	int i;
	char *qdata = parser.data;
	int qlen = strlen(parser.data);
	int istart = -1;

	char fieldstr[4][16] = {0};
	int fieldpos;

	if(qdata)
	{
		for(i=0; i<=qlen; i++)
		{
			if(i==qlen || *(qdata+i) == '&' || (i==qlen-1 && i++))
			{
				if(istart >= 0)
				{
					memcpy(fieldstr[fieldpos], qdata+istart, i-istart);
				}
				istart = -1;
			}
			else if(!strncmp(qdata+i, "opt=", 4))
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
		}

		free(parser.data);
	}

	/* Json data transfer */
	cJSON *pCtrl = cJSON_CreateObject();
	for(i=0; i<4; i++)
	{
		if(strlen(fieldstr[i]) > 0)
		{
			switch(i)
			{
			case 0:
				cJSON_AddStringToObject(pCtrl, "opt", fieldstr[0]);
				break;

			case 1:
				cJSON_AddStringToObject(pCtrl, "devid", fieldstr[1]);
				break;

			case 2:
				cJSON_AddStringToObject(pCtrl, "type", fieldstr[2]);
				break;

			case 3:
				cJSON_AddStringToObject(pCtrl, "ctrl", fieldstr[3]);
				break;
			}
		}
	}

	/* Data Handler */
	pCmd = pCtrl;
	trbuf_t *retbuf = download_handler(pCmd);
	return retbuf;
}

trbuf_t *binary_package_to_http_with_json(trbuf_t *buf)
{
	int i;
	pthread_mutex_lock(&mutex);
	cJSON *pRetData = cJSON_CreateObject();

	cJSON *pcopt = cJSON_GetObjectItem(pCmd, "opt");
	if(pcopt
		&& (!strcmp(pcopt->valuestring, "query")
		  || !strcmp(pcopt->valuestring, "ctrl")))
	{
	}
	else
	{
		cJSON_AddNumberToObject(pRetData, "code", 1);
		cJSON_AddStringToObject(pRetData, "message", "fail");
		goto rethandler;
	}

	cJSON_AddNumberToObject(pRetData, "code", 0);
	cJSON_AddStringToObject(pRetData, "message", "success");

	cJSON *dataArray = cJSON_CreateArray();

	cJSON *pcdevid = cJSON_GetObjectItem(pCmd, "devid");
	cJSON *pctype = cJSON_GetObjectItem(pCmd, "type");

	if(pcdevid)
	{
		int cjsize = cJSON_GetArraySize(pDataArray);
		for(i=0; i<cjsize; i++)
		{
			cJSON *devdata = cJSON_GetArrayItem(pDataArray, i);
			if(!strcmp(pcdevid->valuestring,
				cJSON_GetObjectItem(devdata, "devid")->valuestring))
			{
				if(!pctype || !strcmp(pctype->valuestring,
					cJSON_GetObjectItem(devdata, "type")->valuestring))
				{
					cJSON *cpdevdata = cJSON_CopyObject(devdata);
					randdev_data(cpdevdata);

					cJSON_AddItemToArray(dataArray, cpdevdata);
				}
				break;
			}
		}

		cJSON_AddItemToObject(pRetData, "data", dataArray);
	}
	else if(pctype)
	{
		int cjsize = cJSON_GetArraySize(pDataArray);
		for(i=0; i<cjsize; i++)
		{
			cJSON *devdata = cJSON_GetArrayItem(pDataArray, i);
			if(pctype && !strcmp(pctype->valuestring,
				cJSON_GetObjectItem(devdata, "type")->valuestring))
			{
				cJSON_AddItemToArray(dataArray, cJSON_CopyObject(devdata));
				if(pcdevid)
				{
					break;
				}
			}
		}

		cJSON_AddItemToObject(pRetData, "data", dataArray);
	}
	else {
		cJSON_AddItemToObject(pRetData, "data", cJSON_CopyObject(pDataArray));
	}

rethandler:

	cJSON_Delete(pCmd);

	char *reframe = cJSON_Print(pRetData);
	int flen = 0;
	if(reframe)
	{
		flen = strlen(reframe);
	}

	char httphead[128] = {0};
	sprintf(httphead, "HTTP/1.1 200 OK\r\nContent-Length:%d\r\n\r\n", flen+2);

	trbuf_t *retbuf = calloc(1, sizeof(trbuf_t));
	retbuf->len = flen+strlen(httphead) + 2;
	retbuf->data = calloc(1, retbuf->len);

	if(reframe)
	{
		sprintf(retbuf->data, "%s%s\r\n", httphead, reframe);
		printf("%s%s\r\n", httphead, reframe);
		//free(reframe);
	}
	else
	{
		sprintf(retbuf->data, "%s\r\n", httphead);
	}

	cJSON_Delete(pRetData);
	pthread_mutex_unlock(&mutex);

	return retbuf;
}

trbuf_t *download_handler(cJSON *pCtrl)
{
	trbuf_t *retbuf = NULL;
	char *frame = cJSON_Print(pCtrl);
	if(frame)
	{
		retbuf = calloc(1, sizeof(trbuf_t));
		retbuf->len = strlen(frame);
		retbuf->data = calloc(1, retbuf->len);

		strcpy(retbuf->data, frame);
		free(frame);
	}

	return retbuf;
}

trbuf_t *syncdata_handler(trbuf_t *buf)
{
}

