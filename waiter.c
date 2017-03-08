/**
 *  \brief Waiter module
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "dining-room.h"
#include "logger.h"
#include "simulation.h"

void waiterBirth(Simulation*);
int check_Death_Philosophers(Simulation*);
int check_request(Simulation*);
void finalclean(Simulation*);
void waiter_dead(Simulation*);

void * waiterLife(void * val_p){
  Simulation * sim = ((Simulation *) val_p);

  waiterBirth(sim);
  Waiter* wait = sim->waiter;

  wait->state = W_SLEEP;
  logger(sim);

  while(!check_Death_Philosophers(sim)) {
    if(pthread_mutex_lock(&accessCR) != 0)
    {
      perror("Error on entering monitor (PDead)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    if(!pthread_cond_wait(&waiterNeeded, &accessCR))
    {
      if(pthread_mutex_unlock(&accessCR) != 0)
      {
        perror("Error on entering monitor (PDead)");
        pthread_exit((void *) EXIT_FAILURE);
      }
      if(pthread_mutex_lock(&accessCR) != 0)
      {
        perror("Error on entering monitor (PDead)");
        pthread_exit((void *) EXIT_FAILURE);
      }
      switch (check_request(sim)) {
        case 1:
        {
          /* Cutlery Request */
          while(!cleanForks(sim) && !cleanKnives(sim))
          {
            perror("Invalid request. Clean cutlery is available.");
            exit(EXIT_FAILURE);
          }
          /* Waiting time to wash */
          usleep(sim->params->WASH_TIME * 1000);
          wait->reqCutlery = W_INACTIVE;
          logger(sim);
          pthread_cond_broadcast(&wantCutlery);
          break;
        }
        case 2:
        {
          /* Pizza Request */
          while(!replenishPizza(sim))
          {
            perror("Invalid request. There is pizza available.");
            exit(EXIT_FAILURE);
          }
          /* Wait time to cook */
          usleep(sim->params->EAT_TIME * 1000);
          wait->reqPizza = W_INACTIVE;
          logger(sim);
          pthread_cond_broadcast(&wantPizza);
          break;
        }
        case 3:
        {
          /* Spaghetti Request */
          while(!replenishSpaghetti(sim))
          {
            perror("Invalid request. There is spaghetti available.");
            exit(EXIT_FAILURE);
          }
          /* Wait time to cook */
          usleep(sim->params->EAT_TIME * 1000);
          wait->reqSpaghetti = W_INACTIVE;
          logger(sim);
          pthread_cond_broadcast(&wantSpaghetti);
          break;
        }
        default:
        {
          break;
        }
      }
      wait->state = W_SLEEP;
      logger(sim);
      if(pthread_mutex_unlock(&accessCR) != 0)
      {
        perror("Error on entering monitor (PDead)");
        pthread_exit((void *) EXIT_FAILURE);
      }
    }
    else
    {
      if(pthread_mutex_unlock(&accessCR) != 0)
      {
        perror("Error on entering monitor (PDead)");
        pthread_exit((void *) EXIT_FAILURE);
      }
    }
  }

  //if all philosophers are dead we need to check if there are any forks/knives to clean
  //and if the number of clean knives/forks is equal to the initial one
  finalclean(sim);
  //going to die
  waiter_dead(sim);
  logger(sim);
  return sim;
}


void waiterBirth(Simulation* sim) {
  Waiter * wait= sim->waiter;
  wait->state=W_NONE;
  wait->reqCutlery=W_INACTIVE;
  wait->reqPizza=W_INACTIVE;
  wait->reqSpaghetti=W_INACTIVE;
  logger(sim);
  //printf("Waiter Birth confirmed\n");
}


int check_Death_Philosophers(Simulation* sim){
  //needs to check all philosophers death
  if(!sim->diningRoom->n_philosopher_dead)
    return 1;
  return 0;
}


void finalclean(Simulation* sim) {
//Knives
  while(sim->diningRoom->dirtyKnives>0) {
    cleanKnives(sim);
  }
  //if all knives are clean, we need to check if the number of cleanKnives==NUM_KNIVES
  //or if the number of knives in the room is incorrrect
  //if so we need to "raise" an error

  if (sim->diningRoom->cleanKnives!=sim->params->NUM_KNIVES) {
    //fprintf(stderr, "Invalid NUM_KNIVES. We lost %d knives in the diningRoom\n",sim->params->NUM_KNIVES-sim->diningRoom->cleanKnives );
  }
//Forks
  while(sim->diningRoom->dirtyForks>0) {
    cleanForks(sim);
  }
  //if all Forks are clean, we need to check if the number of cleanForks==NUM_Forks
  //or if the number of Forks in the room is incorrrect
  //if so we need to "raise" an error

  if (sim->diningRoom->cleanForks!=sim->params->NUM_FORKS) {
    //fprintf(stderr, "Invalid NUM_Forks. We lost %d Forks in the diningRoom\n", sim->params->NUM_FORKS-sim->diningRoom->cleanForks);
  }
}

int check_request(Simulation* sim) {
  Waiter * wait = sim->waiter;

  if (wait->reqCutlery == W_ACTIVE) {
    return 1;
  }
  else if (wait->reqPizza == W_ACTIVE) {
    return 2;
  }
  else if(wait->reqSpaghetti == W_ACTIVE){
    return 3;
  }

  return -1;
}

void waiter_dead(Simulation* sim){
  sim->waiter->state=W_DEAD;
  logger(sim);
}
