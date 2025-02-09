//
//  Camera.hpp
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#ifndef Camera_hpp
#define Camera_hpp

#include <stdio.h>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

namespace gps {

	enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT };
	typedef struct BoundingBox {
		glm::vec3 min;
		glm::vec3 max;

		BoundingBox()
			: min(2147483648.0f), max(-2147483648.0f) { };
	}BoundingBox;

	class Camera
	{
	public:
		Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget);
		glm::vec3 getCameraTarget();
		glm::mat4 getViewMatrix();
		void move(MOVE_DIRECTION direction, float speed);
		void rotate(float pitch, float yaw);
		BoundingBox boundingBoxes[100];
		void setCameraPosition(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraDirection);
		glm::vec3 getPosition();
		glm::vec3 getDirection();
	private:
		glm::vec3 cameraPosition;
		glm::vec3 cameraTarget;
		glm::vec3 cameraDirection;
		glm::vec3 cameraRightDirection;
		const float YAW = -90.0f;
		const float PITCH = 0.0f;
		float isCameraInsideBoundingBox(MOVE_DIRECTION direction, float speed);
		glm::vec3 getCameraNewPosition(MOVE_DIRECTION direction, float speed);
	};
}
#endif /* Camera_hpp */
