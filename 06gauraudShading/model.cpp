#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            std::vector<int> t;
            std::vector<int> n;
            int idx, vtIdx, vnIdx;
            iss >> trash; // 'f'
            while (iss >> idx >> trash >> vtIdx >> trash >> vnIdx) {
                // Wavefront OBJ indices are 1-based
                idx   -= 1;
                vtIdx -= 1;
                vnIdx -= 1;
                f.push_back(idx);
                t.push_back(vtIdx);
                n.push_back(vnIdx);
            }
            faces_.push_back(f);
            texIndices_.push_back(t);
            normIndices_.push_back(n);

        } else if(!line.compare(0, 3, "vt ")) {
            Vec2f texture;
            char trash;
            iss >> trash >> trash;
            for (int i=0; i < 2; i++) iss >> texture[i];
            // std::cout << texture << std::endl;

            if (texture[1] == 0.) throw std::runtime_error("wrong read texture");
            tex_coords_.push_back(texture);
        } else if (!line.compare(0, 3, "vn ")) {
            // parse vertex normal
            iss >> trash >> trash; // read 'v' 'n'
            Vec3f n;
            for (int i = 0; i < 3; i++) iss >> n[i];
            norms_.push_back(n);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << " vt#" << tex_coords_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<int> Model::texIndices(int idx) {
    return texIndices_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}
Vec3f Model::vert(int iface, int nthvert) {
    return verts_[faces_[iface][nthvert]];
}

Vec2f Model::texture(int i) {
    return tex_coords_[i];
}

void Model::load_texture(std::string filename, const char* suffix, TGAImage& image) {
    std::string texfile(filename);
    size_t dot = texfile.find_first_of(".");
    if (dot != std::string::npos) {
        texfile = texfile.substr(0, dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (image.read_tga_file(texfile.c_str()) ? "ok" : "failed") <<std::endl;
        image.flip_vertically();
    }
}

Vec3f Model::normal(Vec2f uvf) {
    Vec2i uv(uvf[0] * normalmap_.get_width(), uvf[1] * normalmap_.get_height());
    TGAColor c = normalmap_.get(uv[0], uv[1]);
    Vec3f res;
    for (int i=0; i<3; i++) {
        res[2-i] = (float)c[i]/255.f*2.f - 1.f;
    }

    return res;
}

Vec2f Model::uv(int iface, int nthvert) {
    return uv_[faces_[iface][nthvert]];
}

float Model::specular(Vec2f uvf) {
    Vec2i uv(uvf[0] * specularmap_.get_width(), uvf[1] * specularmap_.get_height());
    return specularmap_.get(uv[0], uv[1])[0] /1.f;
}

Vec3f Model::normal(int iface, int nthvert) {
    int normal_index = normIndices_[iface][nthvert];
    // Make sure normal_index is in range
    return norms_[normal_index].normalize();
}



