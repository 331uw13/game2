#include <stdlib.h>

#include "memory.h"



void freeif(void* ptr) {
    if(ptr) {
        free(ptr);
    }
}


