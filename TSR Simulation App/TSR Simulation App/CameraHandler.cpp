#include "CameraHandler.h"

CameraHandler::CameraHandler(const CameraConfig& cameraConfig)
	: cameraPos(cameraConfig.Position), cameraFront(cameraConfig.Front), cameraUp(cameraConfig.Up), projection(cameraConfig.projection),
	yaw(cameraConfig.yaw), pitch(cameraConfig.pitch), sensitivity(cameraConfig.sensitivity), speed(cameraConfig.speed) {
}

void CameraHandler::moveFront(float dt) {
	cameraPos += glm::vec3(speed * dt) * cameraFront;
}

void CameraHandler::moveBack(float dt) {
	cameraPos -= glm::vec3(speed * dt) * cameraFront;
}

void CameraHandler::moveLeft(float dt) {
	cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * glm::vec3(speed * dt);
}

void CameraHandler::moveRight(float dt) {
	cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * glm::vec3(speed * dt);
}

void CameraHandler::lookUp(float dt) {
	pitch += sensitivity * dt;
}

void CameraHandler::lookDown(float dt) {
	pitch -= sensitivity * dt;
}

void CameraHandler::lookLeft(float dt) {
	yaw -= sensitivity * dt;
}

void CameraHandler::lookRight(float dt) {
	yaw += sensitivity * dt;
}

glm::mat4x4 CameraHandler::getView() {
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	return view;
}

glm::mat4x4 CameraHandler::getProjection() const {
	return projection;
}