/**
 * \brief Dining room data structures
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#ifndef DINING_ROOM_H
#define DINING_ROOM_H

#include "simulation.h"

typedef struct _DiningRoom_ {
   int pizza;                  // number of pizza meals available in dining room [0;NUM_PIZZA]
   int spaghetti;              // number of spaghetti meals available in dining room [0;NUM_SPAGHETTI]
   int cleanForks;             // number of clean forks available in dining room [0;NUM_FORKS]
   int cleanKnives;            // number of clean knives available in dining room [0;NUM_KNIVES]
   int dirtyForks;             // number of dirty forks in dining room [0;NUM_FORKS]
   int dirtyKnives;            // number of dirty knives in dining room [0;NUM_KNIVES]
   int dirtyForksInWaiter;     // number of dirty forks in waiter (i.e. the dirty forks that are being washed)
   int dirtyKnivesInWaiter;    // number of dirty knives in waiter (i.e. the dirty knives that are being washed)
   int n_philosopher_dead;     //number of dead philosophers
} DiningRoom;

int getCleanFork(Simulation *,int,int);
int getCleanKnife(Simulation *,int,int);
int giveSpaghetti(Simulation*,int);
int givePizza(Simulation*,int);
int depositDirtyFork(Simulation*,int,int);
int depositDirtyKnife(Simulation*,int,int);
int death_philosopher(Simulation*,int);
int replenishPizza(Simulation*);
int replenishSpaghetti(Simulation*);
int cleanForks(Simulation*);
int cleanKnives(Simulation*);
void verifyKnives(Simulation*);
void verifyForks(Simulation*);

#endif
