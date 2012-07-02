#include "OBJLoader.h"

OBJLoader::OBJLoader()
{
}

bool OBJLoader::LoadFile(char* filename, std::vector<float3D>* outPositions)
{
	std::ifstream file;
	char buffer[1024];

	file.open(filename, std::ios_base::binary);
	
	if(!file.is_open())
		return false;

	while(file.getline(buffer, 1024))
	{
		char key[20];

		sscanf(buffer, "%s", key);
		if(strcmp(key, "v") == 0)
		{
			float3D currVertex;
			sscanf(buffer, "v %f %f %f", &currVertex.x, &currVertex.y, &currVertex.z);
			outPositions->push_back(currVertex);
		}
		//else if(strcmp(key, "vt") == 0)
		//{
		//	float2D texCoord;
		//	sscanf(buffer, "vt %f %f", &texCoord.u, &texCoord.v);
		//	outTexturePos->push_back(texCoord);
		//}
		//else if(strcmp(key, "vn") == 0)
		//{
		//	float3D currNormal;
		//	sscanf(buffer, "vn %f %f %f", &currNormal.x, &currNormal.y, &currNormal.z);
		//	outNormals->push_back(currNormal);
		//}
	}

	return true;
}