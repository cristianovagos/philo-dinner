/**
 *  \brief Philosopher module
 *
 * \author Miguel Oliveira e Silva - 2016
 * \author Cristiano Marques Vagos (65169)
 * \author João Pedro Fonseca (73779)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "philosopher.h"
#include "dining-room.h"
#include "simulation.h"
#include "logger.h"

/* Internal functions*/
void philosopherbirth(Simulation*, int);
void think(Simulation*, int);
int hungry(Simulation*, int);
int set_table(Simulation*, int, int);
void eating(Simulation*, int);
void full(Simulation*, int);
void clean_table(Simulation*, int);
void philosopher_dead(Simulation*, int);


/*
 * Philosopher's life cycle
 */
void *life_cycle (void *val_p)
{
  unsigned int phi_id=((THREAD_PAR *) val_p)->phi_id;
  Simulation * sim = ((THREAD_PAR *) val_p)->sim;
  unsigned int lifetime = ((THREAD_PAR *) val_p)->lifetime;

  /* make philosopher alive */
  philosopherbirth(sim,phi_id);

  /* the philosopher is alive while there is food to eat
     there is also the problem of time. */
  int plate;

  while (lifetime!=0)
  {
    plate--;
    think(sim, phi_id);                   /* think state */
    plate = hungry(sim,phi_id);           /* philosopher is hungry (chooses meal) */

    /* gets cutlery, gets food */
    if(set_table(sim, phi_id,plate))
    {
      eating(sim, phi_id);                /* philosopher is eating */
      full(sim, phi_id);                  /* philosopher is full */
      clean_table(sim, phi_id);           /* philosopher "cleans" the table */
    }
    else
    {
      perror("Error while picking up food!");
      exit(EXIT_FAILURE);
    }

    /* discount lifetime, too bad it won't last forever */
    lifetime--;
  }

  /* philosopher finished his lifetime */
  philosopher_dead(sim,phi_id);
  pthread_exit((void *) EXIT_SUCCESS);

  return (val_p);
}

/*
 * make philosopher alive
 */
void philosopherbirth(Simulation * sim, int phi_id){
  /* enter critical region */
  if(pthread_mutex_lock(&accessCR) != 0)
  {
    perror("Error on entering monitor (PBirth)");
    pthread_exit((void *) EXIT_FAILURE);
  }

  /* Initializing the factory values of the philosopher */
  Philosopher* philo = sim->philosophers[phi_id];

  philo->meal =  P_NONE;
  philo->state = P_BIRTH;
  philo->cutlery[0] = P_NOTHING;
  philo->cutlery[1] = P_NOTHING;

  /* show updates */
  logger(sim);

  /* leave critical region */
  if(pthread_mutex_unlock(&accessCR) != 0)
  {
    perror("Error on exiting monitor (PBirth)");
    pthread_exit((void *) EXIT_FAILURE);
  }
}

/*
 * thinking state
 */
void think(Simulation* sim,int phi_id)
{
  /* enter critical region */
  if(pthread_mutex_lock(&accessCR) != 0)
  {
    perror("Error on entering monitor (PThink)");
    pthread_exit((void *) EXIT_FAILURE);
  }

  Philosopher* philo = sim->philosophers[phi_id];

  /* philosopher is thinking */
  philo->state = P_THINKING;

  logger(sim);

  /* we wait the time scheduled for thinking */
  usleep(sim->params->THINK_TIME * 1000);

  /* leave critical region */
  if(pthread_mutex_unlock(&accessCR) != 0)
  {
    perror("Error on exiting monitor (PThink)");
    pthread_exit((void *) EXIT_FAILURE);
  }
}

int hungry(Simulation* sim,int phi_id)
{
  /* enter critical region */
  if(pthread_mutex_lock(&accessCR) != 0)
  {
    perror("Error on entering monitor (PHungry)");
    pthread_exit((void *) EXIT_FAILURE);
  }

  Philosopher* philo = sim->philosophers[phi_id];
  int meal, randomfood = rand() % 101;

  /* philosopher is hungry */
  philo->state = P_HUNGRY;

  /* show updates */
  logger(sim);

  /* randomly selected food */
  if (randomfood < sim->params->CHOOSE_PIZZA_PROB)
  {
    /* philosopher will eat some pizza */
    meal = 1;
  }
  else
  {
    /* philosopher will eat spaghetti */
    meal = 0;
  }

  /* leave critical region */
  if(pthread_mutex_unlock(&accessCR) != 0)
  {
    perror("Error on exiting monitor (PHungry)");
    pthread_exit((void *) EXIT_FAILURE);
  }

  return meal;
}

