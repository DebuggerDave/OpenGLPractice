#include "model.h"

#include <cmath>
#include <iostream>
#include <source_location>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"

#include "shader.h"

Model::Model(std::string path)
{
	loadModel(path);
}

void Model::Draw(Shader &shader)
{
    for(unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].Draw(shader);
	}
}

void Model::loadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);	
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << " in " << std::source_location::current().function_name() << "\n";
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(const aiNode *node, const aiScene *scene)
{
    // setup recursion
    meshes = std::vector<Mesh>(scene->mNumMeshes);
    unsigned int mesh_num = 0;
    processNodeRecursion(node, scene, mesh_num);
}

void Model::processNodeRecursion(const aiNode *node, const aiScene *scene, unsigned int& mesh_num)
{
    // recursion
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNodeRecursion(node->mChildren[i], scene, mesh_num);
    }

    // process meshes
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes[mesh_num++] = processMesh(mesh, scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    static const int expected_face_vertices = 3;
    std::vector<Vertex> vertices(mesh->mNumVertices);
    std::vector<unsigned int> indices(mesh->mNumFaces * expected_face_vertices);
    std::vector<Texture> textures;

	// positions, normals, texture coordinates
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vertices[i].Position = glm::vec3(
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		);
        if (mesh->HasNormals()) {
		    vertices[i].Normal = glm::vec3(
			    mesh->mNormals[i].x,
			    mesh->mNormals[i].y,
			    mesh->mNormals[i].z
		    );
        } else {
            vertices[i].Normal = glm::vec3(0.0f);
        }

		if (mesh->mTextureCoords[0]) {
    		vertices[i].TexCoords = glm::vec2(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			);
		}
		else {
			vertices[i].TexCoords = glm::vec2(0.0f);
		}
    }

    // indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
		if (mesh->mFaces[i].mNumIndices != expected_face_vertices) {
			std::cerr << "expected " << expected_face_vertices << " vertices in mesh face, but received " << mesh->mFaces[i].mNumIndices << " vertices in " << std::source_location::current().function_name() << "\n";
		}
    	for(unsigned int j = 0; j < expected_face_vertices; j++) {
        	indices[(i*expected_face_vertices) + j] = (mesh->mFaces[i].mIndices[j]);
		}
	}

    // materials
	if(mesh->mMaterialIndex >= 0)
	{
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuse_maps = loadMaterialTextures(material, 
											aiTextureType_DIFFUSE, TexType::Diffuse);
		textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
		std::vector<Texture> specular_maps = loadMaterialTextures(material, 
											aiTextureType_SPECULAR, TexType::Specular);
		textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
	}

    return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType ai_type, TexType type)
{
    std::vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(ai_type); i++)
    {
        aiString ai_path;
        bool tex_succ = (mat->GetTexture(ai_type, i, &ai_path) == aiReturn_SUCCESS);
 		std::string path(ai_path.C_Str());

        if (unsigned int tex_id;
            tex_succ && !path_texture_map.contains(path) && TextureFromFile(path, tex_id)) {
            path_texture_map[path] = Texture{
                .id=tex_id,
                .type=type
            };
        }

        if (tex_succ && path_texture_map.contains(path)) {
            textures.push_back(path_texture_map[path]);
        }
	}
    return textures;
}

bool Model::TextureFromFile(const std::string filename, unsigned int& tex_id)
{
    std::string file_path = directory + '/' + filename;

    glGenTextures(1, &tex_id);

    int width, height, num_components;
    stbi_set_flip_vertically_on_load(false);
    unsigned char *data = stbi_load(file_path.c_str(), &width, &height, &num_components, 0);
    if (data)
    {
        GLenum format;
        if (num_components == 1)
            format = GL_RED;
        else if (num_components == 3)
            format = GL_RGB;
        else if (num_components == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << file_path << std::endl;
        stbi_image_free(data);
        return false;
    }

    return true;
}