#pragma once

#include "math.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Quaternion: public glm::quat {
public:
    using glm::quat::quat;
    // Automatically convert matricies to quaternions
    Quaternion(glm::mat4& mat) : glm::quat(glm::toQuat(mat)) {}
    Quaternion(glm::mat3& mat) : glm::quat(glm::toQuat(mat)) {}

    glm::vec3 eulerAngles() const { return glm::eulerAngles(*this); }

    // Convert the quaternion to a matrix
    glm::mat4 toMat4() const { return glm::toMat4(*this); }
    glm::mat3 toMat3() const { return glm::toMat3(*this); }
    // Implicit conversion of a quaternion to a matrix
    operator glm::mat4 () const { return toMat4(); }
    operator glm::mat3 () const { return toMat3(); }

public:
    static Quaternion fromEulerAngles(glm::vec3 angels) { return angels; }
};

/// Class presenting the transformation of an object
class Transform: public glm::mat4 {
public:
    using glm::mat4::mat4;
    using glm::mat4::operator=;

    /// Create a transform from a translation, rotation, and scale
    Transform(glm::vec3 translation = {0, 0, 0}, Quaternion rotation = Quaternion::fromEulerAngles({0, 0, 0}), glm::vec3 scale = {1, 1, 1})
    : glm::mat4(glm::translate(glm::identity<glm::mat4>(), translation) * rotation.toMat4() * glm::scale(glm::identity<glm::mat4>(), scale)) { }

    /// Translate the transform by the given ammounts
    Transform& translate(glm::vec3& tran){ (*this)[3][0] += tran.x; (*this)[3][1] += tran.y; (*this)[3][2] += tran.z; return *this; }
    Transform& translate(glm::vec3&& t){ return translate(t); }
    /// Rotate the transform by the given quaternion.
    Transform& rotate(Quaternion& rotation){ *this = rotation.toMat4() * (*this); return *this; }
    Transform& rotate(Quaternion&& rotation){ return rotate(rotation); }
    /// Rotate the transform by the given quaternion.
    ///     The rotation will be preformed on the decomposed transform.
    ///     In otherwords it will rotate pre-translate instead of post-translate
    Transform& rotatePiecewise(Quaternion& rotation){
        auto [t, r, s] = decompose();
        *this = Transform(t, rotation.toMat4() * r.toMat4(), s);
        return *this;
    }
    Transform& rotatePiecewise(Quaternion&& rotation){ return rotatePiecewise(rotation); }
    /// Scale the transform by the given ammounts
    Transform& scale(glm::vec3& scale) { *this = glm::scale(*this, scale); return *this; }
    Transform& scale(glm::vec3&& s) { return scale(s); }
    /// Uniformly scale the tranform by the given ammounts
    Transform& scale(float scale){ *this = glm::scale(*this, {scale, scale, scale}); return *this; }

    /// Extracts the translation from this transformation
    glm::vec3 translation() const;
    /// Extracts the rotation from this transformation
    Quaternion rotation() const;
    /// Extracts the scale from this transformation
    glm::vec3 scale() const;
    /// Decomposes the transformation into a translation, rotation, and scale
    std::tuple<glm::vec3, Quaternion, glm::vec3> decompose() const;

    /// Applies the transformation to the given point
    glm::vec4 operator() (glm::vec4& toTranslate) const { return *this * toTranslate; }
    glm::vec4 operator() (glm::vec4&& o) const { return operator() (o);}
    glm::vec4 operator() (glm::vec3& o) const { return *this * glm::vec4{o.x, o.y, o.z, 1}; }
    glm::vec4 operator() (glm::vec3&& o) const { return operator() (o);}
};
