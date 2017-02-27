//
//  LC_serial.c
//  LocoControl
//
//  Created by Chris Draper on 5/06/15.
//  Copyright (c) 2015 Winter Creek. All rights reserved.
//

#include "LC_serial.h"


/*****************************************
 *
 * initSerial() - initialize the serial port - called from the DRIVER initialisation
 *
 *****************************************/

int initSerial()
{

	int mcs = TIOCM_RTS;

	//-------------------------
	//----- SETUP FOR USB CONNECTED ARDUINO - MODIFY FOR YOUR SETUP-----
	//-------------------------

	//OPEN THE DEVICE as specified by constant SERIAL_DEV - which should be something like "/dev/ttyAMA0" on a raspi
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.

	m_serialhandle = open(getConfigStr("SERIAL_DEVICE"), O_RDWR | O_NOCTTY | O_NDELAY );		//Open in non blocking read/write mode
	if (m_serialhandle < 0)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		fprintf(stderr,"Error opening serial port %s - %s(%d).\n", getConfigStr("SERIAL_DEVICE"), strerror(errno), errno);
		return 1;
	}

	atexit(closeSerial);   //ensure resources are disposed of when we exit

	// Note that open() follows POSIX semantics: multiple open() calls to
	// the same file will succeed unless the TIOCEXCL ioctl is issued.
	// This will prevent additional opens except by root-owned processes.
	// See tty(4) ("man 4 tty") and ioctl(2) ("man 2 ioctl") for details.

	if (ioctl(m_serialhandle, TIOCEXCL) == -1)
	{
		printf("Error setting TIOCEXCL on %s - %s(%d).\n",getConfigStr("SERIAL_DEVICE"), strerror(errno), errno);
		return 1;
	}

	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Supported Baud rates:- 9600, 19200, 38400, 57600, 115200 for this implementation
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	tcgetattr(m_serialhandle, &options);
	options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_cflag &= ~CRTSCTS; //disable hw flow control
	options.c_iflag &= ~(IXON | IXOFF | IXANY); //disable flow control	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //raw mode
	options.c_cc[VMIN] = 1;
	options.c_cc[VTIME] = 0;
	cfsetspeed(&options, getConfigSpeed());
	tcflush(m_serialhandle, TCIFLUSH);
	tcsetattr(m_serialhandle, TCSANOW, &options);

	//TODO - get BAUD RATE FROM CONFIG!

	//clear RTS
	ioctl(m_serialhandle, TIOCMBIC, &mcs);

	return 0;     //Port set up and ready to use.
}

/*****************************************
 *
 * closeSerial()
 *
 *****************************************/

void closeSerial(void)
{
    //Called automatically at exit DO NOT CALL FROM YOUR CODE
    if( m_serialhandle != -1)
    {
        close(m_serialhandle);
        fprintf(stderr,"Serial Closed OK\n");
    }
}

/*****************************************
 *
 * writeSerial()
 *
 *****************************************/

int writeSerial( const void* buf, int byteCount)
{

	int count = 0;

	if (m_serialhandle != -1)
	{
		count = (int) write(m_serialhandle, buf, byteCount);		//Filestream, bytes to write, number of bytes to write
		if (count < 0)
		{
			fprintf(stderr, "UART TX error - %s(%d).\n",strerror(errno),errno);
		}
	}
	return count;

}
/*****************************************
 *
 * readSerial()
 *
 *****************************************/


int readSerial(void* buf,const int MaxBytes)
{

	//Reads complete lines including the newline char from specified serial port
	//Does not wait if no bytes ready to be read.
	//returns the number of bytes read into the supplied buffer

	int rx_length = -1;

	//----- CHECK FOR ANY RX BYTES -----
	if (m_serialhandle != -1)
	{
		// Read up to maxbytes characters from the port if they are there
		rx_length = (int) read(m_serialhandle, buf, MaxBytes);		//Filestream, buffer to store in, number of bytes to read (max)
		if (rx_length < 0 && errno != 35)
		{
			fprintf(stderr,"Error reading from serial port - %s(%d).\n", strerror(errno),errno);
			rx_length = 0;
		}

	}

	return rx_length;
}

/*****************************************
 *
 * getSpeed()
 *
 *****************************************/

unsigned int getConfigSpeed(void)
{

//	Supported Baud rates:- 9600, 19200, 38400, 57600, 115200 for this implementation
//  swtich statement returns constants defined by termios and may be specific to target hardware
//  this was deemed safer than returning the codes from the Raspi implementation of termios
    unsigned int rate = 0;
    int           cfg  = getConfigVal("BAUD_RATE");


    switch(cfg)
    {

        case 9600:
            rate = B9600;
            break;
        case 19200:
            rate = B19200;
            break;
         case 38400:
            rate = B38400;
            break;
        case 57600:
            rate = B57600;
            break;
        case 115200:
            rate = B115200;
            break;
        default:
            fprintf(stderr,"BAUD_RATE %d in config not supported - use 9600,19200, 38400, 57600 or 115200.",cfg);
            rate = B9600;
            break;
    }

     fprintf(stderr,"BAUD_RATE set to %d",cfg);
     return rate;
}
