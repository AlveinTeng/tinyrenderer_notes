#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

// typedef vec<3, float> Vec3f;
// typedef vec<4, float> Vec4f;
#include "rasterizer.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const TGAColor yellow = TGAColor(255, 255, 0, 255);

Model *model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;

Matrix ModelView;

Vec3f light_dir = Vec3f(1.0f, 1.0f, 1.0f);
Vec3f camera(0, 0, 10);
Vec3f center(0,0,0);
Vec3f up(0,1,0);


struct GouraudShader : public IShader {
    Vec3f varing_intensity;
    // Vec3f varing_intensity[3];
    // float varing_intensity[3];


    virtual Vec4f vertex(int iface, int nthvert) {
        // varing_intensity[nthvert] = std::max(0.f, (model->normal(iface, nthvert) * light_dir));
        assert(model != nullptr);
        assert(iface < model->nfaces());
        assert(nthvert < 3);

        Vec3f n = model->normal(iface, nthvert);
        float intensity = std::max(0.f, n * light_dir);
        varing_intensity[nthvert] = intensity;
        Vec3f v = model->vert(iface, nthvert);
        Vec4f gl_Vertex = embed<4>(v);
        Rasterizer::lookat(camera, center, up, ModelView);
        Matrix transfer = Rasterizer::viewport(0, 0, width, height,depth) * Rasterizer::projection(5.f, 100.f, 90.f, width, height) * ModelView; 
        return transfer * gl_Vertex;
        // return Vec4f(0 ,0, 0, 0);
        // return (Rasterizer::viewport(0, 0, width, height,depth) * Rasterizer::projection(5.f, 100.f, 90.f, width, height) * ModelView) * gl_Vertex;

    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varing_intensity * bar;
        // float intensity = varing_intensity[0] * bar.x +
        //           varing_intensity[1] * bar.y +
        //           varing_intensity[2] * bar.z;

        color = TGAColor(255, 255, 255) * intensity;
        return false;
    }
    
};

int main(int argc, char** argv) {
    model = (argc > 1) ? new Model(argv[1]) : new Model("../obj/african_head/african_head.obj");
    if(model == nullptr) {
        throw std::runtime_error("failed to load model");
    }
    TGAImage texture;
    if (!texture.read_tga_file("../obj/african_head/african_head_diffuse.tga")) {
        std::cerr << "Failed to load texture!" << std::endl;
        return 1;
    }

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    Rasterizer rasterizer = Rasterizer(width, height, camera, center, depth, model);
    light_dir.normalize();

    // GouraudShader shader;
    for (int i=0; i<model->nfaces(); i++) {
        GouraudShader shader;
        Vec4f screen_coords[3];
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }

        rasterizer.triangle(screen_coords, shader, image, zbuffer);
    }
    image.flip_vertically();
    zbuffer.flip_vertically();
    image.write_tga_file("../output.tga");
    zbuffer.write_tga_file("../zbuffer.tga");
    delete model;
    // rasterizer.renderModelPerspective(model, image, texture, depth, width, height);

    return 0;
}