#ifndef MESH_H
#define MESH_H

class Shader;
class World;
enum class BlockId : unsigned int;

#include "glad/gl.h"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#include <vector>
#include <iosfwd>

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

        // draw number of instances indicated by num, zero draws without instancing
        void draw(const Shader& shader, const unsigned int num = 0) const;
        // add a vertex attribute array of vec4s for instance rendering
        void setupInstancing(const World& world, const BlockId id) const;

    private:
        //  render data
        GLuint VAO, VBO, EBO;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;
        static const GLuint instance_vertex_attrib_index = 3;

        // convert texture enum to string
        std::string texTypeToString(const TexType type) const;
        // check if the class is ready to be setup
        bool readyForSetup() const;
        // initial setup
        void setupMesh();
        void free();
};  

#endif
