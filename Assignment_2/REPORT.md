Assignment 2
======================================

### Environment

- MacOS 11.6
- CMake 3.21.2
- GNU Make 3.81
- Apple clang 13.0.0 (clang-1300.0.29.3)

### Implementation

```
mkdir build; cd build; cmake ..; make
./assignment2
```

Ex.1: Basic Ray Tracing
-----------------

### Ray Tracing a Parallelogram

In the `raytrace_parallelogram` function, we do orthographic projection on the parallelogram. The funciton `raygram` is
used to check if a ray intersects with an arbitrary parallelogram. Then we can create different output files for
parallelograms with different parameters and analyze the correctness of the result.

### Result

#### Output_1

#### Parameters:

```
// for the parallelogram
Vector3d pgram_origin(-0.7, -0.5, 0);
Vector3d pgram_u(0.5, 1.0, 0);
Vector3d pgram_v(1.0, 0, 0);

// for the ray
Vector3d origin(-1, 1, 1);
Vector3d ray_origin = origin + double(i) * x_displacement + double(j) * y_displacement;
Vector3d ray_direction = RowVector3d(0, 0, -1);
```

![](result/plane_orthographic(output_1).png?raw=true)

#### Analysis:

The parallelogram parallels with XOY plane. So the projection is exactly the same as itself.

#### Output_2

#### Parameters:

```
// for the parallelogram
Vector3d pgram_origin(-0.7, -0.5, 0);
Vector3d pgram_u(0.5, 1.0, 0.5);
Vector3d pgram_v(1.0, 0, 0.5);

// for the ray
Vector3d origin(-1, 1, 1);
Vector3d ray_origin = origin + double(i) * x_displacement + double(j) * y_displacement;
Vector3d ray_direction = RowVector3d(0, 0, -1);
```

![](result/plane_orthographic(output_2).png?raw=true)

#### Analysis:

Now the z-coordinate of `pgram_u` and `pgram_v` is 0.5. We can see the shape of the projection is the same as output_1.
Because the ray direction is parallel to z-axis, and the height of the parallelogram will not affect the projection on
XOY plane. However, the color becomes a little darker because the normal of the parallelogram has changed.

### Ray Tracing with Perspective Projection

In the `raytrace_perspective` function, we do perspective projection on the parallelogram at first. We also
use `raygram` to check for the intersection. The ray origin will be the same one while the directions are different.

### Result

#### Output_3

#### Parameters:

```
// for the parallelogram
Vector3d pgram_origin(-0.7, -0.5, 0);
Vector3d pgram_u(0.5, 1.0, 0);
Vector3d pgram_v(1.0, 0, 0);

// for the ray
Vector3d origin(-1, 1, 1);
Vector3d ray_origin(0, 0, 2);
Vector3d ray_direction = (origin + double(i) * x_displacement + double(j) * y_displacement - ray_origin).normalized();
```

![](result/plane_perspective(output_3).png?raw=true)

#### Analysis:

The shape and position of the parallelogram here is the same as output_1 above. The projection is also a parallelogram
because the parallelogram is parallel to the sensor. And the projection becomes smaller because the sensor is the XOY
plane with z = 1. It is closer to the camera than the object. This will make the projection smaller than the real
object.

#### Output_4

#### Parameters:

```
// for the parallelogram
Vector3d pgram_origin(-0.7, -0.5, 0);
Vector3d pgram_u(0.5, 1.0, 0.5);
Vector3d pgram_v(1.0, 0, 0.5);

// for the ray
Vector3d origin(-1, 1, 1);
Vector3d ray_origin(0, 0, 2);
Vector3d ray_direction = (origin + double(i) * x_displacement + double(j) * y_displacement - ray_origin).normalized();
```

![](result/plane_perspective(output_4).png?raw=true)

#### Analysis:

Now we change `pgram_u` and `pgram_v` so that the shape is the same as output_2. In this way, we can see the projection
is not a normal parallelogram. This is because the upper-right corner of the parallelogram is closer to the camera in
z-axis. So it should become larger.

Then we create the perspective projection on both the sphere and the parallelogram in the same scene and compare the
difference.

#### Output_5

