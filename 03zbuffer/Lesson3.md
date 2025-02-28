# Z-Buffer and Texrture mapping

## Notes on Z-Buffer

### Why we need Z-Buffer?
We don't need to draw/render the object that being hidden from others, also we don't want the secene look unreal if the behind object
can still be shown along with the front ones.

### What is Z-Buffer
Basically it's just a table/hashmap that records the the frontmost(nearest to the camera) distance and its color for each pixel.

### How can we apply Z-buffer
During rasterization, when we try to set up the color for each pixel, we also record its distance to the camera and then update the Z-Buffer, when a "color" is farther from the camera than the current one, we just skip it.

## Notes on Texture mapping

### Why we need texture?
To make the secene looks with more details with the some predefined color(可以讲纹理理解为图库，纹理映射的过程就是去取颜色的过程)

### What is Texture mapping
Basically it's just the process of getting color from the texture and set it to a specific pixel(Create a one to one map from the pixel to the texture)
### How did we implement that?
- Update the model.h and model.cpp so that we can read the texture coordinates for each vertex.
- Load the texture
- Update the rasterization process, when set the color for a specific pixel, instead of setting it up directly, get the colot from the texture.