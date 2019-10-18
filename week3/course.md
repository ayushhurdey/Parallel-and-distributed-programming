
Producer:
```
data = ... [2]
available = true [3]
cv.notify() [4]
```
  
Consumer:
```
if(!available) { [0]
[1]
cv.wait() [5] [6] ... [infinity]
}
use(data)
```

![](https://github.com/alexdarie/Parallel-and-distributed-systems/blob/master/week3/MVIMG_20191018_092732.jpg)

Replace if by while. Also replace while with `cv.wait(l, [&available](){return available;}); use(data);`
