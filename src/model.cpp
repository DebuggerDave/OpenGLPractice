#include "model.h"

#include "pch.h"
#include "shader.h"
#include "mesh.h"
#include "utils.h"

#include <vector>

Model::Model(const std::string& path)
{
	loadModel(path);
}

void Model::Draw(const Shader& shader) const
{
    for(unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].Draw(shader);
	}
}

void Model::loadModel(const std::string& path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        LOG("ERROR::ASSIMP::" << import.GetErrorString() << "")
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(const aiNode* node, const aiScene* scene)
{
    // setup recursion
    meshes = std::vector<Mesh>(scene->mNumMeshes);
    unsigned int mesh_num = 0;
    processNodeRecursion(node, scene, mesh_num);
}

void Model::processNodeRecursion(const aiNode* node, const aiScene* scene, unsigned int& mesh_num)
{
    // recursion
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNodeRecursion(node->mChildren[i], scene, mesh_num);
    }

    // process meshes
    for(unsigned int i = 0; i < node->mNumMeshes; i++, mesh_num++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes[mesh_num] = processMesh(mesh, scene);
    }
}

std::vector<Mesh::Vertex> Model::processVertices(const aiMesh* mesh) const
{
    std::vector<Mesh::Vertex> vertices(mesh->mNumVertices);

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

    return vertices;
}

std::vector<unsigned int> Model::processIndices(const aiMesh* mesh) const
{
    static const int expected_face_vertices = 3;
    std::vector<unsigned int> indices(mesh->mNumFaces * expected_face_vertices);

    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
		if (mesh->mFaces[i].mNumIndices != expected_face_vertices) {
			LOG("expected " << expected_face_vertices << " vertices in mesh face, but received " << mesh->mFaces[i].mNumIndices << " vertices")
		}

    	for(unsigned int j = 0; j < expected_face_vertices; j++) {
            const unsigned int num_preceeding_indices = i * expected_face_vertices;
            indices[num_preceeding_indices + j] = (mesh->mFaces[i].mIndices[j]);
        }
    }

    return indices;
}

std::vector<Mesh::Texture> Model::processTextures(const aiMesh* mesh, const aiScene* scene)
{
    std::vector<Mesh::Texture> textures;

    const unsigned int matIndex = mesh->mMaterialIndex;
	if(mesh->mMaterialIndex >= 0)
	{
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

		std::vector<Mesh::Texture> diffuse_maps = loadMaterialTextures(material, aiTextureType_DIFFUSE, Mesh::TexType::Diffuse);
		textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

		std::vector<Mesh::Texture> specular_maps = loadMaterialTextures(material, aiTextureType_SPECULAR, Mesh::TexType::Specular);
		textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

        std::vector<Mesh::Texture> normal_maps = loadMaterialTextures(material, aiTextureType_HEIGHT, Mesh::TexType::Normal);
		textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());
	}

    return textures;
}

Mesh Model::processMesh(const aiMesh* mesh, const aiScene* scene)
{
    std::vector<Mesh::Vertex> vertices = processVertices(mesh);
    std::vector<unsigned int> indices = processIndices(mesh);
    std::vector<Mesh::Texture> textures = processTextures(mesh, scene);

    return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

std::vector<Mesh::Texture> Model::loadMaterialTextures(const aiMaterial* mat, const aiTextureType ai_type, const Mesh::TexType type)
{
    std::vector<Mesh::Texture> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(ai_type); i++)
    {
        aiString ai_path;
        bool tex_succ = (mat->GetTexture(ai_type, i, &ai_path) == aiReturn_SUCCESS);
 		std::string path(ai_path.C_Str());

        // if we don't have the texture in our map, attempt an insertion
        if (unsigned int tex_id;
            tex_succ && !path_texture_map.contains(path) && texFromFile(path, tex_id, type)) {
            path_texture_map[path] = Mesh::Texture{
                .id=tex_id,
                .type=type
            };
        }

        // if it's in our map, add it to the output
        if (tex_succ && path_texture_map.contains(path)) {
            textures.push_back(path_texture_map[path]);
        }
	}

    return textures;
}

void Model::createTex(unsigned int& tex_id, const unsigned char* data, const int width, const int height, const int num_components, const Mesh::TexType type) {
    glGenTextures(1, &tex_id);
    static const unsigned int pixel_art_threshold = 64;

    GLenum format;
    GLenum internal_format;
    if (num_components == 1) {
        format = GL_RED;
        internal_format = GL_RED;
    }
    else if (num_components == 3) {
        format = GL_RGB;
        internal_format = (type == Mesh::TexType::Diffuse) ? GL_SRGB : GL_RGB;
    }
    else if (num_components == 4) {
        format = GL_RGBA;
        internal_format = (type == Mesh::TexType::Diffuse) ? GL_SRGB_ALPHA : GL_RGBA;
    }

    unsigned short tex_min_filter = 0, tex_mag_filter = 0;
    if ((width <= pixel_art_threshold) && (height <= pixel_art_threshold)) {
        tex_min_filter = GL_NEAREST_MIPMAP_NEAREST;
        tex_mag_filter = GL_NEAREST;
    } else {
        tex_min_filter = GL_LINEAR_MIPMAP_LINEAR;
        tex_mag_filter = GL_LINEAR;
    }

    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_mag_filter);
}

bool Model::texFromFile(const std::string& filename, unsigned int& tex_id, const Mesh::TexType type)
{
    std::string file_path = directory + '/' + filename;
    int width, height, num_components;
    bool succ = false;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(file_path.c_str(), &width, &height, &num_components, 0);
    if (data)
    {
        createTex(tex_id, data, width, height, num_components, type);
        succ = true;
    }
    else
    {
        LOG("Mesh::Texture failed to load at path: " << file_path)
        succ = false;
    }

    stbi_image_free(data);
    return succ;
}