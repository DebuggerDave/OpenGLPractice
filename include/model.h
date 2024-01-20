#ifndef MODEL_H
#define MODEL_H

#include "shader.h"
#include "mesh.h"

#include <string>
#include <vector>
#include <unordered_map>

#include <assimp/scene.h>

class Model 
{
    public:
        Model(const std::string& path);
        void Draw(const Shader& shader);

    private:
        std::vector<Mesh> meshes;
        std::string directory;
        std::unordered_map<std::string, Mesh::Texture> path_texture_map;

        void loadModel(const std::string& path);
        void processNode(const aiNode* node, const aiScene* scene);
        void processNodeRecursion(const aiNode* node, const aiScene* scene, unsigned int& mesh_num);
        std::vector<Mesh::Vertex> processVertices(const aiMesh* mesh);
        std::vector<unsigned int> processIndices(const aiMesh* mesh);
        std::vector<Mesh::Texture> processTextures(const aiMesh* mesh, const aiScene* scene);
        Mesh processMesh(const aiMesh* mesh, const aiScene* scene);
        std::vector<Mesh::Texture> loadMaterialTextures(aiMaterial* mat, const aiTextureType ai_type, const Mesh::TexType type);
        void createTex(unsigned int& tex_id, const unsigned char* data, const int width, const int height, const int num_components);
		bool texFromFile(const std::string& filename, unsigned int& tex_id);
};

#endif