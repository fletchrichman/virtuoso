#include <iostream> 
#include <fcntl.h> 
#include <termios.h> 

#define USB_SERIAL_PORT "/dev/tty.RNBT-0A07-RNI-SPP" // (OSX)

int port_fd; 
int init_serial_input (char * port) {   
  int fd = 0;   
  struct termios options;   

  fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);   
  if (fd == -1)     
    return fd;   
    fcntl(fd, F_SETFL, 0);    // clear all flags on descriptor, enable direct I/O   
    tcgetattr(fd, &options);   // read serial port options   
    // enable receiver, set 8 bit data, ignore control lines   
    options.c_cflag |= (CLOCAL | CREAD | CS8);    
    // disable parity generation and 2 stop bits   
    options.c_cflag &= ~(PARENB | CSTOPB);  
    // set the new port options   
    tcsetattr(fd, TCSANOW, &options);      
    return fd; 
} 

int main() {

  port_fd = init_serial_input(USB_SERIAL_PORT);                 
  // serial is checked              
  if (port_fd == -1)                 
  std::cout << "Couldn't open USB port" << std::endl; 
  else 
  std::cout << port_fd << std::endl;             
}