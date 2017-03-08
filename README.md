# PhilosophersDinner
Operative Systems Delivery - Philosophers Dinner Problem



###Philosopher

The philosopher (very interesting) lifetime consists of repeated cycles of thinking, choosing a meal, and eating.
A meal is chosen randomly (as defined ahead in the parameter table). To eat, the philosopher must fetch the meal and the necessary cutlery from the clean pot. If one of these operations is not possible due to the unavailability of the meal or the clean cutlery, then the philosopher should request the waiter the missing item and should wait for it. At the end of the meal the philosopher returns the dirty cutlery (no waiter notification should occur), and proceeds with its profound, and surely very important, reflection. Its lifetime should be defined randomly using a minimum and a maximum number of iterations.

###Waiter

The (unique) waiter lifetime consists of waiting and fulfilling requests from philosophers.The list of possible requests are: clean cutlery (requires washing dirty cutlery, and possibly waiting for it), replenish pizza meals, and replenish spaghetti meals. The waiter life should terminate when all philosophers are dead (the kitchen should be left clean).



###DiningRoom
The dinning room is the most important part in this "world" , it holds all the available cutlery and meal states and "manages" all the Philosophers and Waiters activity.



##Assignment
Devise a possible solution and write a simulation for this problem, one using POSIX threads, mutexes and condition variables.
The solution should be implemented in C/C++ and be run in Linux.
A logging file, which describes the dynamics of the simulation in a clear and precise way, must be produced.
