#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include "component.h"
class Shader;
class World;

// assimp forward decl
class aiNode;
class aiScene;
class aiMesh;
#include "assimp/material.h"

#include <string>
#include <vector>
#include <unordered_map>

class Model 
{
    public:
        Model(const std::string& path, BlockId id = BlockId::Name::None) noexcept;
	    ~Model() = default;
	    Model(const Model& other) = delete;
	    Model(Model&& other) noexcept = default;
	    Model& operator=(const Model& other) = delete;
	    Model& operator=(Model&& other) = delete;

        // draw number of instances indicated by num, zero draws without instancing
        void draw(const Shader& shader, const unsigned int num = 0) const;
        // add a vertex attribute array of vec4s for instance rendering
        void setupInstancing(const World& world) const;

        const BlockId id;

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
        void createTex(unsigned int& tex_id, const unsigned char* data, const unsigned int width, const unsigned int height, const unsigned int num_component, const Mesh::TexType types);
        // create texture from file path
        bool texFromFile(const std::string& filename, unsigned int& tex_id, const Mesh::TexType type);
};

#endif