//
//  Camera.cpp
//  Lab5
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Camera.hpp"

namespace gps {
	//FILE* file = fopen("C:/Users/edina/42/GPS/proiect/OpenGL/startPos.txt", "w+");

    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget)
    {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
    
	glm::vec3 Camera::getCameraTarget()
	{
		return cameraTarget;
	}

    glm::mat4 Camera::getViewMatrix()
    {
        return glm::lookAt(cameraPosition, cameraPosition + cameraDirection , glm::vec3(0.0f, 1.0f, 0.0f));
    }

	void Camera::setCameraPosition(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraDirection)
	{
		this->cameraPosition = cameraPosition;
		this->cameraTarget = cameraTarget;
		this->cameraDirection = cameraDirection;
		this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
	}

	glm::vec3 Camera::getPosition()
	{
		return cameraPosition;
	}

	glm::vec3 Camera::getDirection()
	{
		return cameraDirection;
	}

	float Camera::isCameraInsideBoundingBox(MOVE_DIRECTION direction, float speed)
	{
		glm::vec3 point = getCameraNewPosition(direction, speed);
		for (gps::BoundingBox box : boundingBoxes) {
			if ((point.x >= box.min.x && point.x <= box.max.x) &&
				(point.y >= box.min.y && point.y <= box.max.y) &&
				(point.z >= box.min.z && point.z <= box.max.z))
				return true;
		}
		return false;
	}
    
	glm::vec3 Camera::getCameraNewPosition(MOVE_DIRECTION direction, float speed) {
		switch (direction) {
			case MOVE_FORWARD:
				return cameraPosition + cameraDirection * speed;
				
			case MOVE_BACKWARD:
				return cameraPosition - cameraDirection * speed;
				
			case MOVE_RIGHT:
				return cameraPosition + cameraRightDirection * speed;

			case MOVE_LEFT:
				return cameraPosition - cameraRightDirection * speed;
		}
	}

    void Camera::move(MOVE_DIRECTION direction, float speed)
    {
		if (!isCameraInsideBoundingBox(direction, speed)) {
			cameraPosition = getCameraNewPosition(direction, speed);
			//fprintf(file, "%s %f, %f, %f\n", "position", cameraPosition.x, cameraPosition.y, cameraPosition.z);
			//fprintf(file, "%s %f, %f, %f\n", "target", cameraTarget.x, cameraTarget.y, cameraTarget.z);
		}
    }
    
	/*
	* pitch - rotation up and down - around x
	* yaw - rotation left and right - around y 
	*/
    void Camera::rotate(float pitch, float yaw)  
    {
		cameraDirection.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		cameraDirection.y = sin(glm::radians(pitch));
		cameraDirection.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

		cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
		//fprintf(file, "%s %f, %f, %f\n", "direction", cameraDirection.x, cameraDirection.y, cameraDirection.z);
    }
}
