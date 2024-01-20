#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <utility>

#include <shader.h>

class Mesh {
    public:
        struct Vertex {
	        glm::vec3 Position;
            glm::vec3 Normal;
            glm::vec2 TexCoords;
        };

        enum TexType {
	        Diffuse,
	        Specular
        };

        struct Texture {
            unsigned int id;
            TexType type;
        };

        Mesh();
        Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures);
        ~Mesh();
        Mesh(const Mesh& other) = delete;
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(const Mesh& other) = delete;
        Mesh& operator=(Mesh&& other) noexcept;

        void Draw(const Shader& shader);
    private:
        //  render data
        unsigned int VAO, VBO, EBO;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        void setupMesh();
};  

#endif
