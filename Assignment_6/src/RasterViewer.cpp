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

int factorial(int num) {
    int res = 1;
    while (num > 0) {
        res *= num;
        num--;
    }
    return res;
}

int combination(int n, int i) {
    return factorial(n) / (factorial(i) * factorial(n - i));
}

int main(int argc, char *args[]) {
    int width = 800;
    int height = 800;
    // The Framebuffer storing the image rendered by the rasterizer
    Eigen::Matrix<FrameBufferAttributes, Eigen::Dynamic, Eigen::Dynamic> frameBuffer(width, height);

    // Global Constants (empty in this example)
    UniformAttributes uniform;

    // Basic rasterization program
    Program program;

    std::vector<VertexAttributes> vertices_triangles;
    std::vector<VertexAttributes> vertices_lines;
    std::vector<Eigen::Matrix4f> transforms;
    std::vector<std::vector<Eigen::Matrix4f>> keyframes;

    // The vertex shader is the identity
    program.VertexShader = [&](const VertexAttributes &va, const UniformAttributes &uniform) {
        VertexAttributes out = va;
        if (va.triangle_index == -1)
            out.position = uniform.camera_transform * out.position;
        else
            out.position = uniform.camera_transform * transforms[out.triangle_index] * out.position;
        return out;
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

    enum Mode {
        NONE, INSERTION, TRANSLATION, ROTATION_SCALE, DELETE, COLOR, ANIMATION
    } mode;
    mode = NONE;
    enum Insertion_Step {
        ONE, TWO, THREE
    } insertion_step;
    insertion_step = ONE;

    float depth = 0.001;  // Insertion Mode: z-buffer
    Eigen::Vector2f position_origin(0, 0);  // origin mouse position
    bool is_moving = false;
    uniform.camera_transform << 1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;

    // Initialize the viewer and the corresponding callbacks
    SDLViewer viewer;
    viewer.init("Viewer Example", width, height);

    viewer.mouse_move = [&](int x, int y, int xrel, int yrel) {
        Eigen::Vector4f position_current((float(x) / float(width) * 2) - 1,
                                         (float(height - 1 - y) / float(height) * 2) - 1, 0, 1);
        position_current = uniform.camera_transform.inverse() * position_current;
        if (mode == INSERTION) {
            if (insertion_step == TWO) {
                unsigned base = vertices_lines.size() / 6 * 6;
                vertices_lines[base + 1].position << position_current(0), position_current(1), depth, 1;
            } else if (insertion_step == THREE) {
                unsigned base = (vertices_lines.size() / 6 - 1) * 6;
                vertices_lines[base + 3].position << position_current(0), position_current(1), depth, 1;
                vertices_lines[base + 4].position << position_current(0), position_current(1), depth, 1;
            }
        } else if (mode == TRANSLATION || mode == ANIMATION) {
            if (is_moving && uniform.selected_triangle != -1) {
                Eigen::Vector2f move = Eigen::Vector2f(position_current(0), position_current(1)) - position_origin;
                Eigen::Matrix4f transform;
                transform << 1, 0, 0, move(0),
                        0, 1, 0, move(1),
                        0, 0, 1, 0,
                        0, 0, 0, 1;
                transforms[uniform.selected_triangle] =
                        transform * transforms[uniform.selected_triangle];
                position_origin << position_current(0), position_current(1);
            }
        }
        viewer.redraw_next = true;
    };

    viewer.mouse_pressed = [&](int x, int y, bool is_pressed, int button, int clicks) {
        Eigen::Vector4f position_current((float(x) / float(width) * 2) - 1,
                                         (float(height - 1 - y) / float(height) * 2) - 1, 0, 1);
        position_current = uniform.camera_transform.inverse() * position_current;
        if (is_pressed) {
            // Pressed
            if (mode == INSERTION) {
                if (insertion_step == ONE) {
                    VertexAttributes vertex(position_current(0), position_current(1), depth);
                    vertex.color << 0, 0, 0, 1;
                    vertices_lines.push_back(vertex);
                    vertices_lines.push_back(vertex);
                    insertion_step = TWO;
                } else if (insertion_step == TWO) {
                    VertexAttributes vertex(position_current(0), position_current(1), depth);
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
                    transforms.push_back(transform);
                    depth += 0.001;
                    insertion_step = ONE;
                }
            } else if (mode == TRANSLATION || mode == ANIMATION) {
                uniform.selected_triangle = -1;
                for (int i = 0; i < vertices_triangles.size() / 3; i++) {
                    if (uniform.selected_triangle != -1 &&
                        vertices_triangles[i * 3].position(2) <
                        vertices_triangles[uniform.selected_triangle * 3].position(2))
                        continue;
                    int index = vertices_triangles[i * 3 + 0].triangle_index;
                    Eigen::Vector4f A(vertices_triangles[i * 3 + 0].position[0],
                                      vertices_triangles[i * 3 + 0].position[1], 0, 1);
                    Eigen::Vector4f B(vertices_triangles[i * 3 + 1].position[0],
                                      vertices_triangles[i * 3 + 1].position[1], 0, 1);
                    Eigen::Vector4f C(vertices_triangles[i * 3 + 2].position[0],
                                      vertices_triangles[i * 3 + 2].position[1], 0, 1);
                    Eigen::Vector4f P(position_current(0), position_current(1), 0, 1);
                    if (inside_triangle(transforms[index] * A, transforms[index] * B,
                                        transforms[index] * C, P))
                        uniform.selected_triangle = i;
                }
                if (uniform.selected_triangle != -1) {
                    for (unsigned i = 0; i < 3; i++) {
                        uniform.selected_triangle_color[i] =
                                vertices_triangles[uniform.selected_triangle * 3 + i].color;
                        vertices_triangles[uniform.selected_triangle * 3 + i].color << 0, 0, 1, 1;
                    }
                    is_moving = true;
                }
                position_origin << position_current(0), position_current(1);
            } else if (mode == ROTATION_SCALE) {
                if (uniform.selected_triangle != -1) {
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_triangle * 3 + i].color
                                << uniform.selected_triangle_color[i];
                    uniform.selected_triangle = -1;
                }
                for (int i = 0; i < vertices_triangles.size() / 3; i++) {
                    if (uniform.selected_triangle != -1 &&
                        vertices_triangles[i * 3].position(2) <
                        vertices_triangles[uniform.selected_triangle * 3].position(2))
                        continue;
                    int index = vertices_triangles[i * 3 + 0].triangle_index;
                    Eigen::Vector4f A(vertices_triangles[i * 3 + 0].position[0],
                                      vertices_triangles[i * 3 + 0].position[1], 0, 1);
                    Eigen::Vector4f B(vertices_triangles[i * 3 + 1].position[0],
                                      vertices_triangles[i * 3 + 1].position[1], 0, 1);
                    Eigen::Vector4f C(vertices_triangles[i * 3 + 2].position[0],
                                      vertices_triangles[i * 3 + 2].position[1], 0, 1);
                    Eigen::Vector4f P(position_current(0), position_current(1), 0, 1);
                    if (inside_triangle(transforms[index] * A, transforms[index] * B,
                                        transforms[index] * C, P))
                        uniform.selected_triangle = i;
                }
                if (uniform.selected_triangle != -1) {
                    for (unsigned i = 0; i < 3; i++) {
                        uniform.selected_triangle_color[i] =
                                vertices_triangles[uniform.selected_triangle * 3 + i].color;
                        vertices_triangles[uniform.selected_triangle * 3 + i].color << 0, 0, 1, 1;
                    }
                }
            } else if (mode == DELETE) {
                uniform.selected_triangle = -1;
                for (int i = 0; i < vertices_triangles.size() / 3; i++) {
                    if (uniform.selected_triangle != -1 &&
                        vertices_triangles[i * 3].position(2) <
                        vertices_triangles[uniform.selected_triangle * 3].position(2))
                        continue;
                    int index = vertices_triangles[i * 3 + 0].triangle_index;
                    Eigen::Vector4f A(vertices_triangles[i * 3 + 0].position[0],
                                      vertices_triangles[i * 3 + 0].position[1], 0, 1);
                    Eigen::Vector4f B(vertices_triangles[i * 3 + 1].position[0],
                                      vertices_triangles[i * 3 + 1].position[1], 0, 1);
                    Eigen::Vector4f C(vertices_triangles[i * 3 + 2].position[0],
                                      vertices_triangles[i * 3 + 2].position[1], 0, 1);
                    Eigen::Vector4f P(position_current(0), position_current(1), 0, 1);
                    if (inside_triangle(transforms[index] * A, transforms[index] * B,
                                        transforms[index] * C, P))
                        uniform.selected_triangle = i;
                }
                if (uniform.selected_triangle != -1) {
                    unsigned base_triangle = (vertices_triangles.size() / 3 - 1) * 3;
                    for (unsigned i = 0; i < 3; i++) {
                        vertices_triangles[uniform.selected_triangle * 3 + i] = vertices_triangles[base_triangle];
                        vertices_triangles[uniform.selected_triangle * 3 +
                                           i].triangle_index = uniform.selected_triangle;
                        vertices_triangles.erase(vertices_triangles.begin() + base_triangle);
                    }
                    unsigned base_line = (vertices_lines.size() / 6 - 1) * 6;
                    for (unsigned i = 0; i < 6; i++) {
                        vertices_lines[uniform.selected_triangle * 6 + i] = vertices_lines[base_line];
                        vertices_lines[uniform.selected_triangle * 6 + i].triangle_index = uniform.selected_triangle;
                        vertices_lines.erase(vertices_lines.begin() + base_line);
                    }
                    transforms[uniform.selected_triangle] = transforms[transforms.size() - 1];
                    transforms.erase(transforms.end() - 1);
                    uniform.selected_triangle = -1;
                }
            } else if (mode == COLOR) {
                uniform.selected_vertex = -1;
                Eigen::Vector2f mouse_position(position_current(0), position_current(1));
                for (int i = 0; i < vertices_triangles.size(); i++) {
                    if (uniform.selected_vertex == -1)
                        uniform.selected_vertex = i;
                    else {
                        Eigen::Vector4f vertex_position_4 = transforms[vertices_triangles[i].triangle_index] *
                                                            vertices_triangles[i].position;
                        Eigen::Vector2f vertex_position(vertex_position_4(0), vertex_position_4(1));
                        Eigen::Vector4f selected_vertex_position_4 =
                                transforms[vertices_triangles[uniform.selected_vertex].triangle_index] *
                                vertices_triangles[uniform.selected_vertex].position;
                        Eigen::Vector2f selected_vertex_position(selected_vertex_position_4(0),
                                                                 selected_vertex_position_4(1));
                        if ((vertex_position - mouse_position).norm() <
                            (selected_vertex_position - mouse_position).norm())
                            uniform.selected_vertex = i;
                    }
                }
            }
        } else {
            // Released
            if (mode == TRANSLATION || mode == ANIMATION) {
                is_moving = false;
                position_origin << 0, 0;
                if (uniform.selected_triangle != -1) {
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_triangle * 3 + i].color
                                << uniform.selected_triangle_color[i];

                }
                uniform.selected_triangle = -1;
            }
        }
        viewer.redraw_next = true;
    };

    viewer.mouse_wheel = [&](int dx, int dy, bool is_direction_normal) {
    };

    viewer.key_pressed = [&](char key, bool is_pressed, int modifier, int repeat) {
        if (is_pressed) {
            if (key == 'i') {
                if (mode == INSERTION) return;
                mode = INSERTION;
                // disable translation or rotation_scale or animation mode
                position_origin << 0, 0;
                if (uniform.selected_triangle != -1)
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_triangle * 3 + i].color
                                << uniform.selected_triangle_color[i];
                uniform.selected_triangle = -1;
                // disable animation mode
                if (!keyframes.empty()) {
                    transforms = keyframes[0];
                    keyframes.clear();
                }
                // disable color mode
                uniform.selected_vertex = -1;
            } else if (key == 'o') {
                if (mode == TRANSLATION) return;
                mode = TRANSLATION;
                // disable insertion mode
                int num = 0;
                if (insertion_step == TWO) num = 2;
                else if (insertion_step == THREE) num = 6;
                while (num-- > 0)
                    vertices_lines.erase(vertices_lines.end() - 1);
                insertion_step = ONE;
                // disable rotation_scale mode
                if (uniform.selected_triangle != -1)
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_triangle * 3 + i].color
                                << uniform.selected_triangle_color[i];
                uniform.selected_triangle = -1;
                // disable color mode
                uniform.selected_vertex = -1;
                // disable animation mode
                position_origin << 0, 0;
                if (uniform.selected_triangle != -1)
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_triangle * 3 + i].color
                                << uniform.selected_triangle_color[i];
                uniform.selected_triangle = -1;
                if (!keyframes.empty()) {
                    transforms = keyframes[0];
                    keyframes.clear();
                }
            } else if (key == 'u') {
                if (mode == ROTATION_SCALE) return;
                mode = ROTATION_SCALE;
                // disable insertion mode
                int num = 0;
                if (insertion_step == TWO) num = 2;
                else if (insertion_step == THREE) num = 6;
                while (num-- > 0)
                    vertices_lines.erase(vertices_lines.end() - 1);
                insertion_step = ONE;
                // disable translation or animation mode
                position_origin << 0, 0;
                if (uniform.selected_triangle != -1)
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_triangle * 3 + i].color
                                << uniform.selected_triangle_color[i];
                uniform.selected_triangle = -1;
                // disable color mode
                uniform.selected_vertex = -1;
                // disable animation mode
                if (!keyframes.empty()) {
                    transforms = keyframes[0];
                    keyframes.clear();
                }
            } else if (key == 'p') {
                if (mode == DELETE) return;
                mode = DELETE;
                // disable insertion mode
                int num = 0;
                if (insertion_step == TWO) num = 2;
                else if (insertion_step == THREE) num = 6;
                while (num-- > 0)
                    vertices_lines.erase(vertices_lines.end() - 1);
                insertion_step = ONE;
                // disable translation or rotation_scale or animation mode
                position_origin << 0, 0;
                if (uniform.selected_triangle != -1)
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_triangle * 3 + i].color
                                << uniform.selected_triangle_color[i];
                uniform.selected_triangle = -1;
                // disable color mode
                uniform.selected_vertex = -1;
                // disable animation mode
                if (!keyframes.empty()) {
                    transforms = keyframes[0];
                    keyframes.clear();
                }
            } else if (key == 'c') {
                if (mode == COLOR) return;
                mode = COLOR;
                // disable insertion mode
                int num = 0;
                if (insertion_step == TWO) num = 2;
                else if (insertion_step == THREE) num = 6;
                while (num-- > 0)
                    vertices_lines.erase(vertices_lines.end() - 1);
                insertion_step = ONE;
                // disable translation or rotation_scale or animation mode
                position_origin << 0, 0;
                if (uniform.selected_triangle != -1)
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_triangle * 3 + i].color
                                << uniform.selected_triangle_color[i];
                uniform.selected_triangle = -1;
                // disable animation mode
                if (!keyframes.empty()) {
                    transforms = keyframes[0];
                    keyframes.clear();
                }
            } else if (key == 'm') {
                mode = ANIMATION;
                // disable insertion mode
                int num = 0;
                if (insertion_step == TWO) num = 2;
                else if (insertion_step == THREE) num = 6;
                while (num-- > 0)
                    vertices_lines.erase(vertices_lines.end() - 1);
                insertion_step = ONE;
                // disable translation mode
                position_origin << 0, 0;
                if (uniform.selected_triangle != -1)
                    for (unsigned i = 0; i < 3; i++)
                        vertices_triangles[uniform.selected_triangle * 3 + i].color
                                << uniform.selected_triangle_color[i];
                uniform.selected_triangle = -1;
                // disable color mode
                uniform.selected_vertex = -1;
                // create a new frame
                keyframes.push_back(transforms);
            } else if (key == 'n') {
                // clear the frames
                if (mode == ANIMATION) {
                    transforms = keyframes[0];
                    keyframes.clear();
                    keyframes.push_back(transforms);
                }
            } else if (key == 'z') {
                // Linear Interpolation
                if (mode == ANIMATION && keyframes.size() > 1) {
                    for (int i = 1; i < keyframes.size(); i++) {
                        float t = 0;
                        while (t < 1 + 0.00001) {
                            for (int j = 0; j < transforms.size(); j++)
                                transforms[j] = (1 - t) * keyframes[i - 1][j] + t * keyframes[i][j];
                            t += 0.1;
                            viewer.redraw_next = true;
                            viewer.update();
                        }
                    }
                    for (int j = 0; j < transforms.size(); j++)
                        transforms[j] = keyframes[keyframes.size() - 1][j];
                    viewer.redraw_next = true;
                    viewer.update();
                }
            } else if (key == 'x') {
                // Bézier curve
                if (mode == ANIMATION && keyframes.size() > 1) {
                    float ins = 1 / (float(keyframes.size() - 1) * 10);
                    float t = 0;
                    while (t < 1 + 0.00001) {
                        for (int j = 0; j < transforms.size(); j++) {
                            transforms[j] = Eigen::Matrix4f::Zero();
                            int n = (int) keyframes.size() - 1;
                            for (int i = 0; i < keyframes.size(); i++)
                                transforms[j] += keyframes[i][j] * combination(n, i) * pow(t, i) * pow(1 - t, n - i);
                        }
                        t += ins;
                        viewer.redraw_next = true;
                        viewer.update();
                    }
                }
            } else if (key == 'h' || key == 'j') {
                if (mode == ROTATION_SCALE && uniform.selected_triangle != -1) {
                    Eigen::Vector4f barycenter(0, 0, 0, 0);
                    for (unsigned i = 0; i < 3; i++)
                        barycenter += transforms[uniform.selected_triangle] *
                                      vertices_triangles[uniform.selected_triangle * 3 + i].position;
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
                    transforms[uniform.selected_triangle] =
                            moveBack * Rotate * moveToOrigin * transforms[uniform.selected_triangle];
                }
            } else if (key == 'k' || key == 'l') {
                if (mode == ROTATION_SCALE && uniform.selected_triangle != -1) {
                    Eigen::Vector4f barycenter(0, 0, 0, 0);
                    for (unsigned i = 0; i < 3; i++)
                        barycenter += transforms[uniform.selected_triangle] *
                                      vertices_triangles[uniform.selected_triangle * 3 + i].position;
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
                    transforms[uniform.selected_triangle] =
                            moveBack * Scale * moveToOrigin * transforms[uniform.selected_triangle];
                }
            } else if (key >= '1' && key <= '9') {
                if (uniform.selected_vertex != -1)
                    vertices_triangles[uniform.selected_vertex].color = uniform.colors[key - '1'];
            } else if (key == '=') {
                Eigen::Matrix4f Scale;
                Scale << 1.2, 0, 0, 0,
                        0, 1.2, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1;
                uniform.camera_transform = Scale * uniform.camera_transform;
            } else if (key == '-') {
                Eigen::Matrix4f Scale;
                Scale << 0.8, 0, 0, 0,
                        0, 0.8, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1;
                uniform.camera_transform = Scale * uniform.camera_transform;
            } else if (key == 'w') {
                Eigen::Matrix4f Translate;
                Translate << 1, 0, 0, 0,
                        0, 1, 0, -0.2,
                        0, 0, 1, 0,
                        0, 0, 0, 1;
                uniform.camera_transform = Translate * uniform.camera_transform;
            } else if (key == 'a') {
                Eigen::Matrix4f Translate;
                Translate << 1, 0, 0, 0.2,
                        0, 1, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1;
                uniform.camera_transform = Translate * uniform.camera_transform;
            } else if (key == 's') {
                Eigen::Matrix4f Translate;
                Translate << 1, 0, 0, 0,
                        0, 1, 0, 0.2,
                        0, 0, 1, 0,
                        0, 0, 0, 1;
                uniform.camera_transform = Translate * uniform.camera_transform;
            } else if (key == 'd') {
                Eigen::Matrix4f Translate;
                Translate << 1, 0, 0, -0.2,
                        0, 1, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1;
                uniform.camera_transform = Translate * uniform.camera_transform;
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