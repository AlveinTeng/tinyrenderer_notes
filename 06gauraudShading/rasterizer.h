#ifndef RASTERIZER_H
#define RASTERIZER_H

#include "tgaimage.h"
#include "geometry.h"

class Rasterizer {
public:
    Rasterizer(int width, int height);

    // Transformation matrices
    Matrix viewport(int x, int y, int w, int h);
    void projection(float coeff = 0.f); // coeff = -1/c
    void lookat(Vec3f eye, Vec3f center, Vec3f up);

    // Triangle rendering
    void triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer);
    void renderModelPerspective(Model *model, TGAImage &image, const TGAImage &texture);

private:
    int width, height, depth;
    Matrix ModelView;
    Matrix Viewport;
    Matrix Projection;

    Vec3f camera;
    Vec3f center;
};

struct IShader {
    virtual ~IShader() {}
    virtual Vec3i vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

#endif // RASTERIZER_H
