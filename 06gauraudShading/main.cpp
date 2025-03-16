#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

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

Vec3f camera(0, 0, 10);
Vec3f center(0,0,0);

Matrix v2m(Vec3f v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

Vec3f m2v(Matrix m) {
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}


Vec3f barycentric2D(const Vec2f &A, const Vec2f &B, const Vec2f &C, const Vec2f &P) {
    float denom = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
    if (std::fabs(denom) < 1e-6) return Vec3f(-1, 1, 1);
    float alpha = ((B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y)) / denom;
    float beta = ((C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y)) / denom;
    float gamma = 1.f - alpha - beta;
    return Vec3f(alpha, beta, gamma);
}

Matrix projection(float near, float far, float fov) {
    float aspect = (float)width / height;
    float tanHalfFov = tan(fov * 0.5f * M_PI / 180.f);
    Matrix proj = Matrix::identity(4);
    proj[0][0] = 1.f / (aspect * tanHalfFov);
    proj[1][1] = 1.f / tanHalfFov;
    proj[2][2] = -(far + near) / (far - near);
    proj[2][3] = -2.f * far * near / (far - near);
    proj[3][2] = -1.f;
    proj[3][3] = 0.f;
    return proj;
}

struct VertexData {
    Vec2f screenXY; // 视口变换后的x,y
    float ndcZ;     // NDC的z值（clip.z/clip.w）
    float oneOverW; // 1/clip.w
    Vec2f uvOverW;  // uv/clip.w
};

void triangleWithTexPerspectiveCorrect(const VertexData v[3], float *zbuffer, TGAImage &image, const TGAImage &texture) {
    Vec2f bboxmin(1e8, 1e8), bboxmax(-1e8, -1e8);
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        bboxmin.x = std::max(0.f, std::min(bboxmin.x, v[i].screenXY.x));
        bboxmin.y = std::max(0.f, std::min(bboxmin.y, v[i].screenXY.y));
        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, v[i].screenXY.x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, v[i].screenXY.y));
    }

    Vec2i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc = barycentric2D(v[0].screenXY, v[1].screenXY, v[2].screenXY, Vec2f(P.x, P.y));
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;

            // 插值参数
            float oneOverW = v[0].oneOverW * bc.x + v[1].oneOverW * bc.y + v[2].oneOverW * bc.z;
            if (oneOverW < 1e-8) continue;
            float w = 1.f / oneOverW;

            // 计算屏幕空间z：将NDC的z [-1,1]映射到[0, depth]
            float z_ndc = (v[0].ndcZ * bc.x + v[1].ndcZ * bc.y + v[2].ndcZ * bc.z);
            float z_screen = (z_ndc + 1.f) * 0.5f * depth;

            // 深度测试
            int idx = P.x + P.y * width;
            if (z_screen < zbuffer[idx]) {
                zbuffer[idx] = z_screen;

                // 计算uv
                Vec2f uv = (v[0].uvOverW * bc.x + v[1].uvOverW * bc.y + v[2].uvOverW * bc.z) * w;
                uv.x = std::max(0.f, std::min(1.f, uv.x));
                uv.y = std::max(0.f, std::min(1.f, uv.y));

                int texX = uv.x * (texture.get_width() - 1);
                int texY = uv.y * (texture.get_height() - 1); // 这里纹理不反向
                image.set(P.x, P.y, texture.get(texX, texY));
            }
        }
    }
}



int main(int argc, char** argv) {
    model = (argc > 1) ? new Model(argv[1]) : new Model("../obj/african_head.obj");
    TGAImage texture;
    if (!texture.read_tga_file("../obj/african_head_diffuse.tga")) {
        std::cerr << "Failed to load texture!" << std::endl;
        return 1;
    }

    TGAImage image(width, height, TGAImage::RGB);
    renderModelPerspective(model, image, texture);
    return 0;
}