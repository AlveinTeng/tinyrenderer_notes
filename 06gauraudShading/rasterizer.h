#ifndef RASTERIZER_H
#define RASTERIZER_H

#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
struct IShader {
    virtual ~IShader() {}
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};
class Rasterizer {
public:
    struct VertexData {
        Vec2f screenXY; // 视口变换后的x,y
        float ndcZ;     // NDC的z值（clip.z/clip.w）
        float oneOverW; // 1/clip.w
        Vec2f uvOverW;  // uv/clip.w
    };

    Rasterizer(int width, int height, Vec3f camera, Vec3f center, int depth, Model* model);

    // Transformation matrices
    static Matrix viewport(int x, int y, int w, int h, int depth);
    static Matrix projection(float near, float far, float fov, int weight, int height); // coeff = -1/c
    static void lookat(Vec3f eye, Vec3f center, Vec3f up, Matrix& ModelView);

    // Triangle rendering
    // void triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer);
    void renderModelPerspective(Model *model, TGAImage &image, const TGAImage &texture, int depth, int weight, int height);
    // void renderModelPerspective(Model *model, TGAImage &image, const TGAImage &texture);
    void triangleWithTexPerspectiveCorrect(const Rasterizer::VertexData v[3], float *zbuffer, TGAImage &image, const TGAImage &texture);

    void triangle(Vec4f* pts, IShader& shader, TGAImage &image, TGAImage& zbuffer);
    

private:
    int width, height, depth;
    Model* model;
    Matrix ModelView;
    Matrix Viewport;
    Matrix Projection;

    Vec3f camera;
    Vec3f center;

    Matrix v2m(Vec3f v);
    Vec3f m2v(Matrix m);
    Vec3f barycentric2D(const Vec2f &A, const Vec2f &B, const Vec2f &C, const Vec2f &P);

    
};



#endif // RASTERIZER_H
