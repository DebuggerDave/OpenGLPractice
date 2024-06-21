#ifndef MESH_H
#define MESH_H

#include "glm/fwd.hpp"
#include "glad/gl.h"

#include <vector>
#include <iosfwd>

class Shader;

class Mesh {
    public:
        struct Vertex {
	        glm::vec3 Position;
            glm::vec3 Normal;
            glm::vec2 TexCoords;
        };

        enum class TexType {
            Diffuse,
            Specular,
            Normal,
            NumTexTypes
        };

        struct Texture {
            unsigned int id;
            TexType type;
        };

        Mesh(const std::vector<Vertex>& vertices = std::vector<Vertex>{},
            const std::vector<unsigned int>& indices = std::vector<unsigned int>{},
            const std::vector<Texture>& textures = std::vector<Texture>{}
            );
        ~Mesh();
        Mesh(const Mesh& other) = delete;
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(const Mesh& other) = delete;
        Mesh& operator=(Mesh&& other) noexcept;

        void draw(const Shader& shader) const;
    private:
        //  render data
        unsigned int VAO, VBO, EBO;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        // convert texture enum to string
        std::string texTypeToString(const TexType type) const;
        // check if the class is ready to be setup
        bool readyForSetup() const;
        // initial setup
        void setupMesh();
        void free();
};  

#endif
