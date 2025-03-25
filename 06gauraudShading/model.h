#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int>> texIndices_;
	std::vector<Vec2f> tex_coords_;

	TGAImage normalmap_;
	TGAImage diffusemap_;
	TGAImage specularmap_;

	std::vector<Vec3f> norms_;
	std::vector<Vec2f> uv_;
	void load_texture(std::string filename, const char* suffix, TGAImage &img);

public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	std::vector<int> face(int idx);
	std::vector<int> texIndices(int idx);

	// std::vector<Vec3f> norms_;
	std::vector<std::vector<int>> normIndices_;

	Vec2f texture(int idx);
	// Vec2f normal(Vec2f vert);
	Vec3f normal(int iface, int nthvert);
	Vec3f normal(Vec2f uv);
	Vec3f vert(int iface, int nthvert);
	Vec2f uv(int iface, int nthvert);
	TGAColor diffuse(Vec2f uv);
	float specular(Vec2f uv);

};

#endif //__MODEL_H__
