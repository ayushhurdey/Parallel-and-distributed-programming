
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
