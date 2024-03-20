#pragma once

#include <vector>
#include <GLFW/glfw3.h>
#include <math/Math.h>
#include <math/Vector3.h>
#include <math/Matrix4.h>

using Math::Vector3;
namespace Rendering {
    /*  Vertex format */
    struct Vertex
    {
        Vertex(Vec3 pos, Vec3 nrm, Vec3 tan, Vec3 bitan, Vec2 uv) : pos(pos), nrm(nrm), tan(tan), bitan(bitan), uv(uv) { }

        Vertex() : pos(Vec3()), nrm(Vec3()), tan(Vec3()), bitan(Vec3()), uv(Vec2()) { }

        Vec3 pos, nrm, tan, bitan;
        Vec2 uv;
    };
    const int vertexSize = sizeof(Vertex);
    const int indexSize = sizeof(int);


    /*  The layouts for specifying the offsets of a vertex
        when it is copied into the graphics pipeline. */
    struct VertexLayout
    {
        int location;
        int size;
        int type;
        bool normalized;
        int offset;
    };

    struct BoundingBoxInfo {
        Vector3 center;
        Vector3 extents;

        BoundingBoxInfo(const Vector3& center= Vector3(0, 0, 0), const Vector3& extents= Vector3(1.f, 1.f, 1.f)) : center(center), extents(extents) {}
    };

    const VertexLayout vLayout[] =
    {
        { 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos) },
        { 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, nrm) },
        { 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tan) },
        { 3, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, bitan) },
        { 4, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv) }
    };

    const int layoutSize = sizeof(VertexLayout);
    const int numAttribs = sizeof(vLayout) / layoutSize;

    using VertexBuffer=std::vector<Vertex>;
    using IndexBuffer=std::vector<int>;

    /*  Mesh format, only contains geometric data but not color/texture */
    struct Mesh
    {
        Mesh();
        /*  Storing the actual vertex/index data */
        VertexBuffer vertexBuffer;
        IndexBuffer indexBuffer;

        int numVertices;
        int numTris;
        int numIndices;

        /*  Once the buffer data has been copied into the pipeline, these array object and
            buffer objects will store  its "states".
            Later on if we want to render a mesh, we just need to bind the VAO.
            We don't need to copy the buffer data again.
        */
        GLuint VAO;
        GLuint VBO;
        GLuint IBO;
        
        BoundingBoxInfo m_boundingBox;//be default scl=(1,1,1), center={0,0,0}

        Math::Matrix4 GetBoundingBoxMat()const;
        /*  Mesh function(s) */
        static Mesh CreatePlane(int stacks, int slices);
        static Mesh CreateCube(int length, int height, int width);
        static Mesh CreateSphere(int stacks, int slices);
        static Mesh LoadOBJMesh(char* filename, bool flipX = false);
    };

}