#### Parameters:

```
// for the parallelogram
Vector3d pgram_origin(0.2, 0.7, 0);
Vector3d pgram_u(0.3, 0.7, 0);
Vector3d pgram_v(0.7, 0, 0);

// for the sphere
const double sphere_radius = 0.5;
Vector3d sphere_center(0, 0, 0);

// for the ray
Vector3d origin(-1, 1, 1);
Vector3d ray_origin(0, 0, 2);
Vector3d ray_direction = (origin + double(i) * x_displacement + double(j) * y_displacement - ray_origin).normalized();
```

![](result/plane_perspective(output_5).png?raw=true)

#### Output_6

#### Parameters:

```
// for the parallelogram
Vector3d pgram_origin(0.2, 0.7, 0);
Vector3d pgram_u(0.3, 0.7, 0.2);
Vector3d pgram_v(0.7, 0, 0.2);

// for the sphere
const double sphere_radius = 0.5;
Vector3d sphere_center(-1, -1, 0);

// for the ray
Vector3d origin(-1, 1, 1);
Vector3d ray_origin(0, 0, 2);
Vector3d ray_direction = (origin + double(i) * x_displacement + double(j) * y_displacement - ray_origin).normalized();
```

![](result/plane_perspective(output_6).png?raw=true)

#### Analysis:

In output_5, the shapes of two objects are normal while in output_6 they are stretched. When we analyze the parameters,
we can find that for the parallelogram, the shape of the projection is normal only when the object is parallel to the
sensor plane. And for the sphere, it is normal only when the ray origin has the same x-coordinate and y-coordinate as
the center of the sphere.

Ex.2: Shading
----------------------

In the `raytrace_shading`, we will create the perspective projection for both sphere and parallelogram in the same
scene. And then we will implement the shading components for them in RGB. The position and the shape of the objects will
be the same as output_5.

#### Output_7

#### Parameters:

```
// for shading
R(i, j) = 0.5 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 20);
G(i, j) = 0.5 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 20);
B(i, j) = 0.5 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 20);
```

![](result/shading(output_7).png?raw=true)

#### Output_8

#### Parameters:

```
// for shading
R(i, j) = 0.5 * ambient + 1 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 20);
G(i, j) = 0.5 * ambient + 1 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 20);
B(i, j) = 0.5 * ambient + 1 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 20);
```

![](result/shading(output_8).png?raw=true)

#### Output_9

#### Parameters:

```
// for shading
R(i, j) = 0.5 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 1 * pow(std::max(specular(i, j), 0.), 20);
G(i, j) = 0.5 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 1 * pow(std::max(specular(i, j), 0.), 20);
B(i, j) = 0.5 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 1 * pow(std::max(specular(i, j), 0.), 20);
```

![](result/shading(output_9).png?raw=true)

#### Output_10

#### Parameters:

```
// for shading
R(i, j) = 0.5 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 100);
G(i, j) = 0.5 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 100);
B(i, j) = 0.5 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 100);
```

![](result/shading(output_10).png?raw=true)

#### Output_11

#### Parameters:

```
// for shading
R(i, j) = 1 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 20);
G(i, j) = 1 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 20);
B(i, j) = 1 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 20);
```

![](result/shading(output_11).png?raw=true)

#### Analysis:

Compared output_7 and output_8, the diffuse coefficient becomes larger. The whole side of the sphere becomes brighter.
Compared output_7 and output_9, the specular coefficient becomes larger. The reflection area on the sphere becomes
brighter. Compared output_7 and output_10, the phong exponent becomes larger. The reflection area on the sphere becomes
smaller. Compared output_7 and output_11, the ambient exponent becomes larger. The whole world becomes brighter.

#### Output_12

#### Parameters:

```
// for shading
R(i, j) = 0;
G(i, j) = 0.5 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 20);
B(i, j) = 0.5 * ambient + 0.6 * std::max(diffuse(i, j), 0.) + 0.5 * pow(std::max(specular(i, j), 0.), 20);
```

![](result/shading(output_12).png?raw=true)

#### Analysis:

This is the output when there is no red ray in the world. We can also change the matrix R, G or B to create different
color.