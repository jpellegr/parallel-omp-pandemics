/* Parallelization: Infectious Disease
 * By Aaron Weeden, Shodor Education Foundation, Inc.
 * November 2011
 * Modified by Yu Zhao, Macalester College.
 * July 2013
 * (Modularized and restructured the original code) */

#ifndef PANDEMIC_INITIALIZE_H
#define PANDEMIC_INITIALIZE_H

#include <stdlib.h>     // for malloc, and various others
#include <unistd.h>     // for random, getopt, some others
#include <time.h>       // for time is used to seed the random number generator
#include <omp.h>       // OpenMP

#include <trng/lcg64_shift.hpp>
#include <trng/yarn2.hpp>
#include <trng/uniform_dist.hpp>
#include <trng/discrete_dist.hpp>
#include <trng/uniform_int_dist.hpp>




#ifdef X_DISPLAY
#include "Display.h"    // for init_display()
#endif

int         init (struct global_t *global, struct const_t *constant,
                struct stats_t *stats, struct display_t *dpy, int *c, char ***v);
void        parse_args (struct global_t *global, struct const_t *constant,
                int argc, char ** argv);
void        init_check(struct global_t *global);
void        allocate_array(struct global_t *global,
                struct const_t *constant, struct display_t *dpy);
void        init_array(struct global_t *global, struct const_t *constant);

/*
    init()
        Initialize runtime environment.
*/
int init (struct global_t *global, struct const_t *constant,
    struct stats_t *stats, struct display_t *dpy, int *c, char ***v)
{
    // command line arguments
    int argc                        = *c;
    char ** argv                    = *v;

    // initialize constant values using DEFAULT values
    constant->environment_width     = DEFAULT_ENVIRO_SIZE;
    constant->environment_height    = DEFAULT_ENVIRO_SIZE;
    constant->infection_radius      = DEFAULT_RADIUS;
    constant->duration_of_disease   = DEFAULT_DURATION;
    constant->contagiousness_factor = DEFAULT_CONT_FACTOR;
    constant->deadliness_factor     = DEFAULT_DEAD_FACTOR;
    constant->total_number_of_days  = DEFAULT_DAYS;
    constant->microseconds_per_day  = DEFAULT_MICROSECS;

    // initialize global people counters using DEFAULT values
    global->number_of_people        = DEFAULT_SIZE;
    global->num_initially_infected  = DEFAULT_INIT_INFECTED;

    // initialize stats data in stats struct
    stats->num_infections = 0.0;
    stats->num_infection_attempts = 0.0;
    stats->num_deaths = 0.0;
    stats->num_recovery_attempts = 0.0;

    // initialize states counters in global struct
    global->num_infected = 0;
    global->num_susceptible = 0;
    global->num_immune = 0;
    global->num_dead = 0;

    // assign different colors for different states
    #ifdef X_DISPLAY
    dpy->red = "#FF0000";
    dpy->green = "#00FF00";
    dpy->black = "#000000";
    dpy->white = "#FFFFFF";
    #endif

    init_check(global);

    parse_args(global, constant, argc, argv);

    allocate_array(global, constant, dpy);

    // Seeds the random number generator based on the current time
    


    init_array(global, constant);

    // if use X_DISPLAY, do init_display()
    #ifdef X_DISPLAY
        init_display(constant, dpy);
    #endif

    return(0);
}

/*
    parse_args()
        Each process is given the parameters of the simulation
*/
void parse_args(struct global_t *global, struct const_t *constant, int argc, char ** argv)
{
    int c = 0;

    // Get command line options -- this follows the idiom presented in the
    // getopt man page (enter 'man 3 getopt' on the shell for more)
    while((c = getopt(argc, argv, "n:i:w:h:t:T:c:d:D:m:p:")) != -1)
    {
        switch(c)
        {
            case 'n':
            global->number_of_people = atoi(optarg);
            break;
            case 'i':
            global->num_initially_infected = atoi(optarg);
            break;
            case 'w':
            constant->environment_width = atoi(optarg);
            break;
            case 'h':
            constant->environment_height = atoi(optarg);
            break;
            case 't':
            constant->total_number_of_days = atoi(optarg);
            break;
            case 'T':
            constant->duration_of_disease = atoi(optarg);
            break;
            case 'c':
            constant->contagiousness_factor = atoi(optarg);
            break;
            case 'd':
            constant->infection_radius = atoi(optarg);
            break;
            case 'D':
            constant->deadliness_factor = atoi(optarg);
            break;
            case 'm':
            constant->microseconds_per_day = atoi(optarg);
            break;
            case 'p':
            omp_set_num_threads(atoi(optarg));
            break;
            // If the user entered "-?" or an unrecognized option, we need
            // to print a usage message before exiting.
            case '?':
            default:
            fprintf(stderr, "Usage: ");
            fprintf(stderr, "%s [-n number_of_people][-i num_initially_infected][-w environment_width]\n[-h environment_height][-t total_number_of_days][-T duration_of_disease]\n[-c contagiousness_factor][-d infection_radius][-D deadliness_factor]\n[-m microseconds_per_day] [-p number of threads]\n", argv[0]);
            exit(-1);
        }
    }
    argc -= optind;
    argv += optind;
}

