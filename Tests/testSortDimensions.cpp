#include <cassert>

#include "tests.h"
#include "illEngine/Util/Geometry/geomUtil.h"

void testSortDimensions() {
    assert(sortDimensions(glm::vec3(0.0f, 1.0f, 2.0f)) == glm::detail::tvec3<uint8_t>(0, 1, 2));
    assert(sortDimensions(glm::vec3(0.0f, 2.0f, 1.0f)) == glm::detail::tvec3<uint8_t>(0, 2, 1));
    assert(sortDimensions(glm::vec3(1.0f, 0.0f, 2.0f)) == glm::detail::tvec3<uint8_t>(1, 0, 2));
    assert(sortDimensions(glm::vec3(1.0f, 2.0f, 0.0f)) == glm::detail::tvec3<uint8_t>(2, 0, 1));
    assert(sortDimensions(glm::vec3(2.0f, 0.0f, 1.0f)) == glm::detail::tvec3<uint8_t>(1, 2, 0));
    assert(sortDimensions(glm::vec3(2.0f, 1.0f, 0.0f)) == glm::detail::tvec3<uint8_t>(2, 1, 0));
}