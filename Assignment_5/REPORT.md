Assignment 5
======================================

### Environment

- MacOS 11.6
- CMake 3.21.2
- GNU Make 3.81
- Apple clang 13.0.0 (clang-1300.0.29.3)

### Implementation

#### Ex1:

```
cd src; python rename.py "Ex1"; cd ..;
mkdir build; cd build; cmake ..; make
./assignment5
```

#### Ex2&3:

```
cd src; python rename.py "Ex2&3"; cd ..;
mkdir build; cd build; cmake ..; make
./assignment5
```

#### Ex4:

```
cd src; python rename.py "Ex4"; cd ..;
mkdir build; cd build; cmake ..; make
./assignment5
```

### Parameters:

For the entire program, we have some global parameters as follows:

```
Camera position: (0, 0, 5)
Gaze direction: (0, 0, -1)
View up: (0, 1, 0)

(l, b, n) = (-1, -1, -3)
(r, t, f) = (1, 1, -6)

light position: (0, 0, 5)
light intensity: (20, 20, 20)
Diffuse color: (0.5, 0.5, 0.5)
Specular color: (0.2, 0.2, 0.2)
Specular exponent: 256
```

Ex.1: Load and Render a 3D model
-----------------
We just load data from off file and render in red. After loading those vertices, we enlarge the object in seven times
and move it to the origin of coordinates.

![](result/bunny_ex1.png?raw=true)

Ex.2: Shading
----------------------
Change the value of string `uniform.shading_option` to change the version of the model. To make the results more clear,
we change the resolution to 1000 * 1000.

### Wireframe

The inputs are vertices in each line.

![](result/bunny_ex2-1.png?raw=true)

### Flat

The inputs are all triangles and all lines of the object. For each mesh, we compute the normal of it and store in vertex
attributes of each vertex, and then rasterize as triangle. For each line, we rasterize as line.

![](result/bunny_ex2-2.png?raw=true)

### Per-Vertex

The inputs are triangles of the object. For each vertex, the normal is the average of the normals of the faces touching
it. So we accumulate normals for each vertex and also store the number of normals. Then we divide by the number for the
accumulated normal each vertex and normalize it and add it into vector.

![](result/bunny_ex2-3.png?raw=true)

#### Analysis:

In the vertex shader, we first compute every thing into camera space. We do ray tracing to get the color for each vertex
in camera space. Then we compute the position of vertices into canonical view column.

In the fragment shader, we pass the color of each vertex and compute the depth of the fragment. For the flat model, we
make the depth of each line a little smaller to make sure it will be rendered in front of each flat.

In the blending shader, we do z-buffer here.

Ex.3: Object Transformation
----------------------

We have done all viewing transformation in ex2. For ex3, we add the rotation and translation to the object and get gif
files as follows. We can see that the front side of the object is always bright because of the light. And it becomes
brighter because it becomes closer to the light. Additionally, to get the animation, we need to
set `uniform.transformation_option = true;`.

### Wireframe

![](result/bunny_ex3-1.gif?raw=true)

### Flat

![](result/bunny_ex3-2.gif?raw=true)

### Per-Vertex

![](result/bunny_ex3-3.gif?raw=true)

Ex.4: Camera
----------------------
Then we add one more matrix to compute for the perspective projection. And we also make the object adapting the aspect
ratio of different resolutions. We can see that in the perspective projection, when the object is moving toward the
camera, it becomes larger.

### Perspective for Per-Vertex(1000 * 1000)
![](result/bunny_ex4-1.gif?raw=true)

### Perspective for Per-Vertex(500 * 1000)
![](result/bunny_ex4-2.gif?raw=true)

### Perspective for Per-Vertex(1000 * 500)
![](result/bunny_ex4-3.gif?raw=true)
