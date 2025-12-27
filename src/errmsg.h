#ifndef ERRMSG_H
#define ERRMSG_H


#define ERRMSGBUF_MAX_SIZE 512

void errmsg_ex
(
    const char* caller_file,
    const int   caller_file_ln,
    const char* caller_func,
    const char* message,
    ...
);

#define eprintf(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__)
#define errmsg(msg, ...) errmsg_ex(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)



#endif
