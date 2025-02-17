#pragma once

#include <Eigen/Core>

class VertexAttributes {
public:
    VertexAttributes(float x = 0, float y = 0, float z = 0, float w = 1) {
        position << x, y, z, w;
        color << 1, 1, 1, 1;
    }

    // Interpolates the vertex attributes
    static VertexAttributes interpolate(
            const VertexAttributes &a,
            const VertexAttributes &b,
            const VertexAttributes &c,
            const float alpha,
            const float beta,
            const float gamma
    ) {
        VertexAttributes r;
        r.position = alpha * a.position + beta * b.position + gamma * c.position;
        r.color = alpha * a.color + beta * b.color + gamma * c.color;
        r.triangle_index = a.triangle_index;
        return r;
    }

    Eigen::Vector4f position;
    Eigen::Vector4f color;
    int triangle_index = -1;
};

class FragmentAttributes {
public:
    FragmentAttributes(float r = 0, float g = 0, float b = 0, float a = 1) {
        color << r, g, b, a;
    }

    Eigen::Vector4f color;
    float depth = 0;
};

class FrameBufferAttributes {
public:
    FrameBufferAttributes(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255) {
        color << r, g, b, a;
    }

    Eigen::Matrix<uint8_t, 4, 1> color;
    float depth = 0;
};

class UniformAttributes {
public:
    Eigen::Matrix4f camera_transform;
    int selected_triangle = -1;
    Eigen::Vector4f selected_triangle_color[3];
    int selected_vertex = -1;
    Eigen::Vector4f colors[9] = {Eigen::Vector4f(0, 0, 0, 1),
                                 Eigen::Vector4f(1, 1, 1, 1),
                                 Eigen::Vector4f(1, 0, 0, 1),
                                 Eigen::Vector4f(0, 1, 0, 1),
                                 Eigen::Vector4f(0, 0, 1, 1),
                                 Eigen::Vector4f(1, 1, 0, 1),
                                 Eigen::Vector4f(1, 0, 1, 1),
                                 Eigen::Vector4f(0, 1, 1, 1),
                                 Eigen::Vector4f(0.5, 0.5, 0.5, 1),};
};