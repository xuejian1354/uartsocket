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
#include <module/netapi.h>
#include <module/netlist.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SERIAL_SUPPORT

static int serial_fd;

static int isStart = 1;
static int mcount = 0;		//match frame head
static int step = 0;       //0,head; 1,fixed data; 2,data packet;
static int fixLen = 3;		//location when copy
static int dataLen = 0;
static uint8 tmpFrame[SERIAL_MAX_LEN]={0};

#ifdef UART_COMMBY_SOCKET
static int refd = -1;
static int reser_fd = -1;
#endif

//Open Serial Port
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


//Set Serial Port Params
int set_serial_params(int fd, uint32 speed, uint8 databit, uint8 stopbit, uint8 parity)
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
    switch(databit) {
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
    switch(stopbit) {
        case 0:
            options.c_cflag &= ~CSTOPB;
            break;
        case 1:
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

#ifdef UART_COMMBY_SOCKET
int get_uart_refd()
{
	return refd;
}
 
int get_reser_fd()
{
	return reser_fd;
}

int get_reser_accept(int fd)
{
	int rw;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	
	rw = accept(fd, (struct sockaddr *)&client_addr, &len);
	
#ifdef DE_PRINT_TCP_PORT
	trans_data_show(DE_TCP_ACCEPT, &client_addr, "", 0);
#endif

#ifdef TRANS_TCP_CONN_LIST
	tcp_conn_t *m_list;

	m_list = (tcp_conn_t *)malloc(sizeof(tcp_conn_t));
	m_list->fd = rw;
	m_list->tclient = RESER_TCLIENT;
	m_list->client_addr = client_addr;
	m_list->next = NULL;

	if (addto_tcpconn_list(m_list) < 0)
	{
		free(m_list);
		close(rw);
		return -1;
	}
#endif

#ifdef SELECT_SUPPORT
	select_set(rw);
#endif
	return 0;
}
#endif

int serial_init(char *dev)
{
	int fd;

#ifdef UART_COMMBY_SOCKET
	if ((refd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("client tcp socket fail");
		return -1;
	}

	struct sockaddr_in reserver_addr;
	reserver_addr.sin_family = PF_INET;
	reserver_addr.sin_port = htons(UART_REPORT);
	reserver_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(refd, (struct sockaddr *)&reserver_addr, sizeof(reserver_addr)) < 0)
	{
		close(refd);
		refd = -1;
#endif
		if ((fd=serial_open(dev)) < 0)
		{
			return -1;
		}
		serial_fd = fd;

		if (set_serial_params(fd, 115200, 8, 1, 0) < 0)
		{
			return -2;
		}

		write(fd, "(^_^)", 5);	//just enable serial port, no pratical meaning

#ifdef UART_COMMBY_SOCKET
		if ((reser_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		{
			perror("reser socket fail");
			return -3;
		}
		
		if (bind(reser_fd, (struct sockaddr *)&reserver_addr, sizeof(struct sockaddr)) < 0)
		{
			perror("bind reser ip fail");
			return -4;
		}
		
		listen(reser_fd, 5);
		
#ifdef SELECT_SUPPORT
		select_set(reser_fd);
#endif
	}
	else
	{
		DE_PRINTF(1, "Replace uart comm by new connection: fd=%d\n", refd);
	}
#endif

	pthread_t uartRead;
	pthread_create(&uartRead, NULL, uart_read_func, NULL);

	return 0;
}

int serial_write(char *data, int datalen)
{
#ifdef DE_PRINT_SERIAL_PORT
#ifdef DE_TRANS_UDP_STREAM_LOG
	if(get_deuart_flag())
	{
#endif
		DE_PRINTF(0, "serial write:%s\n", data);
		//PRINT_HEX(data, datalen);
#ifdef DE_TRANS_UDP_STREAM_LOG
	}
#endif
#endif
#ifdef UART_COMMBY_SOCKET
	if(refd >= 0)
	{
		return send(refd, data, datalen, 0);
	}
	else
#endif
	return write(serial_fd, data, datalen);
}

void *uart_read_func(void *p)
{
	unsigned char rbuf[64] = {0};
    int rlen = 0;
    int i = 0;
	
    while(isStart)
    {
        i = 0;
        memset(rbuf, 0, sizeof(rbuf));
#ifdef UART_COMMBY_SOCKET
		if(refd >= 0)
		{
			rlen = recv(refd, rbuf, sizeof(rbuf), 0);
			if(rlen <= 0)
			{
				DE_PRINTF(1, "Read uart from reser error, abort serial data handler\n");
				return NULL;
			}
		}
		else
#endif
		{
        	rlen = read(serial_fd, rbuf, sizeof(rbuf));
#ifdef UART_COMMBY_SOCKET
			if(rlen > 0 && get_tcp_conn_list()->p_head != NULL)
			{
				tcp_conn_t *t_list;	
				for(t_list=get_tcp_conn_list()->p_head; t_list!=NULL; t_list=t_list->next)
				{
					if(t_list->tclient == RESER_TCLIENT)
					{
						send(t_list->fd, rbuf, rlen, 0);
					}
				}
			}
#endif
		}

        while(i < rlen)
        {
            if(0 == step)
            {
                switch(rbuf[i])
                {
                case 'U':
				case 'D':
                    if(0 == mcount)
					{
						tmpFrame[0] = rbuf[i];
						mcount++;
                    }
                    else  
						mcount=0;
                    break;

                case 'C':
				case 'O':
				case 'H':
				case 'R':
                    if(1 == mcount)
					{
						tmpFrame[1] = rbuf[i];
						mcount++;
                    }
                    else
						mcount=0;
                    break;

                case ':':
					if(1 == mcount)
					{
                        step = 1;
						tmpFrame[1] = 'E';
						mcount++;
						tmpFrame[2] = rbuf[i];
						mcount++;
                    }
                    if(2 == mcount)
					{
                        step = 1;
						mcount++;
                        tmpFrame[2] = rbuf[i];
                    }
					else
						mcount = 0;
					break;
                
                default:
                    if(0 != mcount) 
                    {
						mcount=0;
                    }
                    break;
                }
            }
            else if(1 == step) 
            {
				if(fixLen > SERIAL_MAX_LEN)
				{
					fixLen = 3;
                	step = 0;
					mcount = 0;
					break;
				}
				
                *(tmpFrame+fixLen++) = rbuf[i];
                switch(rbuf[i])
                {
                case 0x3A:
                    if(3 == mcount)
					{
						mcount++;
                    }
                    else  
						mcount=3;
                    break;

                case 0x4F:
                    if(4 == mcount)
					{
						mcount++;
                    }
                    else
						mcount=3;
                    break;

                case 0x0D:
                    if(5 == mcount)
					{
                        mcount++;
                    }
					else
						mcount=3;
					break;

				case 0x0A:
					if(6 == mcount)
					{
						step = 2;
						goto serial_update;
					}
					
                default:
                    if(3 != mcount) 
                    {
						mcount=3;
                    }
                    break;
                }
            }
            else if(2 == step)
            {
serial_update:	
				dataLen = fixLen;
	            fixLen = 3;
                step = 0;
				mcount = 0;

				if(!memcmp("DE:", tmpFrame, 3))
				{
					 uint8 mFrame[SERIAL_MAX_LEN]={0};
					 memcpy(mFrame, "D:", 2);
					 memcpy(mFrame+2, tmpFrame+3, dataLen-3);
					 memcpy(tmpFrame, mFrame, dataLen-1);
					 tmpFrame[dataLen] = 0;
					 dataLen--;
				}
#ifdef DE_PRINT_SERIAL_PORT
#ifdef DE_TRANS_UDP_STREAM_LOG
				if(get_deuart_flag())
				{
#endif
					DE_PRINTF(0, "%s\nserial read:%s\n", get_time_head(), tmpFrame);
					//PRINT_HEX(tmpFrame, dataLen);
#ifdef DE_TRANS_UDP_STREAM_LOG
				}
#endif
				frhandler_arg_t *frarg;
#ifdef UART_COMMBY_SOCKET
				if(refd >= 0)
				{
					frarg = get_frhandler_arg_alloc(refd, NULL, (char *)tmpFrame, dataLen);
				}
				else
#endif
				{
					frarg = get_frhandler_arg_alloc(serial_fd, NULL, (char *)tmpFrame, dataLen);
				}
#ifdef THREAD_POOL_SUPPORT
				//tpool_add_work(analysis_zdev_frame, frarg, TPOOL_LOCK);
#else
				//analysis_zdev_frame(frarg);
#endif
				memset(tmpFrame, 0, sizeof(tmpFrame));
#endif
            }
            i++;
        }
    }
}
#endif

#ifdef __cplusplus
}
#endif
