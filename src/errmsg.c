#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>

#include "errmsg.h"

void errmsg_ex
(
    const char* caller_file,
    const int   caller_file_ln,
    const char* caller_func,
    const char* message,
    ...
){
    va_list args;
    va_start(args, message);

    char buffer[ERRMSGBUF_MAX_SIZE+1] = { 0 };
    
    size_t size = snprintf(buffer, ERRMSGBUF_MAX_SIZE,
            "(ERROR) [%s:%i] %s(): ",
            caller_file,
            caller_file_ln,
            caller_func);

    size += vsnprintf(buffer + size, ERRMSGBUF_MAX_SIZE,
            message,
            args);




    write(STDOUT_FILENO, buffer, size);
    write(STDOUT_FILENO, "\n", 1);
    
    va_end(args);
}
