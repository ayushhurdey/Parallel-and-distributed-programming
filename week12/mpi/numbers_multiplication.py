import math
from mpi4py import MPI
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
p = comm.Get_size()


def compress(x):
    value = 0
    for digit in reversed(x):
        value = value*10 + digit
    return value


def extend(x):
    result = []
    while x > 0:
        result.append(x%10)
        x //= 10
    return result

# print(compress([0, 7, 0, 1, 1]))
# print(extend(compress([0, 1, 9, 1, 0, 3])))


def polynomial_product(first_term, second_term):
    n = len(second_term)
    print(n)

    if rank == 0:
        result = 0
        for operation in range(1, n+1):
            partial_result = comm.recv(source=operation)
            result += compress(partial_result)
            print(partial_result)
            print(compress(partial_result))
        return result
    else:
        second_term_digit = second_term[rank-1]
        result = second_term_digit * compress(first_term)
        result = extend(result)
        for padding in range(rank-1):
            result.insert(0, 0)
        comm.send(result, dest=0)

print(polynomial_product([3, 2, 1], [1, 9]))

# Example
# 123 *
#  91
#
# 1 * (1 2 3) + 9 * (1 2 3) = (1 2 3) + (1 1 0 7 0) = (1 1 1 9 3)
