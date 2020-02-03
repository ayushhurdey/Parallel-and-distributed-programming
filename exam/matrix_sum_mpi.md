```python
'''
Write a parallel program that computes the sum of all elements in a matrix using MPI.
It shall use a binary tree for computing the sum
'''
from mpi4py import MPI
from copy import deepcopy
comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

matrix = [1, 2, 3, 4, 5, 6, 7, 8, 9]

if rank == 0:
    print('rank 0, size='+str(size))
    node_no = 0
    res = 0
    if size-1 >= 2:
        mts1 = deepcopy(matrix[:int(len(matrix)/2)])
        mts2 = deepcopy(matrix[int(len(matrix)/2):])
        comm.send({'mts': mts1, 'node_no': node_no + 1, 'parent': node_no}, dest=node_no + 1)
        comm.send({'mts': mts2, 'node_no': node_no + 2, 'parent': node_no}, dest=node_no + 2)
        response1 = comm.recv(source=node_no + 1)
        response2 = comm.recv(source=node_no + 2)
        res += int(response1) + int(response2)
    elif size-1 == 1:
        mts1 = deepcopy(matrix[:int(len(matrix) / 2)])
        mts2 = deepcopy(matrix[int(len(matrix) / 2):])
        comm.send({'mts': mts1, 'node_no': node_no + 1, 'parent': node_no}, dest=node_no + 1)
        response1 = comm.recv(source=node_no + 1)
        res += int(response1)
        for i in mts2:
            res += i
    else:
        for i in matrix:
            res += i
    print(res)
else:
    message = comm.recv(source=MPI.ANY_SOURCE)
    node_no = int(message['node_no'])
    mts = message['mts']
    parent = int(message['parent'])
    if node_no*2 < size-2:
        mts1 = deepcopy(mts[:int(len(mts)/2)])
        mts2 = deepcopy(mts[int(len(mts)/2):])
        comm.send({'mts': mts1, 'node_no': node_no*2 + 1, 'parent': node_no}, dest=node_no*2 + 1)
        comm.send({'mts': mts2, 'node_no': node_no*2 + 2, 'parent': node_no}, dest=node_no*2 + 2)
        response1 = comm.recv(source=node_no*2 + 1)
        response2 = comm.recv(source=node_no*2 + 2)
        res = int(response1)+int(response2)
        comm.send(res, dest=parent)
    elif node_no*2 == size-2:
        mts1 = deepcopy(mts[:int(len(mts) / 2)])
        mts2 = deepcopy(mts[int(len(mts) / 2):])
        comm.send({'mts': mts1, 'node_no': node_no * 2 + 1, 'parent': node_no}, dest=node_no * 2 + 1)
        response1 = comm.recv(source=node_no * 2 + 1)
        res = int(response1)
        for i in mts2:
            res += i
        comm.send(res, dest=parent)
    else:
        res = 0
        for i in mts:
            res += i
        comm.send(res, dest=parent)
```

