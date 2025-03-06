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
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            std::vector<int> t;
            int itrash, idx, vtIdx;
            iss >> trash;
            while (iss >> idx >> trash >> vtIdx >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                vtIdx --;
                f.push_back(idx);
                t.push_back(vtIdx);
            }
            for (const auto& vt : t) {
                std::cout << vt << std::endl;
            }
            faces_.push_back(f);
            texIndices_.push_back(t);
        } else if(!line.compare(0, 3, "vt ")) {
            Vec2f texture;
            char trash;
            iss >> trash >> trash;
            for (int i=0; i < 2; i++) iss >> texture.raw[i];
            // std::cout << texture << std::endl;

            if (texture.raw[1] == 0.) throw std::runtime_error("wrong read texture");
            tex_coords_.push_back(texture);
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

Vec2f Model::texture(int i) {
    return tex_coords_[i];
}

