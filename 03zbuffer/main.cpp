#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x=x0; x<=x1; x++) {
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.-t) + y1*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f v0 = B - A, v1 = C - A, v2 = P - A;
    float d00 = v0.x * v1.y - v0.y * v1.x;
    float d01 = (v2.x * v1.y - v2.y * v1.x) / d00;
    float d02 = (v0.x * v2.y - v0.y * v2.x) / d00;
    float u = 1.0f - d01 - d02;
    float v = d01;
    float w = d02;
    return Vec3f(u, v, w);
}

void triangleWithTex(Vec3f *pts, Vec2f* tex, float *zbuffer, TGAImage & image, TGAImage &texture) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    Vec2f PointTex;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f bc_screen  = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            P.z = 0;
            PointTex = Vec2f(0, 0);
            for (int i=0; i<3; i++){
                P.z += pts[i][2]*bc_screen[i];
                PointTex.u += tex[i].u * bc_screen[i];
                PointTex.v += tex[i].v * bc_screen[i];
            } 
            
            if (zbuffer[int(P.x+P.y*width)]<P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;
                // PointTex.u = std::min(1.f, std::max(0.f, PointTex.u));
                // PointTex.v = std::min(1.f, std::max(0.f, PointTex.v));
                PointTex.u = std::min(1.f, std::max(0.f, PointTex.u));
                PointTex.v = std::min(1.f, std::max(0.f, PointTex.v));
                int texX = static_cast<int>(PointTex.u * (texture.get_width() - 1));
                int texY = static_cast<int>((PointTex.v) * (texture.get_height() - 1)); // 反转V分量
                TGAColor color = texture.get(texX, texY);
                // TGAColor color = texture.get(PointTex.u * texture.get_width(),PointTex.v * texture.get_height());

                image.set(P.x, P.y, color);
            }
        }
    }

}

void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);

    int minX = std::max(0, static_cast<int>(std::floor(bboxmin.x)));
    int maxX = std::min(image.get_width()-1, static_cast<int>(std::ceil(bboxmax.x)));
    int minY = std::max(0, static_cast<int>(std::floor(bboxmin.y)));
    int maxY = std::min(image.get_height()-1, static_cast<int>(std::ceil(bboxmax.y)));
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    // Vec3f P;
    Vec2f texture;
    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            Vec3f P(x,y,0);
            Vec3f bc_screen  = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            P.z = 0;
            for (int i=0; i<3; i++) P.z += pts[i][2]*bc_screen[i];
            if (zbuffer[int(P.x+P.y*width)]<P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}


int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        std::cout << "african_head" << std::endl;
        model = new Model("../obj/african_head.obj");
        if (model == nullptr){
            throw std::runtime_error("Failed to get model");
        }
    }

    std::cout << "The Model has " << model->nfaces() << " faces" << std::endl;

    float *zbuffer = new float[width*height];
    for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

    TGAImage textureImage;
    if(!textureImage.read_tga_file("../obj/african_head_diffuse.tga")) {
        throw std::runtime_error("Failed to load texture!");
    }


    TGAImage image(width, height, TGAImage::RGB);
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        std::vector<int> textIndices = model->texIndices(i);
        Vec3f pts[3];
        Vec2f tex[3];
        for (int j=0; j<3; j++){
            pts[j] = world2screen(model->vert(face[j]));
            tex[j] = model->texture(textIndices[j]);
        } 

        // triangle(pts, zbuffer, image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
        triangleWithTex(pts, tex, zbuffer, image, textureImage);
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("../output.tga");
    delete model;
    return 0;
}