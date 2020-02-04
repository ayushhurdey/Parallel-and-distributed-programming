```python
'''
MPI Primes up to N. Primes up to sqrt(N) are given
'''

from mpi4py import MPI
from math import sqrt, floor
comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

prime_numbers = [2, 3, 5, 7]


def primes(begin, end):
    for i in range(begin, end):
        is_prime = True
        for n in prime_numbers:
            if i % n == 0:
                is_prime = False
                break
        if is_prime:
            print(i)


if rank == 0:
    for pnr in prime_numbers:
        print(pnr)
    n = 100
    k = size - 1
    sq = floor(sqrt(n))
    c = (n - sq) // k
    r = (n - sq) % k
    intervals = [[sq, sq + c]]
    for _ in range(k - 1):
        intervals.append([intervals[-1][1], intervals[-1][1] + c])
    intervals[-1][1] += r
    pn = 1
    for interval in intervals:
        comm.send({'begin': interval[0], 'end': interval[1]}, dest=pn)
        pn += 1
else:
    obj = comm.recv(source=0)
    begin = obj['begin']
    end = obj['end']
    primes(begin, end)
```

