
Seminar 1:
  - Consider the problem of transferring money from one account to another (lab 1 pb 2)
    - examples of what could happen without any synchronization;
    - simple implementation using mutexes;
    - adding an audit() function;
    - avoiding deadlocks through ordering the mutexes;
    - using atomic integers;
    
Seminar 2:
  - Insertions in a doubly-linked list:
    - cannot do the locking always in the same order;
    - solving deadlocks by revert and retry;
    - using lock-free ideas to do locking in the same order;
  - Simple producer-consumer problem: OneShotEvent
    
Seminar 3:
  - Implement a producer-consumer queue;
  - Parallel computing for the sum of two vectors;
  - Parallel computing for the product of two matrices;
    
Seminar 4:
  - Computing the (pointwise) sum of two vectors, using async() in C++: vector-pointwise-sum.cpp
  - Computing the sum of the elements of a vector, in a binary tree structure, using async() in C++: vector-all-sum.cpp
