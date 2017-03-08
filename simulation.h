/**
 * \brief Simulation data structure
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#ifndef SIMULATION_H
#define SIMULATION_H

#include "pthread.h"
struct _Waiter_;
struct _Parameters_;
struct _DiningRoom_;
struct _Philosopher_;

typedef struct _Simulation_ {
   struct _Parameters_* params;
   struct _DiningRoom_* diningRoom;
   struct _Philosopher_** philosophers;
   struct _Waiter_* waiter;
} Simulation;

typedef struct {
  pthread_t t_waiter;
  pthread_t *t_philo;
} Threads;

typedef struct {
  unsigned int phi_id;                    /* id. do thread filósofo */
  unsigned int lifetime;
  Simulation* sim;                    /* número de iterações */
} THREAD_PAR;

#include "parameters.h"
#include "dining-room.h"
#include "philosopher.h"
#include "waiter.h"

/** \brief locking flag which warrants mutual exclusion inside the monitor */
extern pthread_mutex_t accessCR;

extern pthread_cond_t wantCutlery;
extern pthread_cond_t wantPizza;
extern pthread_cond_t wantSpaghetti;
extern pthread_cond_t waiterNeeded;

Simulation* initSimulation(Simulation* sim, Parameters* params);
void* mem_alloc(int size);

#endif
