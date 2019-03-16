#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>

static char* s_rootPath;

struct float2
{
	float x, y;
};

struct float3
{
	float x, y, z;
};

struct uint2
{
	uint32_t x, y;
};

struct uint3
{
	uint32_t x, y, z;
};

struct uintfloat
{
	uint32_t x;
	float y;
};

enum TextureType
{
	Albedo = 0, 
	Normal, 
	Metallic, 
	Roughness, 
	//AO, 
	Count
};

struct Texture
{
	uint8_t* data;
	std::size_t size;
	std::string path;
	std::string name;
};

struct Material
{
	std::string name;
	Texture textures[TextureType::Count];
};

struct Mesh
{
	std::string name;
	std::vector<float3> vertices;
	std::vector<float3> normals;
	std::vector<float3> tangents;
	std::vector<float2> uvs;
	std::vector<uintfloat> weights;
	std::vector<uint3> indices;

	std::string materialName;
};

void ProcessMesh(const aiMesh* mesh, const aiScene *scene, std::vector<Mesh>& outMeshes)
{
	Mesh outMesh;
	outMesh.name = mesh->mName.C_Str();
	std::replace(outMesh.name.begin(), outMesh.name.end(), ':', '_');

	for (uint32_t i = 0; i < mesh->mNumVertices; i++)
	{
		float3 vertex;
		vertex.x = mesh->mVertices[i].x;
		vertex.y = mesh->mVertices[i].y;
		vertex.z = mesh->mVertices[i].z;
		outMesh.vertices.push_back(vertex);

		float3 normal;
		normal.x = mesh->mNormals[i].x;
		normal.y = mesh->mNormals[i].y;
		normal.z = mesh->mNormals[i].z;
		outMesh.normals.push_back(normal);

		if (mesh->mTangents)
		{
			float3 tangent;
			tangent.x = mesh->mTangents[i].x;
			tangent.y = mesh->mTangents[i].y;
			tangent.z = mesh->mTangents[i].z;
			outMesh.tangents.push_back(tangent);
		}

		if (mesh->mTextureCoords)
		{
			float2 uv;
			uv.x = mesh->mTextureCoords[0][i].x;
			uv.y = mesh->mTextureCoords[0][i].y;
			outMesh.uvs.push_back(uv);
		}

		if (mesh->mBones)
		{
			uintfloat weight;
			weight.x = mesh->mBones[0]->mWeights->mVertexId;
			weight.y = mesh->mBones[0]->mWeights->mWeight;
			outMesh.weights.push_back(weight);
		}
	}

	for (uint32_t i = 0; i < mesh->mNumFaces; i++)
	{
		uint3 index;
		index.x = mesh->mFaces[i].mIndices[0];
		index.y = mesh->mFaces[i].mIndices[1];
		index.z = mesh->mFaces[i].mIndices[2];
		outMesh.indices.push_back(index);
	}

	outMeshes.push_back(outMesh);
}

void ProcessNode(aiNode *node, const aiScene *scene, std::vector<Mesh>& meshes)
{
	// process all the node's meshes (if any)
	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh, scene, meshes);
	}
	// then do the same for each of its children
	for (uint32_t i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(node->mChildren[i], scene, meshes);
	}
}
void ProcessMaterial(const aiScene *scene, std::vector<Material>& materials)
{
	materials.reserve(scene->mNumMaterials);
	for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
	{
		const aiMaterial* material = scene->mMaterials[i];
		Material outMat;

		aiString name;
		material->Get(AI_MATKEY_NAME, name);
		outMat.name = name.C_Str();

		// PARAMETER
		// subsurface albedo/diffuse
		aiColor3D color(0.0f, 0.0f, 0.0f);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);

		// fresnel reflectance at normal/specular
		aiColor3D specular(0.0f, 0.0f, 0.0f);
		material->Get(AI_MATKEY_COLOR_SPECULAR, specular);

		// metalness

		// TEXTURE
		char tmp[256];
		aiTextureType aiTypes[] = { aiTextureType_DIFFUSE,  aiTextureType_HEIGHT, aiTextureType_AMBIENT, aiTextureType_SHININESS };
		for (uint32_t i = 0; i < TextureType::Count; ++i)
		{
			auto& tex = outMat.textures[i];
			aiString path;

			material->GetTexture(aiTypes[i], 0, &path);
			tex.path = path.C_Str();

			int x, y, n;
			sprintf_s(tmp, "%s%s", s_rootPath, path.C_Str());
			tex.data = stbi_load(tmp, &x, &y, &n, 0);
			tex.size = x * y * n;

			char filename[256];
			_splitpath_s(tmp, NULL, 0, NULL, 0, filename, 256, NULL, 0);
			tex.name = filename;
		}

		materials.push_back(std::move(outMat));
	}
}

