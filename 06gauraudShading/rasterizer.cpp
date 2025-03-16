#include "rasterizer.h"
#include "model.h"

Matrix Rasterizer::viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][0] = w / 2.f;
    m[0][3] = x + w / 2.f;

    m[1][1] = h / 2.f; // 反转Y轴
    m[1][3] = y + h / 2.f;

    m[2][2] = depth / 2.f; // 映射Z到[0, depth]
    m[2][3] = depth / 2.f;

    return m;
}

void Rasterizer::lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye-center).normalize();
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

void Rasterizer::renderModelPerspective(Model *model, TGAImage &image, const TGAImage &texture) {
    float *zbuffer = new float[width * height];
    std::fill_n(zbuffer, width * height, std::numeric_limits<float>::max());

    // Matrix ModelView = Matrix::identity(4);
    lookat(camera, center, Vec3f(0,-1,0));
    Matrix viewportMat = viewport(0, 0, width, height);
    ModelView[2][3] = -2.f; // 调整模型位置
    Matrix proj = projection(5.f, 100.f, 90.f);

    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        std::vector<int> texIndices = model->texIndices(i);
        VertexData vdata[3];

        for (int j = 0; j < 3; j++) {
            Vec3f worldPos = model->vert(face[j]);
            Vec2f uv = model->texture(texIndices[j]);

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

        triangleWithTexPerspectiveCorrect(vdata, zbuffer, image, texture);
    }

    image.flip_vertically();
    image.write_tga_file("../output.tga");
    delete[] zbuffer;
}