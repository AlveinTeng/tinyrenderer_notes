// main.cpp
#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "rasterizer.h"
// #include "matrix.h" // Include the header file that defines the Matrix type

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

Model* model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;

Matrix ModelView;
Matrix Projection;
Matrix Viewport;

Vec3f light_dir = Vec3f(1.0f, 1.0f, 1.0f).normalize();
Vec3f camera(0, 0, 2);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

struct GouraudShader : public IShader {
    Matrix varying_tri;  // 三角形坐标
    Vec3f varying_ndc;   // 透视校正参数 (1/w)
    Vec3f varying_int;   // 强度值

    virtual Vec4f vertex(int iface, int nthvert) {
        Vec3f v = model->vert(iface, nthvert);
        Vec4f gl_Vertex = embed<4>(v);
        
        // 法线变换
        Vec3f n = proj<3, 4>(ModelView.inverse_transpose() * embed<4>(model->normal(iface, nthvert), 0.f)).normalize();
        // Vec3f n = model->normal(iface, nthvert);
        light_dir = proj<3, 4>(ModelView * embed<4>(light_dir, 0.f)).normalize();
        
        // 强度计算
        varying_int[nthvert] = std::max(0.0f, n * light_dir);
        
        // 坐标变换
        gl_Vertex = Viewport * (Projection * ModelView * gl_Vertex);
        varying_tri.set_col(nthvert, gl_Vertex);
        varying_ndc[nthvert] = 1.0f / gl_Vertex[3];
        
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        // 透视校正插值
        float w = 1.0f / (bar.x * varying_ndc.x + bar.y * varying_ndc.y + bar.z * varying_ndc.z);
        // float w = 1.0f;
        float intensity = (bar.x * varying_int.x * varying_ndc.x +
                          bar.y * varying_int.y * varying_ndc.y +
                          bar.z * varying_int.z * varying_ndc.z) * w;

        // 深度计算
        // Vec4f frag_pos = varying_tri * bar;
        // float z = frag_pos[2] / frag_pos[3];
        
        color = TGAColor(255, 255, 255) * intensity;
        // return (intensity < 0.0f) || (z < 0.0f) || (z > 1.0f);
        return false;
    }
};




int main(int argc, char** argv) {
    model = new Model(argc > 1 ? argv[1] : "../obj/african_head/african_head.obj");
    
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    // 初始化矩阵
    Rasterizer rasterizer = Rasterizer(width, height, camera, center, depth, model);
    Rasterizer::lookat(camera, center, up, ModelView);
    Projection = rasterizer.projection((camera - center).norm());
    Viewport = Rasterizer::viewport(0, 0, width, height, depth);

    GouraudShader shader;
    for (int i = 0; i < model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        rasterizer.triangle(screen_coords, shader, image, zbuffer);
    }

    // image.flip_vertically();
    // zbuffer.flip_vertically();
    image.write_tga_file("../output.tga");
    zbuffer.write_tga_file("../zbuffer.tga");
    delete model;
    return 0;
}