void Export(const std::vector<Material>& materials, const char* path)
{
	std::fstream fout;
	uint32_t size = materials.size();

	for (uint32_t i = 0; i < size; ++i)
	{
		const Material& material = materials[i];

		fout.open(std::string(path) + "Materials\\" + material.name + ".mate", std::fstream::out);
		for (uint32_t j = 0; j < TextureType::Count; ++j)
		{
			fout << material.textures[j].path << "\n";
		}
		fout.close();

		for (uint32_t j = 0; j < TextureType::Count; ++j)
		{
			const auto& tex = material.textures[j];
			if (!tex.name.empty()) 
			{
				fout.open(std::string(path) + "Textures\\" + tex.name + ".tex", std::fstream::out);
				fout.write(reinterpret_cast<char*>(tex.data), tex.size);
				fout.close();
			}
		}
	}

}

void Export(const std::vector<Mesh>& meshes, const char* path)
{
	uint32_t size = meshes.size();
	std::fstream fout;

	for (uint32_t i = 0; i < size; ++i)
	{
		const Mesh& mesh = meshes[i];
		std::string name = "Models\\" + mesh.name;
		uint32_t num = 0;

		// vertex
		fout.open(std::string(path) + name + ".vert", std::fstream::out);
		num = mesh.vertices.size();
		for (auto& vertex : mesh.vertices)
		{
			fout << num << std::endl;
			fout << vertex.x << " " << vertex.y << " " << vertex.z;
		}
		fout.close();

		// normal
		fout.open(std::string(path) + name + ".norm", std::fstream::out);
		num = mesh.normals.size();
		for (auto& normal : mesh.normals)
		{
			fout << num << std::endl;
			fout << normal.x << " " << normal.y << " " << normal.z;
		}
		fout.close();

		// texCoord
		fout.open(std::string(path) + name + ".texcoord", std::fstream::out);
		num = mesh.uvs.size();
		for (auto& uv : mesh.uvs)
		{
			fout << num << std::endl;
			fout << uv.x << " " << uv.y;
		}
		fout.close();

		// index
		fout.open(std::string(path) + name + ".index", std::fstream::out);
		num = mesh.indices.size();
		for (auto& index : mesh.indices)
		{
			fout << num << std::endl;
			fout << index.x << " " << index.y << " " << index.z;
		}
		fout.close();

		// index
		fout.open(std::string(path) + name + ".mate", std::fstream::out);
		fout << mesh.materialName;
		fout.close();
	}
}

void Export(const std::vector<Mesh>& meshes, const std::vector<Material>& materials, const char* path)
{
	std::fstream fout;
	std::string meshNames;
	std::string materialNames;

	uint32_t numMesh = meshes.size();
	for (uint32_t i = 0; i < numMesh; ++i)
	{
		meshNames.append(meshes[i].name + '\n');
	}

	uint32_t numMat = materials.size();
	for (uint32_t i = 0; i < numMat; ++i)
	{
		materialNames.append(materials[i].name + '\n');
	}

	fout.open(std::string(path) + "Scenes\\sponza.scene", std::fstream::out);
	fout << numMesh << std::endl << meshNames;
	fout << numMat << std::endl << materialNames;
	fout.close();

	//Export(meshes, path);
	Export(materials, path);
}

int main(int argc, char *argv[])
{
	s_rootPath = argv[1];
	const char* inputPath = argv[2];
	const char* outputPath = argv[3];

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(inputPath, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);

	std::vector<Mesh> meshes;
	std::vector<Material> materials;

	//ProcessNode(scene->mRootNode, scene, meshes);
	ProcessMaterial(scene, materials);

	Export(meshes, materials, outputPath);

	return 0;
}