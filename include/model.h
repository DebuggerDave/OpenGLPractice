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
        Model(std::string path);
        void Draw(Shader &shader);

    private:
        std::vector<Mesh> meshes;
        std::string directory;
        std::unordered_map<std::string, Texture> path_texture_map;

        void loadModel(std::string path);
        void processNode(const aiNode *node, const aiScene *scene);
        void processNodeRecursion(const aiNode *node, const aiScene *scene, unsigned int& mesh_num);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType ai_type, TexType type);
		bool TextureFromFile(std::string filename, unsigned int& tex_id);
};

#endif