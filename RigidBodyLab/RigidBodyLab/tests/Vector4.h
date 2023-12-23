#pragma once

#include "Vector3.h"
#include <iostream>

namespace Math {
    /* Brief Explanation of 3D Homogeneous Coordinates: (x, y, z, w)
     *
     * What is 'w'?
     * - 'w' is a scaling factor in homogeneous coordinates that differentiates between points and vectors.
     * - It also plays a crucial role in perspective projection, allowing depth representation and proper scaling of objects based on distance.
     *
     * For Points (w = 1),
     * - When w = 1, the coordinates reduce to (x/w, y/w, z/w, 1), which correspond to standard Cartesian coordinates.
     * - This enables affine transformations, including translation, as the translation components of a matrix affect the point during multiplication.
     *
     * For Vectors (w = 0),
     * - When w = 0, the coordinates represent a direction or a vector in space, not affected by translation.
     * - Here, only the scale and rotation parts of the transformation matrix play a role. The translation components do not affect vectors, aligning with their geometric interpretation as direction or magnitude without a fixed origin.
     *
     * In summary, 'w' in homogeneous coordinates is a versatile tool in 3D graphics and geometry, allowing for a unified approach to representing and transforming points and vectors in 3D space, as well as facilitating perspective projections.
     */
    struct Vector4 {
        Vector3 vec3;
        float w;

        //by default a point
        Vector4(const Vector3& _vec3, float _w = 1.0f)
            : vec3(_vec3), w(_w) {}

        float& operator[](unsigned int idx) {
            if (idx > 3) throw std::out_of_range("index out of range");
            return idx < 3 ? vec3[idx] : w;
        }

        const float& operator[](unsigned int idx) const {
            if (idx > 3) throw std::out_of_range("index out of range");
            return idx < 3 ? vec3[idx] : w;
        }

        operator Vector3() const {
            return vec3;
        }
    };
}