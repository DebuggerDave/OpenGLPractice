#include <mesh.h>

#include <vector>
#include <string>
#include <iostream>

Mesh::Mesh() :
    VAO(0), VBO(0), EBO(0)
{}


Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) :
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

void Mesh::setupMesh()
{
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

void Mesh::Draw(Shader &shader) 
{
    // textures
    for(unsigned int diff_num=0, spec_num=0, i=0; i < textures.size(); i++)
    {
    	TexType name = textures[i].type;
        std::string uniform;
        if (name == TexType::Diffuse) {
            uniform = "material.texture_diffuse" + std::to_string(diff_num++);
		} else if (name == TexType::Specular) {
            uniform = "material.texture_specular" + std::to_string(spec_num++);
		} else {
            continue;
        }

        shader.setInt(uniform.c_str(), i);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    // draw
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}