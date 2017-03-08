/**
 * \brief Dining room
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>
#include <locale.h>
#include "parameters.h"
#include "dining-room.h"
#include "waiter.h"
#include "logger.h"

/* verify forks */
void verifyForks(Simulation* sim)
{
  if (sim->params->NUM_FORKS<(sim->diningRoom->cleanForks+sim->diningRoom->dirtyForks))
  {
    perror("Illegal number of forks");
    pthread_exit((void *) EXIT_FAILURE);
  }
}

void verifyKnives(Simulation* sim)
{
  if (sim->params->NUM_KNIVES<(sim->diningRoom->cleanKnives+sim->diningRoom->dirtyKnives))
  {
    perror("Illegal number of knives");
    pthread_exit((void *) EXIT_FAILURE);
  }
}

/* get clean fork to index position */
int getCleanFork(Simulation *sim, int phi_id, int index)
{
  verifyForks(sim);

  if (sim->params->NUM_FORKS-sim->diningRoom->dirtyForks==0||sim->diningRoom->cleanForks==0)
    return 0;

  /* check philosopher state */
  if(sim->philosophers[phi_id]->state != P_HUNGRY)
  {
    perror("Wrong state.");
    pthread_exit((void *) EXIT_FAILURE);
  }

  /* check philosopher cutlery */
  if(sim->philosophers[phi_id]->cutlery[index] != P_GET_FORK)
  {
    perror("Something gone wrong with cutlery.");
    pthread_exit((void *)EXIT_FAILURE);
  }

  /* we pick up the clean fork */
  sim->diningRoom->cleanForks--;
  sim->philosophers[phi_id]->cutlery[index] = P_FORK;

  /* show updates */
  logger(sim);

  return 1;
}

/* get clean knife to index position */
int getCleanKnife(Simulation *sim, int phi_id, int index)
{
  verifyKnives(sim);

  if (sim->params->NUM_KNIVES-sim->diningRoom->dirtyKnives==0||sim->diningRoom->cleanKnives==0)
    return 0;

  /* check philosopher State */
  if(sim->philosophers[phi_id]->state!=P_HUNGRY)
  {
    perror("Wrong state");
    pthread_exit((void *) EXIT_FAILURE);
  }

  /* check philosopher meal */
  if(sim->philosophers[phi_id]->meal != P_GET_PIZZA)
  {
    perror("Wrong meal");
    pthread_exit((void *) EXIT_FAILURE);
  }

  /* verifiying that the philosopher doesn't have another knife */
  if(sim->philosophers[phi_id]->cutlery[1-index]==P_KNIFE)
  {
    perror("Philosopher already has a knife!");
    pthread_exit((void *) EXIT_FAILURE);
  }

  /* verifying the philosopher cutlery */
  if(sim->philosophers[phi_id]->cutlery[index]!=P_GET_KNIFE)
  {
    perror("Wrong cutlery");
    pthread_exit((void *) EXIT_FAILURE);
  }

  /* we have a clean knive, let's update values */
  sim->diningRoom->cleanKnives--;
  sim->philosophers[phi_id]->cutlery[index] = P_KNIFE;

  /* show updates */
  logger(sim);

  return 1;
}

/* give spaghetti */
int giveSpaghetti(Simulation *sim, int phi_id)
{
  /* verifying number of spaghetti plates */
  if (sim->diningRoom->spaghetti < 0)
  {
    perror("There's no spaghetti");
    pthread_exit((void *) EXIT_FAILURE);
  }

  /* verifiying philosopher state */
  if (sim->philosophers[phi_id]->state!=P_HUNGRY)
  {
    perror("Wrong state");
    pthread_exit((void *) EXIT_FAILURE);
  }

  /* verifiying philosopher plate */
  if (sim->philosophers[phi_id]->meal!=P_GET_SPAGHETTI)
  {
    perror("Wrong meal");
    pthread_exit((void *) EXIT_FAILURE);
  }

  /* There's no spaghetti */
  if (!sim->diningRoom->spaghetti)
    return 0;

  /* we take up one spaghetti plate */
  sim->diningRoom->spaghetti--;

  /* show updates */
  logger(sim);

  return 1;
}

/* give pizza */
int givePizza(Simulation *sim,int phi_id)
{
  /* verifying number of pizza plates */
  if (sim->diningRoom->pizza < 0)
  {
    perror("There's no pizza");
    pthread_exit((void *) EXIT_FAILURE);
  }

  /* verifiying philosopher state */
  if (sim->philosophers[phi_id]->state != P_HUNGRY)
  {
    perror("Wrong state");
    pthread_exit((void *) EXIT_FAILURE);
  }

  if (sim->philosophers[phi_id]->meal != P_GET_PIZZA)
  {
    perror("Wrong plate");
    pthread_exit((void *) EXIT_FAILURE);
  }

  if (!sim->diningRoom->pizza)
    return 0;

  /* we take up one pizza plate */
  sim->diningRoom->pizza--;

  /* show updates */
  logger(sim);

  return 1;
}

