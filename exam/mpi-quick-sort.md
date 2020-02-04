Run it with the closest power of two greater than the number of elements you have in the vector.
```bash
$mpiexec -n 8 python3 mpi-quick-sort.py 
```

```py
from mpi4py import MPI
comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

def partition(arr, lo, hi):
    pivot = arr[hi]
    i = lo
    for j in range(lo, hi):
        if arr[j] < pivot:
            aux = arr[i]
            arr[i] = arr[j]
            arr[j] = aux
            i+=1
    aux = arr[i]
    arr[i] = arr[hi]
    arr[hi] = aux
    return i

if rank == 0:
    arr = [9, 0, 1, 7, 3]
    comm.send({'arr': arr, 'lo': 0, 'hi': len(arr)-1}, dest=1)
    result = comm.recv(source=1)
    print(result)
else:
    obj = comm.recv(source=rank//2)
    arr = obj['arr']
    hi = obj['hi']
    lo = obj['lo']
    if lo < hi:
        p = partition(arr, lo, hi)
        comm.send({'arr': arr, 'lo': lo, 'hi': p-1}, dest=rank*2)
        comm.send({'arr': arr, 'lo': p+1, 'hi': hi}, dest=rank*2+1)
        half1 = comm.recv(source=rank*2)
        half2 = comm.recv(source=rank*2+1)
        half1 = half1[:p]
        half2 = half2[p:]
        half1 += half2
        comm.send(half1, dest=rank//2)
    else:
        comm.send(arr, dest=rank//2)
```
