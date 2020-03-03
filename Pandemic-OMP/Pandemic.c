/* Parallelization: Infectious Disease
 * By Aaron Weeden, Shodor Education Foundation, Inc.
 * November 2011
 * Modified by Yu Zhao, Macalester College.
 * July 2013
 * (Modularized and restructured the original code) */

#include <stdio.h>      // for printf
#include <stdlib.h>     // for malloc, free, and various others
#include <time.h>       // for time is used to seed the random number generator
#include <unistd.h>     // for random, getopt, some others
#ifdef X_DISPLAY
#include <X11/Xlib.h>   // for X display
#include <omp.h>
#endif

#include "Defaults.h"
#include "Initialize.h"
#include "Infection.h"
#include "Core.h"
#include "Finalize.h"

#if defined(X_DISPLAY) || defined(TEXT_DISPLAY)
#include "Display.h"
#endif

int main(int argc, char ** argv)
{
    /**** In Defaults.h ****/
    struct global_t global;
    struct const_t constant;
    struct stats_t stats;
    struct display_t dpy;
    /***********************/

    double start_time=omp_get_wtime();
    /***************** In Initialize.h *****************/
    init(&global, &constant, &stats, &dpy, &argc, &argv);
    /***************************************************/

    // Process starts a loop to run the simulation for the
    // specified number of days
    for(global.current_day = 0; global.current_day <= constant.total_number_of_days;
        global.current_day++)
    {
        /****** In Infection.h ******/
        find_infected(&global);
        /****************************/

        /**************** In Display.h *****************/
        #if defined(X_DISPLAY) || defined(TEXT_DISPLAY)

        do_display(&global, &constant, &dpy);

        throttle(&constant);

        #endif
        /***********************************************/

        /************** In Core.h *************/
        move(&global, &constant);

        susceptible(&global, &constant, &stats);

        infected(&global, &constant, &stats);

        update_days_infected(&global, &constant);
        /**************************************/
    }

    /******** In Finialize.h ********/
    show_results(&global, &stats);

    cleanup(&global, &constant, &dpy);
    /********************************/
    double end_time = omp_get_wtime();
    printf("%lf\t",end_time-start_time);
    exit(EXIT_SUCCESS);

}
