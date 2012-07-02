#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <fstream>
#include <vector>

struct float2D
{
	float		u;
	float		v;
};

struct float3D
{
	float		x;
	float		y;
	float		z;
};

class OBJLoader
{
public:
	OBJLoader();
	bool LoadFile(char* filename, std::vector<float3D>* outPositions/*, 
		std::vector<float2D>* outTexturePos, std::vector<float3D>* outNormals*/);
};
#endif