/*
    init_check()
        Each process makes sure that the total number of initially
        infected people is less than the total number of people
*/
void init_check(struct global_t *global)
{
    int num_initially_infected = global->num_initially_infected;
    int number_of_people = global->number_of_people;

    if(num_initially_infected > number_of_people)
    {
        fprintf(stderr, "ERROR: initial number of infected (%d) must be less than total number of people (%d)\n",
            num_initially_infected, number_of_people);
        exit(-1);
    }
}

/*
    allocate_array()
        Allocate the arrays
*/
void allocate_array(struct global_t *global, struct const_t *constant,
    struct display_t *dpy)
{
    int number_of_people = global->number_of_people;

    // Allocate the arrays in global struct
    global->x_locations = (int*)malloc(number_of_people * sizeof(int));
    global->y_locations = (int*)malloc(number_of_people * sizeof(int));
    global->infected_x_locations = (int*)malloc(number_of_people * sizeof(int));
    global->infected_y_locations = (int*)malloc(number_of_people * sizeof(int));
    global->states = (char*)malloc(number_of_people * sizeof(char));
    global->num_days_infected = (int*)malloc(number_of_people * sizeof(int));

    // Allocate the arrays for text display
    #ifdef TEXT_DISPLAY
    dpy->environment = (char**)malloc(constant->environment_width *
        constant->environment_height * sizeof(char*));
    int current_location_x;
    for(current_location_x = 0;
        current_location_x <= constant->environment_width - 1;
            current_location_x++)
    {
        dpy->environment[current_location_x] = (char*)malloc(
            constant->environment_height * sizeof(char));
    }
    #endif
}

/*
    init_array()
        initialize arrays allocated with data in global
        struct
*/
void init_array(struct global_t *global, struct const_t *constant)
{
    // counter to keep track of current person
    int current_person_id;

    int number_of_people = global->number_of_people;
    int num_initially_infected = global->num_initially_infected;

    // OMP does not support reduction to struct, create local instance
    // and then put local instance back to struct
    int num_infected_local = global->num_infected;
    int num_susceptible_local = global->num_susceptible;

    // Process spawns threads to set the states of the initially
    // infected people and set the count of its infected people
    #ifdef _OPENMP
    #pragma omp parallel for private(current_person_id) \
        reduction(+:num_infected_local)
    #endif
    for(current_person_id = 0; current_person_id
        <= num_initially_infected - 1; current_person_id++)
    {
        global->states[current_person_id] = INFECTED;
        num_infected_local++;
    }
    global->num_infected = num_infected_local;

    // Process spawns threads to set the states of the rest of
    // its people and set the count of its susceptible people
    #ifdef _OPENMP
    #pragma omp parallel for private(current_person_id) \
        reduction(+:num_susceptible_local)
    #endif
    for(current_person_id = num_initially_infected;
        current_person_id <= number_of_people - 1;
        current_person_id++)
    {
        global->states[current_person_id] = SUSCEPTIBLE;
        num_susceptible_local++;
    }
    global->num_susceptible = num_susceptible_local;

    // Process spawns threads to set random x and y locations for
    // each of its people



    int num_threads;
    int rank;
    int x, y;

    #ifdef _OPENMP
    #pragma omp parallel private(current_person_id, num_threads, rank,x ,y)
    #endif
    {
    trng::yarn2 streamx, streamy;


    num_threads = omp_get_num_threads();
    rank = omp_get_thread_num();

    streamx.split(2,0);
    streamy.split(2,1);

    streamx.split(num_threads, rank);
    streamy.split(num_threads, rank);


    trng::uniform_int_dist distx(0, constant ->environment_width);
    trng::uniform_int_dist disty(0, constant ->environment_height);



    #ifdef _OPENMP
    #pragma omp for
    #endif
    for(current_person_id = 0;
        current_person_id <= number_of_people - 1;
        current_person_id++)
    {

        // global->x_locations[current_person_id] = random() % constant->environment_width;
        // global->y_locations[current_person_id] = random() % constant->environment_height;
        global->x_locations[current_person_id] = distx(streamx);
        global->y_locations[current_person_id] = disty(streamy);
    }
  }
    // Process spawns threads to initialize the number of days
    // infected of each of its people to 0
    #ifdef _OPENMP
    #pragma omp parallel for private(current_person_id)
    #endif
    for(current_person_id = 0;
        current_person_id <= number_of_people - 1;
        current_person_id++)
    {
        global->num_days_infected[current_person_id] = 0;
    }
}

#endif