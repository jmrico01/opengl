#include "halfedge.h"

#include <stdio.h>
#include <map>

#include "main.h"
#include "km_math.h"
#include "ogl_base.h"

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

HalfEdgeMesh HalfEdgeMeshFromObj(const char* fileName)
{
    const char* MODEL_DIR = "data/models/";

    HalfEdgeMesh mesh;
    mesh.vertices.Init();
    mesh.faces.Init();
    mesh.halfEdges.Init();

    char filePath[PATH_MAX_LENGTH];
    char fullPath[PATH_MAX_LENGTH];
    CatStrings(MODEL_DIR, fileName, filePath, PATH_MAX_LENGTH);
    CatStrings(GetAppPath(), filePath, fullPath, PATH_MAX_LENGTH);
    FILE* file = fopen(filePath, "rb");
    if (!file) {
        printf("Failed to open OBJ file at: %s\n", filePath);
        return mesh;
    }

    char line[OBJ_LINE_MAX];
    DynamicArray<int> faceInds;
    faceInds.Init();

    while (fgets(line, OBJ_LINE_MAX, file)) {
        if (line[0] == '#') {
            continue;
        }
        else if (line[0] == 'v') {
            if (line[1] == ' ') {
                Vertex vertex;
                vertex.pos = ParseVec3(&line[2]);
                vertex.halfEdge = 0; // This is set later
                mesh.vertices.Append(vertex);
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

    uint32 faceVerts = 0;
    for (uint32 i = 0; i < faceInds.size; i++) {
        if (faceInds[i] == -1 || i == faceInds.size - 1) {
            // Create face
            Face face;
            face.halfEdge = mesh.halfEdges.size - 1;
            mesh.faces.Append(face);
            
            faceVerts = 0;
            continue;
        }

        uint32 vertSrc = faceInds[i] - 1;
        uint32 vertDst = faceInds[i + 1] - 1;
        if (faceInds[i + 1] == -1) {
            vertDst = faceInds[i - faceVerts] - 1;
        }

        // Create edge, i -> i + 1
        // Last edge will have wrong "next"
        std::pair<int, int> forward(vertSrc, vertDst);
        std::pair<int, int> backward(vertDst, vertSrc);
        HalfEdge he;
        he.next = mesh.halfEdges.size + 1;
        if (faceInds[i + 1] == -1) {
            he.next = mesh.halfEdges.size - faceVerts;
        }
        auto edgeBackward = edgeMap.find(backward);
        if (edgeBackward != edgeMap.end()) {
            he.twin = edgeBackward->second;
            mesh.halfEdges[edgeBackward->second].twin =
                mesh.halfEdges.size;
        }
        else {
            he.twin = 0;
        }
        he.vertex = vertDst;
        he.face = mesh.faces.size;

        // Assign forward edge i -> i + 1 to vertex i
        mesh.vertices[vertSrc].halfEdge = mesh.halfEdges.size;

        // Add edge to edgeMap and mesh.halfEdges array
        auto res = edgeMap.insert(
            std::make_pair(forward, mesh.halfEdges.size));
        if (!res.second) {
            printf("ERROR: Edge already in edgeMap\n");
        }
        mesh.halfEdges.Append(he);
        faceVerts++;
    }

    faceInds.Free();
    ComputeFaceNormals(&mesh);
    //PrintHalfEdgeMesh(mesh);

    // TODO free mesh after this
    return mesh;
}

HalfEdgeMesh CopyHalfEdgeMesh(const HalfEdgeMesh* mesh)
{
    HalfEdgeMesh newMesh;
    newMesh.vertices = mesh->vertices.Copy();
    newMesh.faces = mesh->faces.Copy();
    newMesh.halfEdges = mesh->halfEdges.Copy();

    return newMesh;
}

void FreeHalfEdgeMesh(HalfEdgeMesh* mesh)
{
    mesh->vertices.Free();
    mesh->faces.Free();
    mesh->halfEdges.Free();
}

void PrintHalfEdgeMesh(const HalfEdgeMesh& mesh)
{
    printf("----- VERTICES -----\n");
    for (uint32 i = 0; i < mesh.vertices.size; i++) {
        printf("%d: (%f, %f, %f) ; edge %d\n",
            i,
            mesh.vertices[i].pos.x,
            mesh.vertices[i].pos.y,
            mesh.vertices[i].pos.z,
            mesh.vertices[i].halfEdge);
    }
    printf("----- HALF EDGES -----\n");
    for (uint32 i = 0; i < mesh.halfEdges.size; i++) {
        printf("%d: next %d, twin: %d, vert %d, face %d\n",
            i,
            mesh.halfEdges[i].next, mesh.halfEdges[i].twin,
            mesh.halfEdges[i].vertex,
            mesh.halfEdges[i].face);
    }
    printf("----- FACES -----\n");
    for (uint32 i = 0; i < mesh.faces.size; i++) {
        printf("%d: edge %d\n",
            i,
            mesh.faces[i].halfEdge);
    }
}

HalfEdgeMeshGL LoadHalfEdgeMeshGL(const HalfEdgeMesh& mesh)
{
    HalfEdgeMesh meshTri = CopyHalfEdgeMesh(&mesh);
    TriangulateMesh(&meshTri);
    HalfEdgeMeshGL meshGL = {};

    // TODO use indexing for this
    DynamicArray<Vec3> vertices;
    vertices.Init();
    DynamicArray<Vec3> normals;
    normals.Init();

    for (uint32 f = 0; f < meshTri.faces.size; f++) {
        Vec3 normal = meshTri.faces[f].normal;
        uint32 edge = meshTri.faces[f].halfEdge;
        uint32 e = edge;
        do {
            Vertex v = meshTri.vertices[meshTri.halfEdges[e].vertex];
            vertices.Append(v.pos);
            normals.Append(normal);
            e = meshTri.halfEdges[e].next;
        } while (e != edge);
    }

    glGenVertexArrays(1, &meshGL.vertexArray);
    glBindVertexArray(meshGL.vertexArray);

    glGenBuffers(1, &meshGL.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, meshGL.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size * sizeof(Vec3),
        vertices.data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // match shader layout location
        3, // size (vec3)
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0, // stride
        (void*)0 // array buffer offset
    );

    glGenBuffers(1, &meshGL.normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, meshGL.normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size * sizeof(Vec3),
        normals.data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // match shader layout location
        3, // size (vec3)
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0, // stride
        (void*)0 // array buffer offset
    );

    glBindVertexArray(0);

    meshGL.programID = LoadShaders(
        "shaders/model.vert",
        "shaders/model.frag");

    meshGL.vertexCount = (int)vertices.size;

    return meshGL;
}

void FreeHalfEdgeMeshGL(HalfEdgeMeshGL* meshGL)
{
    glDeleteBuffers(1, &meshGL->vertexBuffer);
    glDeleteBuffers(1, &meshGL->normalBuffer);
    glDeleteProgram(meshGL->programID);
    glDeleteVertexArrays(1, &meshGL->vertexArray);

}

void DrawHalfEdgeMeshGL(const HalfEdgeMeshGL& meshGL, Mat4 proj, Mat4 view)
{
    GLint loc;
    glUseProgram(meshGL.programID);
    
    Mat4 model = Mat4::one;
    Mat4 mvp = proj * view * model;
    loc = glGetUniformLocation(meshGL.programID, "mvp");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &mvp.e[0][0]);
    loc = glGetUniformLocation(meshGL.programID, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &model.e[0][0]);
    loc = glGetUniformLocation(meshGL.programID, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &view.e[0][0]);

    glBindVertexArray(meshGL.vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, meshGL.vertexCount);
    glBindVertexArray(0);
}

// Utility functions

// Make a new edge v1 -> v2, splitting face f.
// f will now be v1 -> ... -> v2 -> v1
// new face will be v2 -> ... -> v1 -> v2
void SplitFaceMakeEdge(HalfEdgeMesh* mesh, uint32 f, uint32 v1, uint32 v2)
{
    uint32 edge = mesh->faces[f].halfEdge;
    uint32 eToV1 = edge;
    while (mesh->halfEdges[eToV1].vertex != v1) {
        eToV1 = mesh->halfEdges[eToV1].next;
    }
    uint32 eToV2 = edge;
    while (mesh->halfEdges[eToV2].vertex != v2) {
        eToV2 = mesh->halfEdges[eToV2].next;
    }

    uint32 e = eToV2;
    while (mesh->halfEdges[e].vertex != v1) {
        e = mesh->halfEdges[e].next;
        // Assign new face to edges v2 -> ... -> v1
        mesh->halfEdges[e].face = mesh->faces.size;
    }

    // Update original face f
    mesh->faces[f].halfEdge = mesh->halfEdges.size + 1; // v2 -> v1
    // Create new face
    Face newFace;
    newFace.halfEdge = mesh->halfEdges.size; // v1 -> v2

    // Create new half edges
    HalfEdge v1v2;
    v1v2.next = mesh->halfEdges[eToV2].next;
    v1v2.twin = mesh->halfEdges.size + 1;
    v1v2.vertex = v2;
    v1v2.face = mesh->faces.size;
    HalfEdge v2v1;
    v2v1.next = mesh->halfEdges[eToV1].next;
    v2v1.twin = mesh->halfEdges.size;
    v2v1.vertex = v1;
    v2v1.face = f;

    // Connect endpoints to new half edges
    mesh->halfEdges[eToV1].next = mesh->halfEdges.size;
    mesh->halfEdges[eToV2].next = mesh->halfEdges.size + 1;

    mesh->halfEdges.Append(v1v2);
    mesh->halfEdges.Append(v2v1);
    mesh->faces.Append(newFace);
}

void TriangulateMesh(HalfEdgeMesh* mesh)
{
    for (uint32 i = 0; i < mesh->faces.size; i++) {
        while (true) {
            uint32 numEdges = 0;
            uint32 edge = mesh->faces[i].halfEdge;
            uint32 e = edge;
            uint32 ePrev = e;
            do {
                numEdges++;
                if (numEdges > 3) {
                    SplitFaceMakeEdge(mesh, i, mesh->halfEdges[ePrev].vertex,
                        mesh->halfEdges[edge].vertex);
                    break;
                }
                ePrev = e;
                e = mesh->halfEdges[e].next;
            } while (e != edge);

            if (numEdges == 3) {
                break;
            }
        }
    }

    ComputeFaceNormals(mesh);
    //printf("==> TRIANGULATION FINISHED\n");
    //PrintHalfEdgeMesh((const HalfEdgeMesh&)*mesh);
}

internal Vec3 NormalFromTriangle(const Vec3 triangle[3])
{
    Vec3 a = triangle[1] - triangle[0];
    Vec3 b = triangle[2] - triangle[0];
    return Normalize(Cross(a, b));
}
// Computes normal for the face f, assuming all vertices in f are coplanar.
internal Vec3 ComputeFaceNormal(const HalfEdgeMesh& mesh, uint32 f)
{
    Vec3 triangle[3];
    int numEdges = 0;
    uint32 edge = mesh.faces[f].halfEdge;
    uint32 e = edge;
    do {
        triangle[numEdges++] = mesh.vertices[mesh.halfEdges[e].vertex].pos;
        if (numEdges >= 3) {
            break;
        }
        e = mesh.halfEdges[e].next;
    } while (e != edge);

    return NormalFromTriangle(triangle);
}
void ComputeFaceNormals(HalfEdgeMesh* mesh)
{
    for (uint32 f = 0; f < mesh->faces.size; f++) {
        Vec3 normal = ComputeFaceNormal((const HalfEdgeMesh&)*mesh, f);
        mesh->faces[f].normal = normal;
    }
}

internal float32 ComputeTriangleArea(Vec3 triangle[3])
{
    Vec3 a = triangle[1] - triangle[0];
    Vec3 b = triangle[2] - triangle[0];
    float magA = Mag(a);
    float magB = Mag(b);
    float cosTheta = Dot(a, b) / (magA * magB);
    float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);
    
    return magA * magB * sinTheta / 2.0f;
}
void ComputeFaceAreas(HalfEdgeMesh* mesh)
{
    Vec3 triangle[3];

    for (uint32 f = 0; f < mesh->faces.size; f++) {
        float32 faceArea = 0.0f;

        uint32 edge = mesh->faces[f].halfEdge;
        uint32 e = edge;
        triangle[0] = mesh->vertices[mesh->halfEdges[e].vertex].pos;
        e = mesh->halfEdges[e].next;
        triangle[1] = mesh->vertices[mesh->halfEdges[e].vertex].pos;
        do {
            e = mesh->halfEdges[e].next;
            triangle[2] = mesh->vertices[mesh->halfEdges[e].vertex].pos;
            faceArea += ComputeTriangleArea(triangle);
        } while (e != edge);

        mesh->faces[f].area = faceArea;
    }
}

// Mesh traversal functions
void VerticesOnFace(const HalfEdgeMesh& mesh, uint32 f,
    DynamicArray<Vertex>& out)
{
    uint32 edge = mesh.faces[f].halfEdge;
    uint32 e = edge;
    do {
        out.Append(mesh.vertices[mesh.halfEdges[e].vertex]);
        e = mesh.halfEdges[e].next;
    } while (e != edge);
}
void EdgesOnFace(const HalfEdgeMesh& mesh, uint32 f,
    DynamicArray<HalfEdge>& out)
{
    uint32 edge = mesh.faces[f].halfEdge;
    uint32 e = edge;
    do {
        out.Append(mesh.halfEdges[e]);
        e = mesh.halfEdges[e].next;
    } while (e != edge);
}
void FacesOnFace(const HalfEdgeMesh& mesh, uint32 f,
    DynamicArray<Face>& out)
{
    uint32 edge = mesh.faces[f].halfEdge;
    uint32 e = edge;
    do {
        uint32 twin = mesh.halfEdges[e].twin;
        out.Append(mesh.faces[mesh.halfEdges[twin].face]);
        e = mesh.halfEdges[e].next;
    } while (e != edge);
}

void VerticesOnEdge(const HalfEdgeMesh& mesh, uint32 e,
    DynamicArray<Vertex>& out)
{
}
void FacesOnEdge(const HalfEdgeMesh& mesh, uint32 e,
    DynamicArray<Face>& out)
{
}

void VerticesOnVertex(const HalfEdgeMesh& mesh, uint32 v,
    DynamicArray<Vertex>& out)
{
}
void EdgesOnVertex(const HalfEdgeMesh& mesh, uint32 v,
    DynamicArray<Vertex>& out)
{
}
void FacesOnVertex(const HalfEdgeMesh& mesh, uint32 v,
    DynamicArray<Vertex>& out)
{
}