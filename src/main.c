#include <stdio.h>


#include "state.h"




int main() {
    struct gstate* gst = gstate_init();
    if(gst) {
        gstate_rungame(gst);
    }
    else {
        return 1;
    }
    int exit_code = gst->exit_code;
    free_gstate(gst);
    return exit_code;
}



