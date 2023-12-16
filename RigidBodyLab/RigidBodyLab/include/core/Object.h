#pragma once
#include <math/Math.h>
#include <utilities/ToUnderlyingEnum.h>
#include <rendering/Mesh.h>
#include <rendering/ResourceManager.h>
#include <vector>

namespace Rendering {

	class Object {
	private:
		//Dependency Injection
		Mesh& m_mesh;     

		ImageID m_imageID;
		Mat4 m_modelMatrix;   

	public:
		Object::Object(Mesh& mesh, ImageID imageID, const Mat4& modelMat)
			: m_mesh(mesh), m_imageID(imageID), m_modelMatrix(modelMat) {
		}

		Object(const Object& other) = default;
		Object(Object&& other) noexcept = default;
		Object& operator=(const Object& other) = default;
		Object& operator=(Object&& other) noexcept = default;

		// Getter methods (setters might not be necessary because we are passing by reference)
		const Mesh& GetMesh() const { return m_mesh; }
		const Mat4& GetModelMatrix() const { return m_modelMatrix; }

		ImageID GetImageID() const { return m_imageID; }
		void SetImageID(ImageID id) { m_imageID = id; }
	};

}
