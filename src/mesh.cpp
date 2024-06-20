#include "mesh.h"

#include "pch.h"
#include "utils.h"
#include "shader.h"

#include <vector>

Mesh::Mesh() :
    VAO(0), VBO(0), EBO(0)
{}


Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures) :
    VAO(0), VBO(0), EBO(0),
    vertices(vertices), indices(indices), textures(textures)
{
    setupMesh();
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

Mesh::Mesh(Mesh&& other) noexcept :
    VAO(std::exchange(other.VAO, 0)),
    VBO(std::exchange(other.VBO, 0)),
    EBO(std::exchange(other.EBO, 0)),
    vertices(std::move(other.vertices)),
    indices(std::move(other.indices)),
    textures(std::move(other.textures))
    {}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    vertices = std::move(other.vertices);
    indices = std::move(other.indices);
    textures = std::move(other.textures);

    VAO = std::exchange(other.VAO, 0);
    VBO = std::exchange(other.VBO, 0);
    EBO = std::exchange(other.EBO, 0);

    return *this;
}

void Mesh::Draw(const Shader& shader) const
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
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
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

    glBindVertexArray(0);
}
