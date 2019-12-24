import math
from mpi4py import MPI
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
p = comm.Get_size()


def polynomial_product(first_term, second_term):
    n = len(second_term)

    if rank == 0:
        print('0')
        result = []
        for operation in range(1, n+1):
            partial_result = comm.recv(source=operation)
            i = 0
            while i < len(result):
                result[i] += partial_result[i]
                i += 1
            while i < len(partial_result):
                result.append(partial_result[i])
                i += 1
            print(partial_result)
        return result
    else:
        print('!0')
        second_term_digit = second_term[rank-1]
        result = []
        for digit in first_term:
            result.append(digit * second_term_digit)
        for padding in range(rank-1):
            result.insert(0, 0)
        comm.send(result, dest=0)

print(polynomial_product([3, 2, 1], [1, 9]))