/*
 * philosopher will pick up the food and cutlery needed to have meal
 */
int set_table(Simulation* sim, int phi_id, int pizza)
{
  /* enter critical region */
  if(pthread_mutex_lock(&accessCR) != 0)
  {
    perror("Error on entering monitor (PSet)");
    pthread_exit((void *) EXIT_FAILURE);
  }

  Philosopher* philo = sim->philosophers[phi_id];

  /* check the chosen meal */
  if (pizza)
  {
    /* if there's no pizza available in the dining room he requests it */
    while(!sim->diningRoom->pizza)
    {
      /* request active */
      sim->waiter->state = W_REQUEST_PIZZA;
      sim->waiter->reqPizza = W_ACTIVE;

      /* waiter, I need you! */
      pthread_cond_signal(&waiterNeeded);

      /* we wait for the waiter signal */
      pthread_cond_wait(&wantPizza, &accessCR);
    }

    /* we have pizza! */
    philo->meal=P_GET_PIZZA;

    /* show updates */
    logger(sim);

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* enter critical region */
    if(pthread_mutex_lock(&accessCR) != 0)
    {
      perror("Error on entering monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* we check on cutlery */
    while (sim->diningRoom->cleanForks<1)
    {
      /* request active */
      sim->waiter->state = W_REQUEST_CUTLERY;
      sim->waiter->reqCutlery = W_ACTIVE;

      /* waiter, I need you! */
      pthread_cond_signal(&waiterNeeded);

      /* we wait for cleaned cutlery */
      pthread_cond_wait(&wantCutlery, &accessCR);
    }

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* enter critical region */
    if(pthread_mutex_lock(&accessCR) != 0)
    {
      perror("Error on entering monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* we will pick up a knife */
    while (philo->cutlery[0] != P_KNIFE)
    {
      /* I want a knife */
      philo->cutlery[0] = P_GET_KNIFE;

      /* show updates */
      logger(sim);

      /* if there's no clean knife available, request it */
      if (!getCleanKnife(sim, phi_id, 0))
      {
        /* request active */
        sim->waiter->state = W_REQUEST_CUTLERY;
        sim->waiter->reqCutlery = W_ACTIVE;

        /* waiter, I need you! */
        pthread_cond_signal(&waiterNeeded);

        /* we wait for cleaned cutlery */
        pthread_cond_wait(&wantCutlery, &accessCR);
      }
    }

    /* show updates */
    logger(sim);

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* enter critical region */
    if(pthread_mutex_lock(&accessCR) != 0)
    {
      perror("Error on entering monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* we will pick up a fork */
    while (philo->cutlery[1]!=P_FORK)
    {
      /* I want a fork */
      philo->cutlery[1]=P_GET_FORK;

      /* show updates */
      logger(sim);

      /* if there's no clean fork available, request it */
      if (!getCleanFork(sim, phi_id, 1))
      {
        /* request active */
        sim->waiter->state = W_REQUEST_CUTLERY;
        sim->waiter->reqCutlery = W_ACTIVE;

        /* waiter, I need you! */
        pthread_cond_signal(&waiterNeeded);

        /* we wait for cleaned cutlery */
        pthread_cond_wait(&wantCutlery, &accessCR);
      }
    }

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    // Fake waiter
    givePizza(sim, phi_id);
  }
  else
  {
    /* picking up spaghetti */

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* enter critical region */
    if(pthread_mutex_lock(&accessCR) != 0)
    {
      perror("Error on entering monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* we check if there's spaghetti available in the dining room
     * if there's no spaghetti, request it
     */
    while(!sim->diningRoom->spaghetti)
    {
      /* request active */
      sim->waiter->state = W_REQUEST_SPAGHETTI;
      sim->waiter->reqSpaghetti = W_ACTIVE;

      /* waiter, I need you! */
      pthread_cond_signal(&waiterNeeded);

      /* we wait for spaghetti replenish */
      pthread_cond_wait(&wantSpaghetti, &accessCR);
    }

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* enter critical region */
    if(pthread_mutex_lock(&accessCR) != 0)
    {
      perror("Error on entering monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* we have spaghetti! */
    philo->meal=P_GET_SPAGHETTI;

    /* show updates */
    logger(sim);

    /* we check for clean forks
     * if not, request it
     */
    while (sim->diningRoom->cleanForks<2)
    {
      /* request active */
      sim->waiter->state = W_REQUEST_CUTLERY;
      sim->waiter->reqCutlery = W_ACTIVE;

      /* waiter, I need you! */
      pthread_cond_signal(&waiterNeeded);

      /* we wait for cleaned cutlery */
      pthread_cond_wait(&wantCutlery, &accessCR);
    }

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* enter critical region */
    if(pthread_mutex_lock(&accessCR) != 0)
    {
      perror("Error on entering monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* I need a fork to eat spaghetti */
    while(philo->cutlery[0]!=P_FORK)
    {
      /* I want a fork! */
      philo->cutlery[0]=P_GET_FORK;

      /* show updates */
      logger(sim);

      /* check if there's available forks in dining room
       * if not, request it to waiter
       */
      if (!getCleanFork(sim, phi_id, 0))
      {
        /* request active */
        sim->waiter->state = W_REQUEST_CUTLERY;
        sim->waiter->reqCutlery = W_ACTIVE;

        /* waiter, I need you! */
        pthread_cond_signal(&waiterNeeded);

        /* we wait for cleaned cutlery */
        pthread_cond_wait(&wantCutlery, &accessCR);
      }
    }

    /* show updates */
    logger(sim);

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* enter critical region */
    if(pthread_mutex_lock(&accessCR) != 0)
    {
      perror("Error on entering monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* I need one more fork to eat spaghetti */
    while (philo->cutlery[1]!=P_FORK)
    {
      /* I want a fork to my other hand */
      philo->cutlery[1]=P_GET_FORK;

      /* show updates */
      logger(sim);

      /* check if there's available forks in dining room
       * if not, request it to waiter
       */
      if (!getCleanFork(sim, phi_id, 1))
      {
        /* request active */
        sim->waiter->state = W_REQUEST_CUTLERY;
        sim->waiter->reqCutlery = W_ACTIVE;

        /* waiter, I need you! */
        pthread_cond_signal(&waiterNeeded);

        /* we wait for cleaned cutlery */
        pthread_cond_wait(&wantCutlery, &accessCR);
      }
    }

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PSet)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    //fake waiter
    giveSpaghetti(sim,phi_id);
  }

  /* show updates */
  logger(sim);

  return 1;
}

/*
 * philosopher eating state
 */
void eating(Simulation* sim,int phi_id)
{
  /* enter critical region */
  if(pthread_mutex_lock(&accessCR) != 0)
  {
    perror("Error on entering monitor (PEat)");
    pthread_exit((void *) EXIT_FAILURE);
  }

  Philosopher* philo =sim->philosophers[phi_id];

  /* eat the chosen meal */
  if (philo->meal==P_GET_SPAGHETTI)
    philo->meal = P_EAT_SPAGHETTI;
  else if(philo->meal==P_GET_PIZZA)
    philo->meal = P_EAT_PIZZA;
  else
  {
    perror("Something gone wrong, there's no meal chosen");
    exit(EXIT_FAILURE);
  }

  /* philosopher is eating */
  philo->state = P_EATING;

  /* show updates */
  logger(sim);

  /* we wait the scheduled eat time */
  usleep(sim->params->EAT_TIME * 1000);

  /* leave critical region */
  if(pthread_mutex_unlock(&accessCR) != 0)
  {
    perror("Error on exiting monitor (PEat)");
    pthread_exit((void *) EXIT_FAILURE);
  }
}

/*
 * philosopher finished meal, it's full :)
 * time to lay down the dirty cutlery and put it on their pot
 */
void full(Simulation* sim,int phi_id)
{
  /* enter critical region */
  if(pthread_mutex_lock(&accessCR) != 0)
  {
    perror("Error on entering monitor (PFull)");
    pthread_exit((void *) EXIT_FAILURE);
  }

  Philosopher * philo = sim->philosophers[phi_id];

  /* making sure that info is correct */
  if (philo->state == P_EATING) {
    /* full state */
    philo->state=P_FULL;

    /* we get to know the meal picked before and put down the cutlery */
    if (philo->meal==P_EAT_PIZZA)
    {
      philo->cutlery[0]=P_PUT_KNIFE;
      philo->cutlery[1]=P_PUT_FORK;
    }
    else
    {
      philo->cutlery[0]=P_PUT_FORK;
      philo->cutlery[1]=P_PUT_FORK;
    }
  }
  else
  {
    perror("Something gone wrong with philosopher state.");
    pthread_exit((void *) EXIT_FAILURE);
  }

  /* show updates */
  logger(sim);

  /* leave critical region */
  if(pthread_mutex_unlock(&accessCR) != 0)
  {
    perror("Error on exiting monitor (PFull)");
    pthread_exit((void *) EXIT_FAILURE);
  }
}

/*
 * clean table, remove the dirty cutlery from table
 */
void clean_table(Simulation* sim,int phi_id)
{
  /* enter critical region */
  if(pthread_mutex_lock(&accessCR) != 0)
  {
    perror("Error on entering monitor (PClean)");
    pthread_exit((void *) EXIT_FAILURE);
  }

  Philosopher * philo = sim->philosophers[phi_id];

  /* we find the meal chosen before */
  if (philo->meal==P_EAT_PIZZA)
  {
    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PClean)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* enter critical region */
    if(pthread_mutex_lock(&accessCR) != 0)
    {
      perror("Error on entering monitor (PClean)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* we put the dirty Knife into the dirty cutlery pot */
    depositDirtyKnife(sim, phi_id,0);

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PClean)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* enter critical region */
    if(pthread_mutex_lock(&accessCR) != 0)
    {
      perror("Error on entering monitor (PClean)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* we put the dirty fork into the dirty cutlery pot */
    depositDirtyFork(sim, phi_id,1);

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PClean)");
      pthread_exit((void *) EXIT_FAILURE);
    }
  }
  else if(philo->meal==P_EAT_SPAGHETTI)
  {
    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PClean)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* enter critical region */
    if(pthread_mutex_lock(&accessCR) != 0)
    {
      perror("Error on entering monitor (PClean)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* we put the dirty fork into the dirty cutlery pot */
    depositDirtyFork(sim, phi_id,0);

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PClean)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* enter critical region */
    if(pthread_mutex_lock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PClean)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    /* we put the dirty fork into the dirty cutlery pot */
    depositDirtyFork(sim, phi_id,1);

    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PClean)");
      pthread_exit((void *) EXIT_FAILURE);
    }
  }
  else
  {
    /* leave critical region */
    if(pthread_mutex_unlock(&accessCR) != 0)
    {
      perror("Error on exiting monitor (PClean)");
      pthread_exit((void *) EXIT_FAILURE);
    }

    perror("Error on philosopher meal");
    exit(EXIT_FAILURE);
  }

  /* return to "sleeping" state */
  philo->meal=P_NONE;

  /* show updates */
  logger(sim);
}

/*
 * finish philosopher life cycle
 */
void philosopher_dead(Simulation* sim,int phi_id)
{
  /* enter critical region */
  if(pthread_mutex_lock(&accessCR) != 0)
  {
    perror("Error on entering monitor (PDead)");
    pthread_exit((void *) EXIT_FAILURE);
  }

  Philosopher * philo = sim->philosophers[phi_id];

  /* Philosopher is dead :( */
  philo->state = P_DEAD;

  sim->diningRoom->n_philosopher_dead--;

  /* show updates */
  logger(sim);

  /* warn waiter about the philosopher death */
  pthread_cond_signal(&waiterNeeded);

  /* leave critical region */
  if(pthread_mutex_unlock(&accessCR) != 0)
  {
    perror("Error on exiting monitor (PDead)");
    pthread_exit((void *) EXIT_FAILURE);
  }
}
