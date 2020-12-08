#include "SerialUtils.hpp"

#include <stdio.h>
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions 	   */ 
#include <errno.h>   /* ERROR Number Definitions           */
#include <string>
#include <iostream>
#include <vector>

using namespace BlockNDN;


SerialUtils::SerialUtils(){};

std::string SerialUtils::SerialRead(int len){

    int fd;/*File Descriptor*/
		
    //printf("\n +----------------------------------+");
    // printf("\n |        Serial Port Read          |");
    // printf("\n +----------------------------------+");

    /*------------------------------- Opening the Serial Port -------------------------------*/

    /* Change /dev/ttyUSB0 to the one corresponding to your system */

        fd = open(this->port,O_RDWR | O_NOCTTY);	/* ttyUSB0 is the FT232 based USB2SERIAL Converter   */
                            /* O_RDWR   - Read/Write access to serial port       */
                            /* O_NOCTTY - No terminal will control the process   */
                            /* Open in blocking mode,read will wait              */
                                
                                                                        
                                
        if(fd == -1)						/* Error Checking */
                printf("\n  Error! in Opening  ..");
        // else
        //         printf("\n  ttyUSB0 Opened Successfully ");


    /*---------- Setting the Attributes of the serial port using termios structure --------- */
    
    struct termios SerialPortSettings;	/* Create the structure                          */

    tcgetattr(fd, &SerialPortSettings);	/* Get the current attributes of the Serial port */

    /* Setting the Baud rate */
    cfsetispeed(&SerialPortSettings,B115200); /* Set Read  Speed as 9600                       */
    cfsetospeed(&SerialPortSettings,B115200); /* Set Write Speed as 9600                       */

    /* 8N1 Mode */
    SerialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
    SerialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
    SerialPortSettings.c_cflag &= ~CSIZE;	 /* Clears the mask for setting the data size             */
    SerialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */
    
    SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
    SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */ 
    
    
    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
    SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */

    SerialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/
    
    /* Setting Time outs */
    SerialPortSettings.c_cc[VMIN] = 10; /* Read at least 10 characters */
    SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */


    if((tcsetattr(fd,TCSANOW,&SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
        printf("\n  ERROR ! in Setting attributes");
    // else
    //             printf("\n  BaudRate = 115200 \n  StopBits = 1 \n  Parity   = none");
        
        /*------------------------------- Read data from serial port -----------------------------*/

    //tcflush(fd, TCIFLUSH);   /* Discards old data in the rx buffer            */                                  //just removed this ...

    char read_buffer[len];   /* Buffer to store the data received              */
    int  bytes_read = 0;    /* Number of bytes read by the read() system call */
    int i = 0;

    //std::cout << "Start reading..." << std::endl;
    bytes_read = read(fd,&read_buffer,len); /* Read the data                   */
        
    //printf("\n\n  Bytes Rxed -%d", bytes_read); /* Print the number of bytes read */
    //printf("\n\n  ");

    //std::cout << "---READING START----" << std::endl;

    //for(i=0;i<bytes_read;i++)	 /*printing only the received characters*/
    //    printf("%c",read_buffer[i]);
    // std::cout << "---READING END----" << std::endl;
    //printf("\n +----------------------------------+\n\n\n");

    std::string sb(read_buffer);

    close(fd); /* Close the serial port */

    return sb;
}


void SerialUtils::SerialWrite(char write_buffer[], size_t len){
    //std::cout << "HEEEEELLLO: " << prefix << std::endl;
    int fd;/*File Descriptor*/
		
    // printf("\n +----------------------------------+");
    // printf("\n |        Serial Port Write         |");
    // printf("\n +----------------------------------+");

    /*------------------------------- Opening the Serial Port -------------------------------*/

    /* Change /dev/ttyUSB0 to the one corresponding to your system */

        fd = open(this->port,O_RDWR | O_NOCTTY | O_NDELAY);	/* ttyUSB0 is the FT232 based USB2SERIAL Converter   */
                                /* O_RDWR Read/Write access to serial port           */
                                /* O_NOCTTY - No terminal will control the process   */
                                /* O_NDELAY -Non Blocking Mode,Does not care about-  */
                                /* -the status of DCD line,Open() returns immediatly */                                        
                                
        if(fd == -1)						/* Error Checking */
                printf("\n  Error! in Opening ");
        // else
        //         printf("\n  ttyUSB2 Opened Successfully ");


    /*---------- Setting the Attributes of the serial port using termios structure --------- */
    
    struct termios SerialPortSettings;	/* Create the structure                          */

    tcgetattr(fd, &SerialPortSettings);	/* Get the current attributes of the Serial port */

    cfsetispeed(&SerialPortSettings,B115200); /* Set Read  Speed as 9600                       */
    cfsetospeed(&SerialPortSettings,B115200); /* Set Write Speed as 9600                       */

    SerialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
    SerialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
    SerialPortSettings.c_cflag &= ~CSIZE;	 /* Clears the mask for setting the data size             */
    SerialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */

    SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
    SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */ 
    
    
    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
    SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */

    SerialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/

    if((tcsetattr(fd,TCSANOW,&SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
        printf("\n  ERROR ! in Setting attributes");
    //else
                //printf("\n  BaudRate = 115200 \n  StopBits = 1 \n  Parity   = none");
        
        /*------------------------------- Write data to serial port -----------------------------*/

    //std::string asdf = "HAFSTEINN";
    //std::cout << asdf.size() << std::endl;

    // Suggestion.. send info as string of characters such that:
    // (int)Prefix | (int)View | (int)Number | (char)Hash

    //char write_buffer[] = prefix[];	/* Buffer containing characters to write into port	     */	
    int  bytes_written  = 0;  	/* Value for storing the number of bytes written to the port */ 

    //std::cout << "THE SIZE IS: " << write_buffer.size() << std::endl;
    tcflush(fd, TCIFLUSH);   /* Discards old data in the rx buffer            */
    bytes_written = write(fd,write_buffer,len);/* use write() to send data to port                                            */
                                        /* "fd"                   - file descriptor pointing to the opened serial port */
                                        /*	"write_buffer"         - address of the buffer containing data	            */
                                        /* "sizeof(write_buffer)" - No of bytes to write                               */
    
    // char read_buffer[1];   /* Buffer to store the data received              */
    // int  bytes_read = 0;    /* Number of bytes read by the read() system call */
    // bytes_read = read(fd,&read_buffer,1); /* Read the data                   */

    // printf("\n  %s written to ttyUSB0",write_buffer);
    // printf("\n  %d Bytes written to ttyUSB0", bytes_written);
    // printf("\n +----------------------------------+\n\n");

    

    close(fd);/* Close the Serial port */
}
