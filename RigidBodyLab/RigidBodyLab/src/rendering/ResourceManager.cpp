#include <rendering/ResourceManager.h>
using namespace Rendering;

ResourceManager::ResourceManager() {
    meshes[TO_INT(MeshID::CUBE)] = CreateCube(1, 1, 1);
    meshes[TO_INT(MeshID::VASE)] = LoadOBJMesh("../RigidBodyLab/models/vase.obj");
    meshes[TO_INT(MeshID::PLANE)] = CreatePlane(1, 1);
    meshes[TO_INT(MeshID::SPHERE)] = CreateSphere(16, 16);
}

Mesh& ResourceManager::GetMesh(MeshID id) {
    return meshes[TO_INT(id)];
}
