#pragma once

#include "km_math.h"

struct HalfEdge;

struct Vertex {
    Vec3 position;
    HalfEdge* halfEdge;
};

struct Face {
    HalfEdge* halfEdge;
};

struct HalfEdge {
    HalfEdge* next;
    HalfEdge* twin;
    Vertex* vertex;
    Face* face;
};

struct HalfEdgeMesh {
    uint32 numVertices;
    Vertex* vertices;
    uint32 numFaces;
    Face* faces;
    uint32 numEdges;
    HalfEdge* halfEdges;
};

HalfEdgeMesh HalfEdgeMeshFromObj(const char* filePath);