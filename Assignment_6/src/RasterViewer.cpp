#include "SDLViewer.h"

#include <Eigen/Core>
#include <Eigen/Dense>

#include <functional>
#include <iostream>

#include "raster.h"

// Image writing library
#define STB_IMAGE_WRITE_IMPLEMENTATION // Do not include this line twice in your project!

#include "stb_image_write.h"

bool inside_triangle(const Eigen::Vector4f &A, const Eigen::Vector4f &B, const Eigen::Vector4f &C,
                     const Eigen::Vector4f &P) {
    Eigen::Vector4f AB = B - A;
    Eigen::Vector4f AC = C - A;
    Eigen::Vector4f AP = P - A;
    Eigen::Vector4f Zero(0, 0, 0, 0);
    Eigen::Matrix4f M;
    M << AB, AC, Zero, Zero;
    Eigen::Vector4f x = M.colPivHouseholderQr().solve(AP);
    return x(0) >= 0 && x(1) >= 0 && x(0) + x(1) <= 1;
}

int main(int argc, char *args[]) {
    int width = 500;
    int height = 500;
    // The Framebuffer storing the image rendered by the rasterizer
    Eigen::Matrix<FrameBufferAttributes, Eigen::Dynamic, Eigen::Dynamic> frameBuffer(width, height);

    // Global Constants (empty in this example)
    UniformAttributes uniform;

    // Basic rasterization program
    Program program;

    // The vertex shader is the identity
    program.VertexShader = [](const VertexAttributes &va, const UniformAttributes &uniform) {
        if (va.triangle_index == -1) return va;
        else {
            VertexAttributes out;
            out.color = va.color;
            out.triangle_index = va.triangle_index;
            out.position = uniform.transforms[va.triangle_index] * va.position;
            return out;
        }
    };

    // The fragment shader uses a fixed color
    program.FragmentShader = [](const VertexAttributes &va, const UniformAttributes &uniform) {
        FragmentAttributes out(va.color(0), va.color(1), va.color(2));
        out.depth = va.position(2);
        return out;
    };

    // The blending shader converts colors between 0 and 1 to uint8
    program.BlendingShader = [](const FragmentAttributes &fa, const FrameBufferAttributes &previous) {
        if (fa.depth > previous.depth + 0.0001) {
            FrameBufferAttributes out(fa.color[0] * 255, fa.color[1] * 255, fa.color[2] * 255, fa.color[3] * 255);
            out.depth = fa.depth;
            return out;
        } else return previous;
    };

    std::vector<VertexAttributes> vertices_triangles;
    std::vector<VertexAttributes> vertices_lines;

    enum Mode {
        NONE, INSERTION, TRANSLATION, DELETE
    } mode;
    mode = NONE;
    enum Insertion_Step {
        ONE, TWO, THREE
    } insertion_step;
    insertion_step = ONE;

    float depth = 0.01;  // Insertion Mode: z-buffer
    Eigen::Vector2f position_origin(0, 0);  // origin mouse position
    bool is_moving = false;

    // Initialize the viewer and the corresponding callbacks
    SDLViewer viewer;
    viewer.init("Viewer Example", width, height);

    viewer.mouse_move = [&](int x, int y, int xrel, int yrel) {
        if (mode == INSERTION) {
            if (insertion_step == TWO) {
                unsigned base = vertices_lines.size() / 6 * 6;
                vertices_lines[base + 1].position << (float(x) / float(width) * 2) - 1,
                        (float(height - 1 - y) / float(height) * 2) - 1, depth, 1;
            } else if (insertion_step == THREE) {
                unsigned base = (vertices_lines.size() / 6 - 1) * 6;
                vertices_lines[base + 3].position << (float(x) / float(width) * 2) - 1,
                        (float(height - 1 - y) / float(height) * 2) - 1, depth, 1;
                vertices_lines[base + 4].position << (float(x) / float(width) * 2) - 1,
                        (float(height - 1 - y) / float(height) * 2) - 1, depth, 1;
            }
        } else if (mode == TRANSLATION) {
            if (is_moving && uniform.selected_index != -1) {
                Eigen::Vector2f move = Eigen::Vector2f((float(x) / float(width) * 2) - 1,
                                                       (float(height - 1 - y) / float(height) * 2) - 1) -
                                       position_origin;
                Eigen::Matrix4f transform;
                transform << 1, 0, 0, move(0),
                        0, 1, 0, move(1),
                        0, 0, 1, 0,
                        0, 0, 0, 1;
                uniform.transforms[uniform.selected_index] = transform * uniform.transforms[uniform.selected_index];
                position_origin << (float(x) / float(width) * 2) - 1, (float(height - 1 - y) / float(height) * 2) - 1;
            }
        }
        viewer.redraw_next = true;
    };

    viewer.mouse_pressed = [&](int x, int y, bool is_pressed, int button, int clicks) {
        if (is_pressed) {
            // Pressed
            if (mode == INSERTION) {
                if (insertion_step == ONE) {
                    VertexAttributes vertex((float(x) / float(width) * 2) - 1,
                                            (float(height - 1 - y) / float(height) * 2) - 1,
                                            depth);
                    vertex.color << 0, 0, 0, 1;
                    vertices_lines.push_back(vertex);
                    vertices_lines.push_back(vertex);
                    insertion_step = TWO;
                } else if (insertion_step == TWO) {
                    VertexAttributes vertex((float(x) / float(width) * 2) - 1,
                                            (float(height - 1 - y) / float(height) * 2) - 1,
                                            depth);
                    vertex.color << 0, 0, 0, 1;
                    vertices_lines.push_back(vertex);
                    vertices_lines.push_back(vertex);
                    vertices_lines.push_back(vertex);
                    unsigned base = vertices_lines.size() / 6 * 6;
                    vertices_lines.push_back(vertices_lines[base + 0]);
                    insertion_step = THREE;
                } else if (insertion_step == THREE) {
                    unsigned base = (vertices_lines.size() / 6 - 1) * 6;
                    int triangle_index = (int) vertices_triangles.size() / 3;
                    for (unsigned i = 0; i < 6; i++)
                        vertices_lines[base + i].triangle_index = triangle_index;
                    for (unsigned i = 0; i < 3; i++) {
                        vertices_triangles.push_back(vertices_lines[base + i * 2]);
                        vertices_triangles[vertices_triangles.size() - 1].color << 1, 0, 0, 1;
                    }
                    Eigen::Matrix4f transform;
                    transform << 1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1;
                    uniform.transforms.push_back(transform);
                    depth += 0.01;
                    insertion_step = ONE;
                }
            } else if (mode == TRANSLATION) {
                uniform.selected_index = -1;
                for (int i = 0; i < vertices_triangles.size() / 3; i++) {
                    int index = vertices_triangles[i * 3 + 0].triangle_index;
                    Eigen::Vector4f A(vertices_triangles[i * 3 + 0].position[0],
                                      vertices_triangles[i * 3 + 0].position[1], 0, 1);
                    Eigen::Vector4f B(vertices_triangles[i * 3 + 1].position[0],
                                      vertices_triangles[i * 3 + 1].position[1], 0, 1);
                    Eigen::Vector4f C(vertices_triangles[i * 3 + 2].position[0],
                                      vertices_triangles[i * 3 + 2].position[1], 0, 1);
                    Eigen::Vector4f P((float(x) / float(width) * 2) - 1,
                                      (float(height - 1 - y) / float(height) * 2) - 1, 0, 1);
                    if (inside_triangle(uniform.transforms[index] * A, uniform.transforms[index] * B,
                                        uniform.transforms[index] * C, P))
                        uniform.selected_index = i;
                }
                if (uniform.selected_index != -1) {
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_index * 3 + i].color << 0, 0, 1, 1;
                    is_moving = true;
                }
                position_origin << (float(x) / float(width) * 2) - 1, (float(height - 1 - y) / float(height) * 2) - 1;
            } else if (mode == DELETE) {
                uniform.selected_index = -1;
                for (int i = 0; i < vertices_triangles.size() / 3; i++) {
                    int index = vertices_triangles[i * 3 + 0].triangle_index;
                    Eigen::Vector4f A(vertices_triangles[i * 3 + 0].position[0],
                                      vertices_triangles[i * 3 + 0].position[1], 0, 1);
                    Eigen::Vector4f B(vertices_triangles[i * 3 + 1].position[0],
                                      vertices_triangles[i * 3 + 1].position[1], 0, 1);
                    Eigen::Vector4f C(vertices_triangles[i * 3 + 2].position[0],
                                      vertices_triangles[i * 3 + 2].position[1], 0, 1);
                    Eigen::Vector4f P((float(x) / float(width) * 2) - 1,
                                      (float(height - 1 - y) / float(height) * 2) - 1, 0, 1);
                    if (inside_triangle(uniform.transforms[index] * A, uniform.transforms[index] * B,
                                        uniform.transforms[index] * C, P))
                        uniform.selected_index = i;
                }
                if (uniform.selected_index != -1) {
                    unsigned base_triangle = (vertices_triangles.size() / 3 - 1) * 3;
                    for (unsigned i = 0; i < 3; i++) {
                        vertices_triangles[uniform.selected_index * 3 + i] = vertices_triangles[base_triangle];
                        vertices_triangles[uniform.selected_index * 3 + i].triangle_index = uniform.selected_index;
                        vertices_triangles.erase(vertices_triangles.begin() + base_triangle);
                    }
                    unsigned base_line = (vertices_lines.size() / 6 - 1) * 6;
                    for (unsigned i = 0; i < 6; i++) {
                        vertices_lines[uniform.selected_index * 6 + i] = vertices_lines[base_line];
                        vertices_lines[uniform.selected_index * 6 + i].triangle_index = uniform.selected_index;
                        vertices_lines.erase(vertices_lines.begin() + base_line);
                    }
                    uniform.transforms[uniform.selected_index] = uniform.transforms[uniform.transforms.size() - 1];
                    uniform.transforms.erase(uniform.transforms.end() - 1);
                    uniform.selected_index = -1;
                }
            }
        } else {
            // Released
            if (mode == TRANSLATION) {
                is_moving = false;
                position_origin << 0, 0;
                if (uniform.selected_index != -1)
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_index * 3 + i].color << 1, 0, 0, 1;
//                uniform.selected_index = -1;
            }
        }
        viewer.redraw_next = true;
    };

    viewer.mouse_wheel = [&](int dx, int dy, bool is_direction_normal) {
    };

    viewer.key_pressed = [&](char key, bool is_pressed, int modifier, int repeat) {
        if (is_pressed) {
            if (key == 'i') {
                mode = INSERTION;
                // disable translation mode
                position_origin << 0, 0;
                if (uniform.selected_index != -1)
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_index * 3 + i].color << 1, 0, 0, 1;
                uniform.selected_index = -1;
            } else if (key == 'o') {
                mode = TRANSLATION;
                // disable insertion mode
                int num = 0;
                if (insertion_step == TWO) num = 2;
                else if (insertion_step == THREE) num = 6;
                while (num-- > 0)
                    vertices_lines.erase(vertices_lines.end() - 1);
                insertion_step = ONE;
            } else if (key == 'p') {
                mode = DELETE;
                // disable insertion mode
                int num = 0;
                if (insertion_step == TWO) num = 2;
                else if (insertion_step == THREE) num = 6;
                while (num-- > 0)
                    vertices_lines.erase(vertices_lines.end() - 1);
                insertion_step = ONE;
                // disable translation mode
                position_origin << 0, 0;
                if (uniform.selected_index != -1)
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_index * 3 + i].color << 1, 0, 0, 1;
                uniform.selected_index = -1;
            } else if (key == 'h' || key == 'j') {
                if (mode == TRANSLATION && uniform.selected_index != -1) {
                    Eigen::Vector4f barycenter(0, 0, 0, 0);
                    for (unsigned i = 0; i < 3; i++)
                        barycenter += uniform.transforms[uniform.selected_index] *
                                      vertices_triangles[uniform.selected_index * 3 + i].position;
                    barycenter /= 3;
                    float degree = key == 'h' ? -10 : 10;
                    Eigen::Matrix4f moveToOrigin;
                    moveToOrigin << 1, 0, 0, -barycenter(0),
                            0, 1, 0, -barycenter(1),
                            0, 0, 1, -barycenter(2),
                            0, 0, 0, 1;
                    Eigen::Matrix4f Rotate;
                    float radian = degree / 180 * (float) M_PI;
                    Rotate << cos(radian), -sin(radian), 0, 0,
                            sin(radian), cos(radian), 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1;
                    Eigen::Matrix4f moveBack;
                    moveBack << 1, 0, 0, barycenter(0),
                            0, 1, 0, barycenter(1),
                            0, 0, 1, barycenter(2),
                            0, 0, 0, 1;
                    uniform.transforms[uniform.selected_index] =
                            moveBack * Rotate * moveToOrigin * uniform.transforms[uniform.selected_index];
                }
            } else if (key == 'k' || key == 'l') {
                if (mode == TRANSLATION && uniform.selected_index != -1) {
                    Eigen::Vector4f barycenter(0, 0, 0, 0);
                    for (unsigned i = 0; i < 3; i++)
                        barycenter += uniform.transforms[uniform.selected_index] *
                                      vertices_triangles[uniform.selected_index * 3 + i].position;
                    barycenter /= 3;
                    float scale = key == 'k' ? 0.25 : -0.25;
                    Eigen::Matrix4f moveToOrigin;
                    moveToOrigin << 1, 0, 0, -barycenter(0),
                            0, 1, 0, -barycenter(1),
                            0, 0, 1, -barycenter(2),
                            0, 0, 0, 1;
                    Eigen::Matrix4f Scale;
                    Scale << 1 + scale, 0, 0, 0,
                            0, 1 + scale, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1;
                    Eigen::Matrix4f moveBack;
                    moveBack << 1, 0, 0, barycenter(0),
                            0, 1, 0, barycenter(1),
                            0, 0, 1, barycenter(2),
                            0, 0, 0, 1;
                    uniform.transforms[uniform.selected_index] =
                            moveBack * Scale * moveToOrigin * uniform.transforms[uniform.selected_index];
                }
            } else {
                mode = NONE;
                // disable insertion mode
                int num = 0;
                if (insertion_step == TWO) num = 2;
                else if (insertion_step == THREE) num = 6;
                while (num-- > 0)
                    vertices_lines.erase(vertices_lines.end() - 1);
                insertion_step = ONE;
                // disable translation mode
                position_origin << 0, 0;
                if (uniform.selected_index != -1)
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_index * 3 + i].color << 1, 0, 0, 1;
                uniform.selected_index = -1;
            }
        }
        viewer.redraw_next = true;
    };

    viewer.redraw = [&](SDLViewer &viewer) {
        // Clear the framebuffer
        for (unsigned i = 0; i < frameBuffer.rows(); i++)
            for (unsigned j = 0; j < frameBuffer.cols(); j++) {
                frameBuffer(i, j).color << 255, 255, 255, 255;
                frameBuffer(i, j).depth = 0;
            }

        rasterize_lines(program, uniform, vertices_lines, 1.0, frameBuffer);
        rasterize_triangles(program, uniform, vertices_triangles, frameBuffer);

        // Buffer for exchanging data between rasterizer and sdl viewer
        Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic> R(width, height);
        Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic> G(width, height);
        Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic> B(width, height);
        Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic> A(width, height);

        for (unsigned i = 0; i < frameBuffer.rows(); i++) {
            for (unsigned j = 0; j < frameBuffer.cols(); j++) {
                R(i, frameBuffer.cols() - 1 - j) = frameBuffer(i, j).color(0);
                G(i, frameBuffer.cols() - 1 - j) = frameBuffer(i, j).color(1);
                B(i, frameBuffer.cols() - 1 - j) = frameBuffer(i, j).color(2);
                A(i, frameBuffer.cols() - 1 - j) = frameBuffer(i, j).color(3);
            }
        }
        viewer.draw_image(R, G, B, A);
    };

    viewer.launch();

    return 0;
}