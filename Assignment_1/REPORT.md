Assignment 1
======================================

### Environment

- MacOS 11.6
- CMake 3.21.2
- GNU Make 3.81
- Apple clang 13.0.0 (clang-1300.0.29.3)

Ex.1: Convex Hull
-----------------

### Implementation

```
mkdir build; cd build; cmake ..; make
./convex_hull ../data/points.xyz output.obj
```

### Algorithm

1. Check x-coordinate and y-coordinate of each point and find the left-most one `p0`.

2. Sort those points in increasing order of the angle they and `p0` make with the x-axis. Determine the order of each points `p1` and `p2` by calculating `det(p1 - p0, p2 - p0)`.

3. Consider each points in the sorted list. If one turns left, push into the stack `hull`. If one turns right, pop it and continue checking the top of the stack until one turns left.

4. After considering all points. We got the result in `hull`.

### Result

![](result/hull.png?raw=true)

The result screenshort contains the output file `output.obj` and the original data `points.xyz`.

Ex.2: Point In Polygon
----------------------

### Implementation

```
mkdir build; cd build; cmake ..; make
./point_in_polygon ../data/points.xyz ../data/polygon.obj result.xyz
```

### Algorithm

1. Consider each point on the polygon, find the maximum value of x-coordinate and y-coordinate as `maxX` and `maxY`. So that the outside point `outside` could be `(maxX+1,maxY+1)`.
   
2. For each point, check if it is inside the polygon. We count the number of intersection. If we have `P1=(x1,y1),P2=(x2,y2),P3=(x3,y3),P4=(x4,y4)` and two vectors `P1P2` and `P3P4`. If they intersect, then `P1` and `P2` must on the different sides of `P3P4` and vice versa. So we can calculate `det(P1P2, P1P3) * det(P1P2, P1P4)` and `det(P3P4, P3P1) * det(P3P4, P3P2)`. If these two value are both negative, we can say these two vectors intersect.

3. If the number of intersection is odd, the point is inside the polygon. If the number is even, it is outside the polygon.
   
### Result

![](result/inside.png?raw=true)

The result screenshot contains the output file `result.xyz` and the original data files `points.xyz` and `polygon.obj`. The points in `result.xyz` is larger than the original points.