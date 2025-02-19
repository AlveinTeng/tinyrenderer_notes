#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;


void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	for(float t = 0.; t < 1; t += .01) {
		int x = x0 + (x1 - x0) * t;
		int y = y0 + (y1 - y0) * t;
		image.set(x, y, color);
	}
}

void line2(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	for(int x = x0; x <= x1; x++) {
		float t = (x-x0) / float(x1 -x0);
		int y = y0*(1.-t) + y1*t;
		image.set(x,y,color);
	}
}

void line3(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
    bool steep = false; 
    if (std::abs(x0-x1)<std::abs(y0-y1)) { // if the line is steep, we transpose the image 
        std::swap(x0, y0); 
        std::swap(x1, y1); 
        steep = true; 
    } 
    if (x0>x1) { // make it left−to−right 
        std::swap(x0, x1); 
        std::swap(y0, y1); 
    } 
    for (int x=x0; x<=x1; x++) { 
        float t = (x-x0)/(float)(x1-x0); 
        int y = y0*(1.-t) + y1*t; 
        if (steep) { 
            image.set(y, x, color); // if transposed, de−transpose 
        } else { 
            image.set(x, y, color); 
        } 
    } 
}

void line4(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
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
    int dx = x1-x0; 
    int dy = y1-y0; 
    float derror = std::abs(dy/float(dx)); 
    float error = 0; 
    int y = y0; 
    for (int x=x0; x<=x1; x++) { 
        if (steep) { 
            image.set(y, x, color); 
        } else { 
            image.set(x, y, color); 
        } 
        error += derror; 
        if (error>.5) { 
            y += (y1>y0?1:-1); 
            error -= 1.; 
        } 
    } 
} 

void line5(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
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
    int dx = x1-x0; 
    int dy = y1-y0; 
    int derror2 = std::abs(dy)*2; 
    int error2 = 0; 
    int y = y0; 
    for (int x=x0; x<=x1; x++) { 
        if (steep) { 
            image.set(y, x, color); 
        } else { 
            image.set(x, y, color); 
        } 
        error2 += derror2; 
        if (error2 > dx) { 
            y += (y1>y0?1:-1); 
            error2 -= dx*2; 
        } 
    } 
}

std::pair<Vec2i, Vec2i> findBoundingBox(TGAImage& image, Vec2i* pts){
    Vec2i minBBox(image.get_width() - 1, image.get_height() - 1);
    Vec2i maxBBox(0, 0);
    Vec2i boundary(image.get_width() - 1, image.get_height() - 1);

    for(int i = 0; i < 3; i++){
        minBBox.x = std::max(0, std::min(minBBox.x, pts[i].x));
        minBBox.y = std::max(0, std::min(minBBox.y, pts[i].y));

        maxBBox.x = std::min(boundary.x, std::max(maxBBox.x, pts[i].x));
        maxBBox.y = std::min(boundary.y, std::max(maxBBox.y, pts[i].y));
    }

    return std::make_pair(minBBox, maxBBox);
}

static Vec3f computeBarycentric2D(float x, float y, const Vec2i* v) {
    float c1 = (x * (v[1].y - v[2].y) + (v[2].x - v[1].x) * y + v[1].x * v[2].y - v[2].x * v[1].y) /
               (v[0].x * (v[1].y - v[2].y) + (v[2].x - v[1].x) * v[0].y + v[1].x * v[2].y - v[2].x * v[1].y);
    float c2 = (x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * y + v[2].x * v[0].y - v[0].x * v[2].y) /
               (v[1].x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * v[1].y + v[2].x * v[0].y - v[0].x * v[2].y);
    float c3 = (x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * y + v[0].x * v[1].y - v[1].x * v[0].y) /
               (v[2].x * (v[0].y - v[1].y) + (v[1].x - v[0].x) * v[2].y + v[0].x * v[1].y - v[1].x * v[0].y);

    return {c1, c2, c3};
}

static bool insideTriangle(int x, int y, const Vec2i* _v)
{   
    // auto [c1, c2, c3] = computeBarycentric2D(x, y, _v); // Call the compute function
    auto coordinates = computeBarycentric2D(x, y, _v);
    auto c1 = coordinates.x;
    auto c2 = coordinates.y;
    auto c3 = coordinates.z;

    const float epsilon = 1e-6;

    // Check if all barycentric coordinates are >= 0 and their sum is approximately 1
    if (c1 >= -epsilon && c2 >= -epsilon && c3 >= -epsilon && std::abs(c1 + c2 + c3 - 1.0f) <= epsilon)
        return true;

    return false;
}


void triangle(Vec2i *pts, TGAImage &image, TGAColor color) { 
    auto [minBBox, maxBBox] = findBoundingBox(image, pts);
    for(int x=minBBox.x; x < maxBBox.x; x++){
        for(int y = minBBox.y; y < maxBBox.y; y++) {
            if(insideTriangle(x, y, pts)){
                image.set(x, y, color);
            }
        }
    }
} 



void saveAndShow(TGAImage& image, const char* outputPath){

	image.write_tga_file(outputPath);

    // Try to open the output file with the default viewer based on the platform
    #ifdef _WIN32
        system(("start " + std::string(outputPath)).c_str());  // Windows
    #elif __APPLE__
        system(("open " + std::string(outputPath)).c_str());  // macOS
    #else
        system(("xdg-open " + std::string(outputPath)).c_str());  // Linux
    #endif

}

int main(int argc, char** argv) {
    Vec3f light_dir(0,0,-1);
	if (2==argc) {
        model = new Model(argv[1]);
    } else {
		std::cout << "load the predefined model" << "\n";
        model = new Model("../obj/african_head.obj");
    }
	if(model == NULL) {
		throw std::runtime_error("Loading the model failed");
	}
	const char* outputPath = "../output.tga";

    TGAImage image(width, height, TGAImage::RGB);
    for (int i=0; i<model->nfaces(); i++) { 
    std::vector<int> face = model->face(i); 
    Vec2i screen_coords[3]; 
    Vec3f world_coords[3]; 
    for (int j=0; j<3; j++) { 
        Vec3f v = model->vert(face[j]); 
        screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.); 
        world_coords[j]  = v; 
    } 
    Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]); 
    n.normalize(); 
    float intensity = n*light_dir; 
    if (intensity>0) { 
        triangle(screen_coords, image, TGAColor(intensity*255, intensity*255, intensity*255, 255)); 
    } 
}
    image.flip_vertically();

    saveAndShow(image, outputPath);

    // TGAImage frame(200, 200, TGAImage::RGB); 
    // Vec2i pts[3] = {Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160)}; 
    // triangle(pts, frame, TGAColor(255, 0,   0,   255)); 
    // frame.flip_vertically(); // to place the origin in the bottom left corner of the image 

    // // frame.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    // // image.write_tga_file("output.tga");
	// saveAndShow(frame, outputPath);
    delete model;
    return 0;
	
}