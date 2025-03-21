#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
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

Vec3f light_dir(1, 1, 1);
Vec3f camera(0, 0, 10);
Vec3f center(0,0,0);
Vec3f up(0,1,0);

template<size_t LEN, size_t DIM, typename T>
vec<LEN, T> embed(const vec<DIM, T> &v, T fill = 1) {
    vec<LEN, T> ret;
    for (size_t i = 0; i < LEN; i++) {
        ret[i] = (i < DIM ? v[i] : fill);
    }
    return ret;
}
struct GouraudShader : public IShader {
    Vec3f varing_intensity;

    virtual Vec4f vertex(int iface, int nthvert) {
        varing_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir);
        Vec3f v = model->vert(iface, nthvert);
        Vec4f gl_Vertex = embed<4>(v);
        Rasterizer::lookat(camera, center, up, ModelView);
        return Rasterizer::viewport(0, 0, width, height,depth) * Rasterizer::projection(5.f, 100.f, 90.f, width, height) * ModelView * gl_Vertex;
    }
    
};

int main(int argc, char** argv) {
    model = (argc > 1) ? new Model(argv[1]) : new Model("../obj/african_head/african_head.obj");
    TGAImage texture;
    if (!texture.read_tga_file("../obj/african_head/african_head_diffuse.tga")) {
        std::cerr << "Failed to load texture!" << std::endl;
        return 1;
    }

    TGAImage image(width, height, TGAImage::RGB);
    Rasterizer rasterizer = Rasterizer(width, height, camera, center, depth, model);
    rasterizer.renderModelPerspective(model, image, texture, depth, width, height);
    return 0;
}