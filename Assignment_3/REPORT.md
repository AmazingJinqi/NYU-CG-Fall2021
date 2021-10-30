Assignment 3
======================================

### Environment

- MacOS 11.6
- CMake 3.21.2
- GNU Make 3.81
- Apple clang 13.0.0 (clang-1300.0.29.3)

### Implementation

```
mkdir build; cd build; cmake ..; make
./assignment3 ../data/scene.json
```

Ex.1: Field of View and Perspective Camera
-----------------

### Result

![](result/raytrace_1.png?raw=true)


Ex.2: Shadow Rays
----------------------

### Result

![](result/raytrace_2.png?raw=true)

There is no different after adding shadow because all lights are visible right now.
We try to add one more sphere at (7,7,0) to make some difference.

### Result

![](result/raytrace_3.png?raw=true)

Now it becomes dark.

Then we try to remove the epsilon.

### Result

![](result/raytrace_7.png?raw=true)

Ex.3: Reflection & Refraction
----------------------

### Result

![](result/raytrace_6.png?raw=true)

Ex.4: Depth of Field
----------------------

### Result

It becomes blur.

![](result/raytrace_4.png?raw=true)

Now move the rightmost sphere more into focus.

### Result

Now it becomes clear.

![](result/raytrace_5.png?raw=true)

Ex.5: Animation
----------------------

We move all sphere far from the camera.

### Result

![](result/out.gif?raw=true)
