#include <stdio.h>


#include "state.h"




int main() {
    struct gstate* gst = gstate_init();
    if(!gst) {
        return 1;
    }
  
    gstate_rungame(gst);
    free_gstate(gst);
    return 0;
}