int depositDirtyFork(Simulation *sim, int phi_id, int index)
{
  verifyForks(sim);
  if (sim->params->NUM_FORKS-sim->diningRoom->dirtyForks==0) {
    //fprintf(stderr, "All forks are clean \n");
    return 0;
  }
//verifiying philosopher state

  if(sim->philosophers[phi_id]->state!=P_FULL){
    //fprintf(stderr, "P_FULL ERROR\n");
    //fprintf(stderr, "Illegal operation. Impossible State %d while fork deposit\n", sim->philosophers[phi_id]->cutlery[index]);
    //fprintf(stderr, "Philosopher Id %d\n", phi_id);
    exit(EXIT_FAILURE);
  }

//verifying the philosopher cutlery
  if(sim->philosophers[phi_id]->cutlery[index]!=P_PUT_FORK){
    //fprintf(stderr, "Illegal operation.Impossible State %d  while fork deposit \n", sim->philosophers[phi_id]->cutlery[index]);
    //fprintf(stderr, "Philosopher Id %d\n", phi_id);
    exit(EXIT_FAILURE);
  }

  sim->philosophers[phi_id]->cutlery[index]=P_NOTHING;
  sim->diningRoom->dirtyForks+=1;
  //printf("Philosopher %d deposit dirty Fork\n",phi_id );
  logger(sim);
  return 1;
}

int depositDirtyKnife(Simulation *sim,int phi_id,int index){
    verifyKnives(sim);
    if (sim->params->NUM_KNIVES-sim->diningRoom->dirtyKnives==0) {
      //fprintf(stderr, "All knives are dirty \n");
      return 0;
    }
  //verifiying philosopher state

    if(sim->philosophers[phi_id]->state!=P_FULL){
      //fprintf(stderr, "P_FULL ERROR\n");
      //fprintf(stderr, "Illegal operation. Impossible State %d while Knife deposit\n", sim->philosophers[phi_id]->cutlery[index]);
      //fprintf(stderr, "Philosopher Id %d\n", phi_id);
      exit(EXIT_FAILURE);
    }

  //verifying the philosopher cutlery
    if(sim->philosophers[phi_id]->cutlery[index]!=P_PUT_KNIFE){
      //fprintf(stderr, "Illegal operation.Impossible State %d  while Knife deposit \n", sim->philosophers[phi_id]->cutlery[index]);
      //fprintf(stderr, "Philosopher Id %d\n", phi_id);
      exit(EXIT_FAILURE);
    }

    sim->philosophers[phi_id]->cutlery[index]=P_NOTHING;
    sim->diningRoom->dirtyKnives+=1;
    //printf("Philosopher %d deposit dirty Knife\n",phi_id );
    logger(sim);
    return 1;
}

int check_death_philosopher(Simulation *sim,int phi_id){
  //verifying philosopher state
  if (sim->philosophers[phi_id]->state!=P_DEAD) {
    //fprintf(stderr, "Philosopher %d is alive\n", phi_id);
    return 0;
  }
  sim->diningRoom->n_philosopher_dead--;
  return 1;
}


int replenishSpaghetti(Simulation *sim){
  if(sim->diningRoom->spaghetti!=0){
    //printf("There is spaghetti in diningRoom \n");
    return 0;
  }

  //printf("No spaghetti in diningRoom \n");
  sim->diningRoom->spaghetti = sim->params->NUM_SPAGHETTI;
  //pthread_cond_broadcast(&wantSpaghetti);
  logger(sim);
  return 1;
}


int replenishPizza(Simulation *sim){
  if(sim->diningRoom->pizza!=0){
    //printf("There is pizza in diningRoom \n");
    return 0;
  }

  //printf("No pizza in diningRoom \n");
  sim->diningRoom->pizza = sim->params->NUM_PIZZA;
  //pthread_cond_broadcast(&wantPizza);
  logger(sim);
  return 1;
}

int cleanForks(Simulation* sim){
  verifyForks(sim);
  //clean all forks
  if (sim->diningRoom->dirtyForks>1) {
    sim->diningRoom->dirtyForksInWaiter += sim->diningRoom->dirtyForks;
    sim->diningRoom->dirtyForks=0;
    logger(sim);

    sim->diningRoom->cleanForks += sim->diningRoom->dirtyForksInWaiter;
    sim->diningRoom->dirtyForksInWaiter = 0;
    logger(sim);
    //thread_cond_broadcast(&wantCutlery);
    return 1;
  }
  return 0;
}


int cleanKnives(Simulation* sim){
  verifyKnives(sim);
  //clean all forks
  if (sim->diningRoom->dirtyKnives>1) {
    sim->diningRoom->dirtyKnivesInWaiter += sim->diningRoom->dirtyKnives;
    sim->diningRoom->dirtyKnives=0;
    logger(sim);

    sim->diningRoom->cleanKnives += sim->diningRoom->dirtyKnivesInWaiter;
    sim->diningRoom->dirtyKnivesInWaiter = 0;
    logger(sim);
    //pthread_cond_broadcast(&wantCutlery);
    return 1;
  }
  return 0;
}
