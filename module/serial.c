/*
 * serial.c
 *
 * Sam Chen <xuejian1354@163.com>
 *
 */
#include "serial.h"
#include <termios.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <module/netlist.h>
#include <services/datahandler.h>

#ifdef __cplusplus
extern "C" {
#endif

serial_dev_t *g_serial_dev = NULL;

static int serial_open(char *dev);
static int set_serial_params(int fd, uint32 speed, uint8 databits, uint8 stopbits, uint8 parity);

static void *uart_read_handler(void *p);
static void *tcpserver_accept_handler(void *p);
static void *tcpserver_read_handler(void *p);
static void *tcpclient_read_handler(void *p);
static void *udpserver_read_handler(void *p);
static void *udpclient_read_handler(void *p);

serial_dev_t *get_serial_dev()
{
	return g_serial_dev;
}

int add_serial_dev(serial_dev_t *t_serial_dev)
{
	serial_dev_t *pre_dev = NULL;
	serial_dev_t *t_dev = g_serial_dev;

	if(t_serial_dev == NULL)
	{
		return -1;
	}
	else
	{
		t_serial_dev->next = NULL;
	}

	while(t_dev != NULL)
	{
		if(strcmp(t_dev->dev, t_serial_dev->dev))
		{
			pre_dev = t_dev;
			t_dev = t_dev->next;
		}
		else
		{
			return 1;
		}
	}

	t_serial_dev->next = g_serial_dev;
	g_serial_dev = t_serial_dev;

	return 0;
}

serial_dev_t *query_serial_dev(char *dev)
{
	serial_dev_t *t_sesial_dev = g_serial_dev;

	while(t_sesial_dev != NULL)
	{
		if(strcmp(t_sesial_dev->dev, dev))
		{
			t_sesial_dev = t_sesial_dev->next;
		}
		else
		{
			return t_sesial_dev;
		}
	}

	return NULL;
}

int del_serial_dev(char *dev)
{
	serial_dev_t *pre_serial_dev = NULL;
	serial_dev_t *t_serial_dev = g_serial_dev;

	while(t_serial_dev != NULL)
	{
		if(strcmp(t_serial_dev->dev, dev))
		{
			pre_serial_dev = t_serial_dev;
			t_serial_dev = t_serial_dev->next;
		}
		else
		{
			if(pre_serial_dev != NULL)
			{
				pre_serial_dev->next = t_serial_dev->next;
			}
			else
			{
				g_serial_dev = t_serial_dev->next;
			}

			free(t_serial_dev);
			return 0;
		}
	}

	return -1;
}

void serial_dev_free()
{
	serial_dev_t *m_serial_dev = g_serial_dev;
	while(m_serial_dev != NULL)
	{
		serial_dev_t *t_serial_dev = m_serial_dev;
		m_serial_dev = m_serial_dev->next;
		session_free(t_serial_dev->session);
		free(t_serial_dev);
	}

	g_serial_dev = NULL;
}

int serial_init(trsess_t *session)
{
	if(session == NULL)
	{
		return -1;
	}

	if(!session->enabled)
	{
		return 1;
	}

	serial_dev_t *m_serial_dev = query_serial_dev(session->dev);
	if(m_serial_dev == NULL)
	{
		serial_dev_t *t_serial_dev = calloc(1, sizeof(serial_dev_t));
		strcpy(t_serial_dev->dev, session->dev);
		t_serial_dev->speed = session->speed;
		t_serial_dev->num = 0;
		t_serial_dev->session = NULL;
		if(add_serial_dev(t_serial_dev) != 0)
		{
			free(t_serial_dev);
			t_serial_dev = query_serial_dev(session->dev);
		}

		m_serial_dev = t_serial_dev;

		if ((t_serial_dev->serial_fd=serial_open(session->dev)) < 0
			|| set_serial_params(t_serial_dev->serial_fd, t_serial_dev->speed, 8, 2, 0) < 0)
		{
			del_serial_dev(session->dev);
			return -2;
		}

		int ret = write(t_serial_dev->serial_fd, "(^_^)", 5);	//just enable serial port, no pratical meaning

		pthread_t uartRead;
		pthread_create(&uartRead, NULL, uart_read_handler, (void *)t_serial_dev);
	}

	trsess_t *t_session = calloc(1, sizeof(trsess_t));
	memcpy(t_session, session, sizeof(trsess_t));
	t_session->parent = m_serial_dev;

	int ret = add_trans_session(&m_serial_dev->session, t_session);
	if(ret != 0)
	{
		free(t_session);
		t_session = query_trans_session(m_serial_dev->session, session->name);
	}
	else
	{
		m_serial_dev->num++;

		if(t_session->tocol == UT_TCP)
		{
			if(t_session->mode == UM_MASTER)
			{
				struct sockaddr_in reserver_addr;
				reserver_addr.sin_family = PF_INET;
				reserver_addr.sin_port = htons(t_session->port);
				reserver_addr.sin_addr.s_addr = htonl(INADDR_ANY);

				if ((t_session->refd = socket(PF_INET, SOCK_STREAM, 0)) < 0
					|| bind(t_session->refd, (struct sockaddr *)&reserver_addr, sizeof(struct sockaddr)) < 0)
				{
					perror("reser socket fail");
					return -3;
				}

				listen(t_session->refd, 5);

				pthread_t reserAccept;
				pthread_create(&reserAccept, NULL, tcpserver_accept_handler, (void *)t_session);
			}
			else if(t_session->mode == UM_SLAVE)
			{
				struct sockaddr_in reclient_addr;
				reclient_addr.sin_family = PF_INET;
				reclient_addr.sin_port = htons(t_session->port);
				reclient_addr.sin_addr.s_addr = t_session->ip;

				if ((t_session->refd = socket(PF_INET, SOCK_STREAM, 0)) < 0
					|| connect(t_session->refd, (struct sockaddr *)&reclient_addr, sizeof(reclient_addr)) < 0)
				{
					perror("recli socket fail");
					return -4;
				}

				pthread_t recliAccept;
				pthread_create(&recliAccept, NULL, tcpclient_read_handler, (void *)t_session);
			}
		}
		else if(t_session->tocol == UT_UDP)
		{
			if(t_session->mode == UM_MASTER)
			{}
			else if(t_session->mode == UM_SLAVE)
			{}
		}
	}

	return 0;
}

void *uart_read_handler(void *p)
{
	unsigned char rbuf[2048];
	int rlen;

	serial_dev_t *m_serial_dev = (serial_dev_t *)p;
	
    while(1)
    {
        memset(rbuf, 0, sizeof(rbuf));
    	rlen = read(m_serial_dev->serial_fd, rbuf, sizeof(rbuf));
		trsess_t *m_session = m_serial_dev->session;
		while(m_session != NULL)
		{
			trbuf_t buffer;
			buffer.data = rbuf;
			buffer.len = rlen;

			trbuf_t *senbuf = NULL;
			dhandler_t *phandler = get_datahandler(m_session->haname);
			if(phandler)
			{
				senbuf = phandler->sendcall(&buffer);
			}
			else
			{
				senbuf = calloc(1, sizeof(trbuf_t));
				senbuf->len = rlen;
				senbuf->data = malloc(rlen);
				memcpy(senbuf->data, rbuf, rlen);
			}

			if(senbuf)
			{
				if(m_session->tocol == UT_TCP && m_session->mode == UM_MASTER)
				{
					tcp_conn_t *t_conn = (tcp_conn_t *)m_session->arg;
					while(t_conn != NULL)
					{
						int ret = write(t_conn->fd, senbuf->data, senbuf->len);
						t_conn = t_conn->next;
					}
				}
				else if(m_session->tocol == UT_TCP && m_session->mode == UM_SLAVE)
				{
					int ret = write(m_session->refd, senbuf->data, senbuf->len);
				}

				free(senbuf->data);
			}

			free(senbuf);
			m_session = m_session->next;
		}
    }
}

void *tcpserver_accept_handler(void *p)
{
	int rw;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	trsess_t *m_session = (trsess_t *)p;

	while(1)
	{
		rw = accept(m_session->refd, (struct sockaddr *)&client_addr, &len);

		tcp_conn_t *t_conn = calloc(1, sizeof(tcp_conn_t));
		t_conn->fd = rw;
		t_conn->client_addr = client_addr;
		t_conn->parent = p;
		t_conn->next = NULL;

		if(addto_tcpconn_list((tcp_conn_t **)&m_session->arg, t_conn) != 0)
		{
			free(t_conn);
			t_conn = queryfrom_tcpconn_list((tcp_conn_t *)m_session->arg, rw);
		}
		
		pthread_t reserRead;
		pthread_create(&reserRead, NULL, tcpserver_read_handler, (void *)t_conn);
	}
}

void *tcpserver_read_handler(void *p)
{
	int nbytes;
	char buf[2048];

	tcp_conn_t *m_conn = (tcp_conn_t *)p;
	trsess_t *m_session = (trsess_t *)m_conn->parent;
	int isStart = 1;

	while(isStart)
	{
		memset(buf, 0, sizeof(buf));
	   	if ((nbytes = recv(m_conn->fd, buf, sizeof(buf), 0)) <= 0)
	   	{
	      		close(m_conn->fd);
			delfrom_tcpconn_list((tcp_conn_t **)&((trsess_t *)m_conn->parent)->arg, m_conn->fd);
			isStart = 0;
		}
		else
		{
			trbuf_t buffer;
			buffer.data = buf;
			buffer.len = nbytes;
			trbuf_t *recbuf = NULL;
			dhandler_t *phandler = get_datahandler(m_session->haname);
			if(phandler)
			{
				recbuf = phandler->recvcall(&buffer);

				/*-----------------------------------------*/
				trbuf_t *senbuf = phandler->sendcall(recbuf);
				if(senbuf)
				{
					write(m_conn->fd, senbuf->data, senbuf->len);
					free(senbuf->data);
				}
				free(senbuf);
				/*-----------------------------------------*/
			}
			else
			{
				recbuf = calloc(1, sizeof(trbuf_t));
				recbuf->len = nbytes;
				recbuf->data = malloc(nbytes);
				memcpy(recbuf->data, buf, nbytes);
			}

			if(recbuf)
			{

				write(((serial_dev_t *)m_session->parent)->serial_fd, recbuf->data, recbuf->len);
				free(recbuf->data);
			}

			free(recbuf);
		}
	}
}

void *tcpclient_read_handler(void *p)
{
	trsess_t *m_session = (trsess_t *)p;
	if(m_session == NULL)
	{
		return NULL;
	}

	int nbytes;
	char buf[2048];

	int isStart = 1;

	while(isStart)
	{
		memset(buf, 0, sizeof(buf));
		if ((nbytes = read(m_session->refd, buf, sizeof(buf))) <= 0)
		{
			close(m_session->refd);
			isStart = 0;
		}
		else
		{
			trbuf_t buffer;
			buffer.data = buf;
			buffer.len = nbytes;

			trbuf_t *recbuf = NULL;
			dhandler_t *phandler = get_datahandler(m_session->haname);
			if(phandler)
			{
				recbuf = phandler->recvcall(&buffer);
			}
			else
			{
				recbuf = calloc(1, sizeof(trbuf_t));
				recbuf->len = nbytes;
				recbuf->data = malloc(nbytes);
				memcpy(recbuf->data, buf, nbytes);
			}

			if(recbuf)
			{
				write(((serial_dev_t *)(m_session->parent))->serial_fd, recbuf->data, recbuf->len);
				free(recbuf->data);
			}

			free(recbuf);
		}
	}
}

void *udpserver_read_handler(void *p)
{}

void *udpclient_read_handler(void *p)
{}

int serial_open(char *dev)
{
	int fd;
	if(NULL == dev)
	{
		perror("Can't open serial port");
		return -1;
	}
	
    fd = open(dev, O_RDWR | O_NOCTTY);

	if(fd < 0)
	{
		perror("Can't open serial port");
	}
	
    return fd;
}

int set_serial_params(int fd, uint32 speed, uint8 databits, uint8 stopbits, uint8 parity)
{
    int iSpeed = 0;
    struct termios options;
    tcgetattr(fd, &options);

    cfmakeraw(&options);
    //Set Baudrate
    switch(speed) {
        case 38400:
            iSpeed = B38400;
            break;
        case 19200:
            iSpeed = B19200;
            break;
        case 115200:
            iSpeed = B115200;
            break;
		case 57600:
			iSpeed = B57600;
			break;
        case 9600:
            iSpeed = B9600;
            break;
        case 4800:
            iSpeed = B4800;
            break;
        case 2400:
            iSpeed = B2400;
            break;
        case 1200:
            iSpeed = B1200;
            break;
        case 300:
            iSpeed = B300;
            break;
        default:
            perror("Unsupport Baudrate");
            return -1;
    }
    cfsetispeed(&options,iSpeed);
    cfsetospeed(&options,iSpeed);

    //  Set DataBits
    options.c_cflag &= ~CSIZE;
    switch(databits) {
        case 5:
          options.c_cflag |= CS5;
          break;
        case 6:
            options.c_cflag |= CS6;
            break;
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            perror("Unsupported data size");
            return -1;
    }
    //  Set Parity
    switch(parity) {
        case 0:
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case 1:
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
            break;
        case 2:
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        default:
            perror("Unsupported Parity");
            return -1;
    }

    // Set Stop Bits
    switch(stopbits) {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            perror("Unsupported Stop Bits");
            return -1;
    }


    options.c_lflag &= ~(ICANON | ECHO | ECHONL | ECHOE | ISIG | IEXTEN);
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);

    if(tcsetattr(fd, TCSANOW, &options) != 0) {
        perror("Tcsetattr Fail");
        return -1;
    }

    tcflush(fd, TCIFLUSH);
    return 0;
}

#ifdef __cplusplus
}
#endif
