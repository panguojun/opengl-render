#pragma once
#include <vector>
#include "etart.hpp"
#include <GL/glew.h>
#include <GL/glut.h>

#pragma comment(lib, "libPMDLL")

// ********************************************
// mesh
// ********************************************
struct mesh_t
{
    int drawCount = 0;
    GLuint VBO[4], VAO;
    mesh_t() {}
    void bendsm(submesh& sm)
    {
        std::vector<vec3> positions;
        std::vector<vec2> texCoords;
        std::vector<vec3> normals;
        std::vector<int> tris;

        for (unsigned int i = 0; i < sm.vertices.size(); ++i)
        {
            positions.push_back(sm.vertices[i].p);
            texCoords.push_back(sm.vertices[i].uv);
            normals.push_back(sm.vertices[i].n);
        }
        for (unsigned int i = 0; i < sm.tris.size(); ++i)
        {
            tris.push_back(sm.tris[i].vertexIndex[0]);
            tris.push_back(sm.tris[i].vertexIndex[1]);
            tris.push_back(sm.tris[i].vertexIndex[2]);
        }

        drawCount = tris.size();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(4, VBO);
        glBindVertexArray(VAO);
       

        // positions
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]), &positions[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        // texCoords
        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(texCoords[0]), &texCoords[0], GL_STATIC_DRAW);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        // normals
        glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), &normals[0], GL_STATIC_DRAW);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);

        // indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[3]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, tris.size() * sizeof(tris[0]), &tris[0], GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

   ~mesh_t()
    {
        glDeleteVertexArrays(1, &VAO);
    }

    void draw()
    {
        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLES, drawCount, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }
};