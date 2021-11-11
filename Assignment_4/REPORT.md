Assignment 1
======================================

### Environment

- MacOS 11.6
- CMake 3.21.2
- GNU Make 3.81
- Apple clang 13.0.0 (clang-1300.0.29.3)

### Implementation

```
mkdir build; cd build; cmake ..; make
./assignment4 ../data/scene.json
```

Ex.1: Triangle Mesh Ray Tracing
-----------------

Without acceleration, we need to Traverse every triangle and return the closest hit and calculate the color.
Then, we can render bunny, cube and dodecahedron.

First, we keep the origin position and focal length of the camera.

### Result
#### Cube:
![](result/raytrace_1.png?raw=true)

#### Dodecahedron：
![](result/raytrace_2.png?raw=true)

#### Bunny:
![](result/raytrace_3.png?raw=true)

---
After changing the parameters of the camera.

#### Cube:
![](result/raytrace_4.png?raw=true)

#### Dodecahedron:
![](result/raytrace_5.png?raw=true)

#### Bunny:
![](result/raytrace_6.png?raw=true)

Ex.2: Triangle Mesh Ray Tracing
-----------------

I implemented AABB tree by the top-down construction. Then we can render the dragon. And it takes about 1 minute time.

### Result
The origin camera:

![](result/raytrace_7.png?raw=true)

After changing the parameters of the camera.

![](result/raytrace_8.png?raw=true)
