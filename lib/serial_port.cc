// Licence 2

#include <cstring>
#include <stdlib.h>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "include/dart_api.h"
#include "include/dart_native_api.h"
#include "native_helper.h"

Dart_NativeFunction ResolveName(Dart_Handle name, int argc);

Dart_Handle HandleError(Dart_Handle handle);


enum METHOD_CODE {
  OPEN = 1,
  CLOSE = 2,
  READ = 3,
  WRITE = 4
};

int selectBaudrate(int baudrate_speed){
  switch(baudrate_speed){
    // TODO baudrate 0 ? B0
    case 50: return B50; break;
    case 75: return B75; break;
    case 110: return B110; break;
    case 134: return B134; break;
    case 150: return B150; break;
    case 200: return B200; break;
    case 300: return B300; break;
    case 600: return B600; break;
    case 1200: return B1200; break;
    case 1800: return B1800; break;
    case 2400: return B2400; break;
    case 4800: return B4800; break;
    case 9600: return B9600; break;
    case 19200: return B19200; break;
    case 38400: return B38400; break;
    case 57600: return B57600; break;
    case 115200: return B115200; break;
    case 230400: return B230400; break;
    #ifdef B460800
    case 460800: return B460800;break;
    #endif
    #ifdef B500000
    case 500000: return B500000; break;
    #endif
    #ifdef B576000
    case 576000: return B576000; break;
    #endif
    #ifdef B921600
    case 921600: return B921600; break;
    #endif
    #ifdef B1000000
    case 1000000: return B1000000; break;
    #endif
    #ifdef B1152000
    case 1152000: return B1152000; break;
    #endif
    #ifdef B1500000
    case 1500000: return B1500000; break;
    #endif
    #ifdef B2000000
    case 2000000: return B2000000; break;
    #endif
    #ifdef B2500000
    case 2500000: return B2500000; break;
    #endif
    #ifdef B3000000
    case 3000000: return B3000000; break;
    #endif
    #ifdef B3500000
    case 3500000: return B3500000; break;
    #endif
    #ifdef B4000000
    case 4000000: return B4000000; break;
    #endif
    #ifdef B7200
    case 7200: return B7200; break;
    #endif
    #ifdef B14400
    case 14400: return B14400; break;
    #endif
    #ifdef B28800
    case 28800: return B28800; break;
    #endif
    #ifdef B76800
    case 76800: return B76800; break;
    #endif
    default: return -1;
  }
}

int selectDataBits(int dataBits) {
  switch (dataBits) {
    case 5: return CS5;
    case 6: return CS6;
    case 7: return CS7;
    case 8: return CS8;
    default: return -1;
  }
}

