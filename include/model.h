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
        void Draw(const Shader& shader) const;

    private:
        // all loaded meshed from the 3d model
        std::vector<Mesh> meshes;
        // directory of loaded model
        std::string directory;
        // map of loaded textures
        std::unordered_map<std::string, Mesh::Texture> path_texture_map;

        // load 3d object from file path
        void loadModel(const std::string& path);
        // begin recursive processing of scene node
        void processNode(const aiNode* node, const aiScene* scene);
        // recursively process scene nodes
        void processNodeRecursion(const aiNode* node, const aiScene* scene, unsigned int& mesh_num);
        // extract vertices from mesh
        std::vector<Mesh::Vertex> processVertices(const aiMesh* mesh) const;
        // extract indices from mesh
        std::vector<unsigned int> processIndices(const aiMesh* mesh) const;
        // extract textures from mesh
        std::vector<Mesh::Texture> processTextures(const aiMesh* mesh, const aiScene* scene);
        // extract all data from mesh
        Mesh processMesh(const aiMesh* mesh, const aiScene* scene);
        // load textures from material into memory
        std::vector<Mesh::Texture> loadMaterialTextures(const aiMaterial* mat, const aiTextureType ai_type, const Mesh::TexType type);
        // create texture from image data
        void createTex(unsigned int& tex_id, const unsigned char* data, const int width, const int height, const int num_components);
        // create texture from file path
		bool texFromFile(const std::string& filename, unsigned int& tex_id);
};

#endif