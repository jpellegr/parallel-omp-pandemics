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
#endif

#include "Defaults.h"
#include "Initialize.h"
#include "Infection.h"
#include "Core.h"
#include "Finalize.h"
#include <omp.h>

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

    double start_init=omp_get_wtime();
    /***************** In Initialize.h *****************/
    init(&global, &constant, &stats, &dpy, &argc, &argv);
    /***************************************************/
    double final_init=omp_get_wtime()- start_init;
    // printf("Initialization time:%lf\n", final_init);

    double start_core= omp_get_wtime();
    // Process starts a loop to run the simulation for the
    // specified number of days
    double move_total;
    double sus_total;
    double infected_total;
    double days_total;


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
        double start_move=omp_get_wtime();
        move(&global, &constant);
         double end_move=omp_get_wtime()- start_move;
        move_total = move_total + end_move;

        double start_sus=omp_get_wtime();
        susceptible(&global, &constant, &stats);
        double end_sus=omp_get_wtime()- start_sus;
        sus_total = sus_total + end_sus;

        double start_infected=omp_get_wtime();
        infected(&global, &constant, &stats);
        double end_infected=omp_get_wtime()- start_infected;
        infected_total = infected_total+ end_infected;

        double start_days=omp_get_wtime();
        update_days_infected(&global, &constant);
        double end_days=omp_get_wtime()- start_days;
        days_total = days_total + end_days;
        /**************************************/
    }
    // printf("Move time: %lf\n", move_total);
    printf("Sus time: %lf\n",sus_total);
    // printf("Infected time: %lf\n",infected_total);
    // printf("Update days time: %lf\n", days_total);






    double end_core=omp_get_wtime() - start_init;
    printf("%lf\t", end_core);

    /******** In Finialize.h ********/
    show_results(&global, &stats);

    cleanup(&global, &constant, &dpy);
    /********************************/


    exit(EXIT_SUCCESS);
}
