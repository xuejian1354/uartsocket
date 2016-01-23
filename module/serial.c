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

int serial_init(char *dev)
{
	int fd;
	int ret;

	if ((fd=serial_open(dev)) < 0)
	{
		return -1;
	}
	serial_fd = fd;

	if (set_serial_params(fd, 115200, 8, 1, 0) < 0)
	{
		return -2;
	}

	ret = write(fd, "(^_^)", 5);	//just enable serial port, no pratical meaning

	pthread_t uartRead;
	pthread_create(&uartRead, NULL, uart_read_func, NULL);

	return 0;
}

int serial_write(char *data, int datalen)
{
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
		rlen = read(serial_fd, rbuf, sizeof(rbuf));

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
				AI_PRINTF("%s\nserial read:%s\n", get_time_head(), tmpFrame);
				//PRINT_HEX(tmpFrame, dataLen);
				frhandler_arg_t *frarg;
				frarg = get_frhandler_arg_alloc(serial_fd, NULL, (char *)tmpFrame, dataLen);
#ifdef THREAD_POOL_SUPPORT
				//tpool_add_work(analysis_zdev_frame, frarg, TPOOL_LOCK);
#else
				//analysis_zdev_frame(frarg);
#endif
				memset(tmpFrame, 0, sizeof(tmpFrame));
            }
            i++;
        }
    }
}
#endif

#ifdef __cplusplus
}
#endif
