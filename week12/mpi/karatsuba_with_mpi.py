import math 
from mpi4py import MPI
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
p = comm.Get_size()


def karatsuba_util(x, y):
    if x < 10 and y < 10:
        return x * y

    n = max(len(str(x)), len(str(y)))
    m = int(math.ceil(float(n) / 2))

    # divide x into two half
    xh = int(math.floor(x / 10 ** m))
    xl = int(x % (10 ** m))

    # divide y into two half
    yh = int(math.floor(y / 10 ** m))
    yl = int(y % (10 ** m))

    # Karatsuba's algorithm.
    s1 = karatsuba_util(xh, yh)
    s2 = karatsuba_util(yl, xl)
    s3 = karatsuba_util(xh + xl, yh + yl)
    s4 = s3 - s2 - s1

    return int(s1 * (10 ** (m*2)) + s4 * (10 ** m) + s2)


def karatsuba(x, y):
    if x < 10 and y < 10:
        return x * y

    n = max(len(str(x)), len(str(y)))
    m = int(math.ceil(float(n) / 2))

    # divide x into two half
    xh = int(math.floor(x / 10 ** m))
    xl = int(x % (10 ** m))

    # divide y into two half
    yh = int(math.floor(y / 10 ** m))
    yl = int(y % (10 ** m))

    # Karatsuba's algorithm.
    if rank == 1:
        print("1")
        f1 = karatsuba_util(xh, yh)
        comm.send(f1, dest=0)
    elif rank == 2:
        print("2")
        f2 = karatsuba_util(xl, yl)
        comm.send(f2, dest=0)
    elif rank == 3:
        print("3")
        f3 = karatsuba_util(xh + xl, yh + yl)
        comm.send(f3, dest=0)
    elif rank == 0:
        print("0")
        s1 = comm.recv(source=1)
        print('received: ', s1)
        s2 = comm.recv(source=2)
        print('received: ', s2)
        s3 = comm.recv(source=3)
        print('received: ', s3)
        s4 = s3 - s2 - s1
        print('computed s4 as:' , s4)
        return int(s1*(10**(m*2)) + s4*(10**m) + s2)

print('result: ', karatsuba(123, 91))
