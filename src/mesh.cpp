#include "mesh.h"

#include "shader.h"
#include "world.h"
#include "component.h"
#include "utils.h"

#include "glad/gl.h"

#include <vector>
#include <string>
#include <utility>
#include <cmath>
#include <type_traits>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures) :
    VAO(0), VBO(0), EBO(0),
    vertices(vertices), indices(indices), textures(textures)
{
    if (readyForSetup()) {
        setupMesh();
    }
}

Mesh::~Mesh() {
    free();
}


Mesh::Mesh(Mesh&& other) noexcept
{
    *this = std::move(other);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    free();
    if (this != &other) {
        VAO = std::exchange(other.VAO, 0);
        VBO = std::exchange(other.VBO, 0);
        EBO = std::exchange(other.EBO, 0);

        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        textures = std::move(other.textures);
    }
    return *this;
}

void Mesh::draw(const Shader& shader, const unsigned int num) const
{
    // textures
    if (textures.size() > GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS) {
        LOG("unable to use all textures, exceeded max texture units")
    }
    size_t num_tex = std::min((size_t)GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, textures.size());
    for(unsigned int tex_nums[(int)TexType::NumTexTypes]={0}, i=0; i < num_tex; i++)
    {
        TexType type = textures[i].type;
        std::string uniform;
        uniform = "material.texture_" + texTypeToString(type) + std::to_string(tex_nums[(unsigned int)type]++);

        shader.setInt(uniform.c_str(), i);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // draw
    glBindVertexArray(VAO);
    if (num == 0) {
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    } else {
        glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, num);
    }
}

void Mesh::setupInstancing(const World& world, const BlockId id) const
{
    world.setupInstancing(VAO, instance_vertex_attrib_index, id);
}

std::string Mesh::texTypeToString(const TexType type) const
{
    if (type == TexType::Diffuse) {
        return "diffuse";
    } else if (type == TexType::Specular) {
        return "specular";
    } else if (type == TexType::Normal) {
        return "normal";
    } else {
        LOG("unable to convert TexType to string, type unknown")
        return std::string("unknown");
    }
}

bool Mesh::readyForSetup() const {
    // textures can be empty
    return (VAO == 0) && (VBO == 0) && (EBO == 0) && !vertices.empty() && !indices.empty();
}

void Mesh::setupMesh()
{
    if (!readyForSetup()) {
        LOG("class is not ready for setup")
        return;
    }

    glGenVertexArrays(1, &VAO); 
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
}

void Mesh::free() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}
