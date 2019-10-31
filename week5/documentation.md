### Asynchronously function calls disambiguation

Calls to `fn` with `args` will return without waiting for the execution of fn to complete.

### Thread pool disambiguation

A thread pool is a group of pre-instantiated, idle threads which stand ready to be given work. These are preferred over instantiating new threads for each task when there is a large number of short tasks to be done rather than a small number of long ones. This prevents having to incur the overhead of creating a thread a large number of times.

Implementation will vary by environment, but in simplified terms, you need the following:

A way to create threads and hold them in an idle state. This can be accomplished by having each thread wait at a barrier until the pool hands it work. (This could be done with mutexes as well.)
A container to store the created threads, such as a queue or any other structure that has a way to add a thread to the pool and pull one out.
A standard interface or abstract class for the threads to use in doing work. This might be an abstract class called Task with an execute() method that does the work and then returns.
When the thread pool is created, it will either instantiate a certain number of threads to make available or create new ones as needed depending on the needs of the implementation.

When the pool is handed a Task, it takes a thread from the container (or waits for one to become available if the container is empty), hands it a Task, and meets the barrier. This causes the idle thread to resume execution, invoking the execute() method of the Task it was given. Once execution is complete, the thread hands itself back to the pool to be put into the container for re-use and then meets its barrier, putting itself to sleep until the cycle repeats.

> Notes:
> - vector of threads
> - vector of futures (run all of them and wait, .get() is not required because I am editing a memory location, so there is no need to .get() the returned value, just .wait() them to finish)

> To do:
> Compare the performance of the various mechanisms.
> 
> Write a documentation containing:
> 
> for each synchronization primitive used, the rules involving it (for a mutex, what invariants it protects; for a producer-> consumer synchronization device, when is it triggered);
> what tests have you done, on what hardware platform, for what size of the data, and what was the time consumed.
