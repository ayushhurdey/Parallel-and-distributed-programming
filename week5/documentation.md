Asynchronously function calls disambiguation

Calls to `fn` with `args` will return without waiting for the execution of fn to complete. The value returned by `fn` can 

- vector of threads
- vector of futures (run all of them and wait, .get() is not required because I am editing a memory location, so there is no need to .get() the returned value, just .wait() them to finish)


