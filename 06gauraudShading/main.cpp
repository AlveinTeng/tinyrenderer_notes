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

Vec3f camera(0, 0, 10);
Vec3f center(0,0,0);

int main(int argc, char** argv) {
    model = (argc > 1) ? new Model(argv[1]) : new Model("../obj/african_head/african_head.obj");
    TGAImage texture;
    if (!texture.read_tga_file("../obj/african_head/african_head_diffuse.tga")) {
        std::cerr << "Failed to load texture!" << std::endl;
        return 1;
    }

    TGAImage image(width, height, TGAImage::RGB);
    Rasterizer rasterizer = Rasterizer(width, height, camera, center, depth, model);
    rasterizer.renderModelPerspective(model, image, texture);
    return 0;
}