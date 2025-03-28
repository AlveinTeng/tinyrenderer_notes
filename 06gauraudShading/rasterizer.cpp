#include "rasterizer.h"
#include "model.h"
// #include <cassert>


Rasterizer::Rasterizer(int width, int height, Vec3f camera, Vec3f center, int depth, Model* model) {
    this->width = width; 
    this->height = height; 
    this->camera = camera; 
    this->center = center; 
    this->depth = depth; 
    this->model = model; 
}

Matrix Rasterizer::viewport(int x, int y, int w, int h, int depth) {
    Matrix m = Matrix::identity(4);
    m[0][0] = w / 2.f;
    m[0][3] = x + w / 2.f;

    m[1][1] = h / 2.f; // 反转Y轴
    m[1][3] = y + h / 2.f;

    m[2][2] = depth / 2.f; // 映射Z到[0, depth]
    m[2][3] = depth / 2.f;

    return m;
}

void Rasterizer::lookat(Vec3f eye, Vec3f center, Vec3f up, Matrix& ModelView) {
    Vec3f z = (eye - center).normalize();
    Vec3f x =up.cross(z).normalize();
    Vec3f y = z.cross(x).normalize();
    Matrix Minv = Matrix::identity(4);
    Matrix Tr   = Matrix::identity(4);
    for (int i=0; i<3; i++) {
        Minv[0][i] = x[i];
        Minv[1][i] = y[i];
        Minv[2][i] = z[i];
        Tr[i][3] = -eye[i];
    }
    ModelView = Minv*Tr;
    
}
Matrix Rasterizer::projection(float near, float far, float fov, int width, int height) {
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

Matrix Rasterizer::v2m(Vec3f v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

Vec3f Rasterizer::m2v(Matrix m) {
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}


Vec3f Rasterizer::barycentric2D(const Vec2f &A, const Vec2f &B, const Vec2f &C, const Vec2f &P) {
    float denom = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
    if (std::fabs(denom) < 1e-6) return Vec3f(-1, 1, 1);
    float alpha = ((B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y)) / denom;
    float beta = ((C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y)) / denom;
    float gamma = 1.f - alpha - beta;
    return Vec3f(alpha, beta, gamma);
}
void Rasterizer::triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j]/pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j]/pts[i][3]);
        }
    }
    Vec2i P;
    TGAColor color;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec2f Pf(P[0], P[1]);
            Vec3f c = barycentric2D(proj<2, 4>(pts[0]*(1.f / pts[0][3])), proj<2, 4>(pts[1]*( 1.f /pts[1][3])), proj<2, 4>(pts[2] *(1.f/pts[2][3])), Pf);
            float z = pts[0][2]*c.x + pts[1][2]*c.y + pts[2][2]*c.z;
            float w = pts[0][3]*c.x + pts[1][3]*c.y + pts[2][3]*c.z;
            int frag_depth = std::max(0, std::min(255, int(z/w+.5)));
            if (c.x<0 || c.y<0 || c.z<0 || zbuffer.get(P.x, P.y)[0]>frag_depth) continue;
            bool discard = shader.fragment(c, color);
            if (!discard) {
                zbuffer.set(P.x, P.y, TGAColor(frag_depth));
                image.set(P.x, P.y, color);
            }
        }
    }
}
void Rasterizer::triangleWithTexPerspectiveCorrect(const Rasterizer::VertexData v[3], float *zbuffer, TGAImage &image, const TGAImage &texture) {
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

void Rasterizer::renderModelPerspective(Model *model, TGAImage &image, const TGAImage &texture , int depth, int width, int height) {
    float *zbuffer = new float[width * height];
    std::fill_n(zbuffer, width * height, std::numeric_limits<float>::max());

    // Matrix ModelView = Matrix::identity(4);
    lookat(camera, center, Vec3f(0,-1,0), ModelView);
    Matrix viewportMat = viewport(0, 0, width, height, depth);
    ModelView[2][3] = -2.f; // 调整模型位置
    Matrix proj = projection(5.f, 100.f, 90.f, width, height);

    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        std::vector<int> texIndices = model->texIndices(i);
        VertexData vdata[3];

        for (int j = 0; j < 3; j++) {
            Vec3f worldPos = model->vert(face[j]);
            Vec2f uv = model->texture(texIndices[j]);
            uv.y = 1 - uv.y;

            Matrix clipCoord = proj * ModelView * v2m(worldPos);
            float w_clip = clipCoord[3][0];
            Vec3f ndc = Vec3f(clipCoord[0][0]/w_clip, clipCoord[1][0]/w_clip, clipCoord[2][0]/w_clip);

            // Vec2f screenXY = Vec2f((ndc.x + 1) * 0.5f * width, (ndc.y + 1) * 0.5f * height);
            // 视口变换后的x,y
            Vec3f ScreenCoords = m2v(viewportMat * v2m(ndc));
            Vec2f screenXY = Vec2f(ScreenCoords.x, ScreenCoords.y);
            
            vdata[j].screenXY = screenXY;
            vdata[j].ndcZ = ndc.z;
            vdata[j].oneOverW = 1.f / w_clip;
            vdata[j].uvOverW = uv * vdata[j].oneOverW;
        }

        Rasterizer::triangleWithTexPerspectiveCorrect(vdata, zbuffer, image, texture);
    }

    // image.flip_vertically();
    image.write_tga_file("../output.tga");
    delete[] zbuffer;
};

Matrix Rasterizer::projection(float coeff) {
    Matrix m = Matrix::identity(4);
    m[3][2] = coeff;
    return m;
};