DECLARE_DART_NATIVE_METHOD(native_open){
  DECLARE_DART_RESULT;
  // TODO : macro validation nbr arg
  // TODO : get args macro
  const char* portname = GET_STRING_ARG(0);
  int64_t baudrate_speed = GET_INT_ARG(1);
  int64_t databits_nb = GET_INT_ARG(2);
  
  int baudrate = selectBaudrate(baudrate_speed);
  if(baudrate == -1){
     SET_ERROR("Invalid baudrate");
     RETURN_DART_RESULT;
  }

  int databits = selectDataBits(databits_nb);
  if(databits == -1) {
     SET_ERROR("Invalid databits");
     RETURN_DART_RESULT;
  }

  int tty_fd = open(portname, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if(tty_fd < 0){
    // TODO errno
    SET_ERROR("Invalid access");
  }
  struct termios tio;
  memset(&tio, 0, sizeof(tio));
  tio.c_iflag=0;
  tio.c_oflag= IGNPAR;
  tio.c_cflag= databits | CREAD | CLOCAL | HUPCL;
  tio.c_lflag=0;
  tio.c_cc[VMIN]=1;
  tio.c_cc[VTIME]=0;
  cfsetospeed(&tio, baudrate);
  cfsetispeed(&tio, baudrate);
  tcflush(tty_fd, TCIFLUSH);
  tcsetattr(tty_fd, TCSANOW, &tio);
  SET_RESULT_INT(tty_fd);
  
  RETURN_DART_RESULT;
}

DECLARE_DART_NATIVE_METHOD(native_close){
  DECLARE_DART_RESULT;  
  int64_t tty_fd = GET_INT_ARG(0);

  int value = close(tty_fd);
  if(value <0){
    // TODO errno
    SET_ERROR("Impossible to close");
    RETURN_DART_RESULT;    
  }
  SET_RESULT_BOOL(true);  

  RETURN_DART_RESULT;  
}

DECLARE_DART_NATIVE_METHOD(native_write){
  DECLARE_DART_RESULT; 

  int64_t tty_fd = GET_INT_ARG(0);

  // TODO int[]
  const char* data = GET_STRING_ARG(1);

  int value = write(tty_fd, data, strlen(data));
  if(value <0){
    // TODO errno
    SET_ERROR("Impossible to close");
    RETURN_DART_RESULT;
  }
  SET_RESULT_INT(value);

  RETURN_DART_RESULT; 
}

DECLARE_DART_NATIVE_METHOD(native_read){
  DECLARE_DART_RESULT;

  int64_t tty_fd = GET_INT_ARG(0);
  int buffer_size = (int) GET_INT_ARG(1);
  int8_t buffer[buffer_size];
  fd_set readfs;
  FD_ZERO(&readfs);
  FD_SET(tty_fd, &readfs);
  select(tty_fd+1, &readfs, NULL, NULL, NULL);
  int n =  read(tty_fd, &buffer, sizeof(buffer));
  if(n > 0){
    // TODO SET_INT_ARRAY_RESULT;
    // TODO try to not use malloc or use free
    current[1].type = Dart_CObject_kArray;
    current[1].value.as_array.length = n;
    for(int i=0; i<n; i++){
      //printf("%d => %d \n", i, buffer[i]);
      Dart_CObject* byte = (Dart_CObject*) malloc(sizeof(Dart_CObject_kInt32));
      byte->type = Dart_CObject_kInt32;
      byte->value.as_int32 = buffer[i];
      current[1].value.as_array.values[i] = byte;
    }

  }
  RETURN_DART_RESULT;
}

DISPATCH_METHOD()
  // TODO check args nb
  SWITCH_METHOD_CODE {
    case OPEN :
      CALL_DART_NATIVE_METHOD(native_open);
      break;
    case CLOSE:
      CALL_DART_NATIVE_METHOD(native_close);
      break;
    case WRITE:
      CALL_DART_NATIVE_METHOD(native_write);
      break;
    case READ:
      CALL_DART_NATIVE_METHOD(native_read);
    default:
     UNKNOW_METHOD_CALL;
     break;
  }
  /*
  } else if(method_code == READ) {
   int64_t tty_fd = argv[0]->value.as_int64;
   int buffer_size = (int) argv[1]->value.as_int64;
   int8_t buffer[buffer_size];
   fd_set readfs;
   FD_ZERO(&readfs);
   FD_SET(tty_fd, &readfs);
   select(tty_fd+1, &readfs, NULL, NULL, NULL);
   int n =  read(tty_fd, &buffer, sizeof(buffer));
   if(n > 0){

     result.type = Dart_CObject_kArray;
     result.value.as_array.length = n;

     for(int i=0; i<n; i++){
       Dart_CObject* byte = (Dart_CObject*) malloc(sizeof(Dart_CObject_kInt32));
       byte->type = Dart_CObject_kInt32;
       byte->value.as_int32 = buffer[i];
       result.value.as_array.values[i] = byte;
     }

    } else {
      result.type = Dart_CObject_kNull;
    }
  }
   else {

  }
*/

} 

DECLARE_LIB(serial_port, serialPortServicePort)