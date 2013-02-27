#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

#include <cassert>
#include "tests.h"
#include "Logging/logging.h"
#include "Util/Geometry/geomUtil.h"

void testTransform() {
	for(unsigned int testRun = 0; testRun < 10; testRun++) {
		glm::vec3 translate = glm::linearRand(glm::vec3(-100.0f), glm::vec3(100.0f));
		glm::quat rotate = glm::quat_cast(directionToMat3(glm::normalize(glm::linearRand(glm::vec3(-1.0f), glm::vec3(1.0f))), 
			glm::normalize(glm::linearRand(glm::vec3(-1.0f), glm::vec3(1.0f)))));
		glm::vec3 scale = glm::linearRand(glm::vec3(-2.0f), glm::vec3(2.0f));

		glm::mat4 transform = glm::translate(translate) * glm::mat4_cast(rotate) * glm::scale(scale);

		glm::vec3 translate2 = getTransformPosition(transform);
		glm::quat rotate2;
		glm::vec3 scale2;

		getTransformRotationScale(transform, rotate2, scale2);
		
		glm::mat4 transform2 = glm::translate(translate2) * glm::mat4_cast(rotate2) * glm::scale(scale2);

		assert(eqMat4(transform, transform2));

		/*assert(eqVec(translate, translate2));
		assert(eqQuat(rotate, rotate2));
		assert(eqVec(scale, scale2));*/
	}
}

void testGeomUtil() {
	testTransform();
}