#pragma once

#include <GL/glew.h>
#include <map>

#include "km_math.h"

struct HalfEdge;

struct Vertex {
    Vec3 pos;
    uint32 halfEdge;
};

struct Face {
    uint32 halfEdge;
};

struct HalfEdge {
    uint32 next;
    uint32 twin;
    uint32 vertex;
    uint32 face;
};

struct HalfEdgeMesh {
    uint32 numVertices;
    Vertex* vertices;
    uint32 numFaces;
    Face* faces;
    uint32 numHalfEdges;
    HalfEdge* halfEdges;
    // TODO stop using std god damnit
    std::map<std::pair<uint32, uint32>, uint32>* edgeMap;
};

struct HalfEdgeMeshGL
{
    GLuint vertexArray;
    GLuint vertexBuffer;
    GLuint indexBuffer;
    GLuint programID;
};

HalfEdgeMesh HalfEdgeMeshFromObj(const char* filePath);
void SplitFaceMakeEdge(const HalfEdgeMesh& mesh);
void FreeHalfEdgeMesh(const HalfEdgeMesh& mesh);

HalfEdgeMeshGL LoadHalfEdgeMeshGL(const HalfEdgeMesh& mesh);
void FreeHalfEdgeMeshGL(const HalfEdgeMeshGL& meshGL);