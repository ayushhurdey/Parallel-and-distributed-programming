```py
from threading import Thread

v = [1, 9, 10, 2, 1, 9, 9, 10]

def f(v, i, j):
    v[j] += v[j-i]

def sums(v):
    n = len(v)
    i = 1
    while i < n:
        threads = []
        for j in range(2*i-1, n, 2*i):
            threads.append(Thread(target=f, args=(v,i,j,)))
            threads[-1].start()
        for th in threads:
            th.join()
        i *= 2
    t = n//4
    while t > 0:
        threads = []
        for i in range(3*t-1, n, 2*t):
            threads.append(Thread(target=f, args=(v,t,i,)))
            threads[-1].start()
        for th in threads:
            th.join()
        t //= 2
    return v

print(sums(v))
```
