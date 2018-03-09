#include "halfedge.h"

#include <stdio.h>
#include <map>

#include "main.h"
#include "km_lib.h"
#include "km_math.h"

#define OBJ_LINE_MAX 512

Vec3 ParseVec3(char* str)
{
    Vec3 result = { 0.0f, 0.0f, 0.0f };
    char* el = str;
    int i = 0;
    for (char* c = str; *c != '\0'; c++) {
        if (*c == ' ' || *c == '\n') {
            char* endptr = el;
            *c = '\0';
            result.e[i++] = (float32)strtod(el, &endptr);
            // TODO check error
            el = c + 1;
        }
    }

    return result;
}

HalfEdgeMesh HalfEdgeMeshFromObj(const char* filePath)
{
    HalfEdgeMesh mesh = {
        0, nullptr,
        0, nullptr,
        0, nullptr
    };

    char fullPath[PATH_MAX_LENGTH];
    CatStrings(GetAppPath(), filePath, fullPath, PATH_MAX_LENGTH);
    FILE* file = fopen(filePath, "rb");
    if (!file) {
        printf("Failed to open OBJ file at: %s\n", filePath);
        return mesh;
    }

    char line[OBJ_LINE_MAX];
    DynamicArray<Vertex> vertices;
    vertices.Init();
    DynamicArray<int> faceInds;
    faceInds.Init();

    while (fgets(line, OBJ_LINE_MAX, file)) {
        if (line[0] == '#') {
            continue;
        }
        else if (line[0] == 'v') {
            if (line[1] == ' ') {
                Vec3 vertexPos = ParseVec3(&line[2]);
                vertices.Append({ vertexPos, 0 });
            }
            else if (line[1] == 'n') {
                printf("vertex normal! UNHANDLED\n");
            }
            else {
                // idk
            }
        }
        else if (line[0] == 'f') {
            char* el = &line[2];
            int i = 0;
            for (char* c = &line[2]; *c != '\0'; c++) {
                if (*c == ' ' || *c == '\n') {
                    char* endptr = el;
                    *c = '\0';
                    faceInds.Append((int)strtol(el, &endptr, 10));
                    // TODO check error
                    el = c + 1;
                }
            }
            faceInds.Append(-1);
        }
        else {
            // idk
        }
    }

    std::map<std::pair<uint32, uint32>, uint32> edgeMap;
    DynamicArray<HalfEdge> halfEdges;
    halfEdges.Init();
    /*DynamicArray<HalfEdge> edgesForward;
    edgesForward.Init();
    DynamicArray<HalfEdge> edgesBackward;
    edgesBackward.Init();*/
    DynamicArray<Face> faces;
    faces.Init();

    uint32 faceVerts = 0;
    for (uint32 i = 0; i < faceInds.size; i++) {
        if (faceInds.data[i] == -1 || i == faceInds.size - 1) {
            // Create face
            Face face;
            face.halfEdge = halfEdges.size - 1;
            faces.Append(face);
            
            faceVerts = 0;
            continue;
        }

        uint32 vertSrc = faceInds.data[i] - 1;
        uint32 vertDst = faceInds.data[i + 1] - 1;
        if (faceInds.data[i + 1] == -1) {
            vertDst = faceInds.data[i - faceVerts] - 1;
        }

        // Create edge, i -> i + 1
        // Last edge will have wrong "next"
        std::pair<int, int> forward(vertSrc, vertDst);
        std::pair<int, int> backward(vertDst, vertSrc);
        HalfEdge he;
        he.next = halfEdges.size + 1;
        if (faceInds.data[i + 1] == -1) {
            he.next = halfEdges.size - faceVerts;
        }
        auto edgeBackward = edgeMap.find(backward);
        if (edgeBackward != edgeMap.end()) {
            he.twin = edgeBackward->second;
            halfEdges.data[edgeBackward->second].twin = halfEdges.size;
            printf("Connected twins: %d and %d\n",
                edgeBackward->second, halfEdges.size);
        }
        else {
            he.twin = 0;
        }
        he.vertex = vertDst;
        he.face = faces.size;

        // Assign forward edge i -> i + 1 to vertex i
        vertices.data[vertSrc].halfEdge = halfEdges.size;

        // Add edge to edgeMap and halfEdges array
        auto res = edgeMap.insert(std::make_pair(forward, halfEdges.size));
        if (!res.second) {
            printf("ERROR: Edge already in edgeMap\n");
        }
        printf("mapped (%d, %d) -> %d\n", forward.first, forward.second,
            halfEdges.size);
        halfEdges.Append(he);
        faceVerts++;
    }

    mesh.numVertices = vertices.size;
    mesh.vertices = vertices.data;

    mesh.numHalfEdges = halfEdges.size;
    mesh.halfEdges = halfEdges.data;

    mesh.numFaces = faces.size;
    mesh.faces = faces.data;

    printf("----- VERTICES -----\n");
    for (uint32 i = 0; i < mesh.numVertices; i++) {
        printf("%d: (%f, %f, %f) ; edge %d\n",
            i,
            mesh.vertices[i].pos.x,
            mesh.vertices[i].pos.y,
            mesh.vertices[i].pos.z,
            mesh.vertices[i].halfEdge);
    }
    printf("----- HALF EDGES -----\n");
    for (uint32 i = 0; i < mesh.numHalfEdges; i++) {
        printf("%d: next %d, twin: %d, vert %d, face %d\n",
            i,
            mesh.halfEdges[i].next, mesh.halfEdges[i].twin,
            mesh.halfEdges[i].vertex,
            mesh.halfEdges[i].face);
    }
    printf("----- FACES -----\n");
    for (uint32 i = 0; i < mesh.numFaces; i++) {
        printf("%d: edge %d\n",
            i,
            mesh.faces[i].halfEdge);
    }

    // TODO free mesh after this
    return mesh;
}

HalfEdgeMeshGL LoadHalfEdgeMeshGL(const HalfEdgeMesh& mesh)
{
    HalfEdgeMeshGL meshGL;

    /*TexturedRectGL texturedRectGL;
    // TODO probably use indexing for this
    const GLfloat vertices[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    };
    const GLfloat uvs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f
    };

    glGenVertexArrays(1, &texturedRectGL.vertexArray);
    glBindVertexArray(texturedRectGL.vertexArray);

    glGenBuffers(1, &texturedRectGL.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texturedRectGL.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // match shader layout location
        3, // size (vec3)
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0, // stride
        (void*)0 // array buffer offset
    );

    glGenBuffers(1, &texturedRectGL.uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texturedRectGL.uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // match shader layout location
        2, // size (vec2)
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0, // stride
        (void*)0 // array buffer offset
    );

    glBindVertexArray(0);

    texturedRectGL.programID = LoadShaders(
        "shaders/texturedRect.vert",
        "shaders/texturedRect.frag");
    
    return texturedRectGL;*/

    return meshGL;
}

void FreeHalfEdgeMeshGL(const HalfEdgeMeshGL& meshGL)
{
}