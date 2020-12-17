#include "transform.hpp"

#include <glm/gtc/matrix_access.hpp>

/// Extracts the translation from this transformation
/// NOTE: based on formulas from: https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati/417813
glm::vec3 Transform::translation() const {
    return {(*this)[3][0], (*this)[3][1], (*this)[3][2]};
}

/// Extracts the scale from this transformation
/// NOTE: based on formulas from: https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati/417813
glm::vec3 Transform::scale() const {
    return {glm::length(glm::column(*this, 0)), glm::length(glm::column(*this, 1)), glm::length(glm::column(*this, 2))};
}

/// NOTE: based on formulas from: https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati/417813
Quaternion rotationExtracter(const Transform* t, glm::vec3& scale){
    glm::mat4 out = *t;
    // Remove translation
    out[3][0] = out[3][1] = out[3][2] = 0;
    // Remove scale
    return glm::scale(out, {1/scale.x, 1/scale.y, 1/scale.z});
}
Quaternion rotationExtracter(const Transform* t, glm::vec3&& s){ return rotationExtracter(t, s); }

/// Extracts the rotation from this transformation
Quaternion Transform::rotation() const {
    return rotationExtracter(this, scale());
}

/// Decomposes the transformation into a translation, rotation, and scale
/// NOTE: based on formulas from: https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati/417813
std::tuple<glm::vec3, Quaternion, glm::vec3> Transform::decompose() const {
    glm::vec3 scale = this->scale();
    return {translation(), rotationExtracter(this, scale), scale};
}
