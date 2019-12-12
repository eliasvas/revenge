#include "glm/glm.hpp"
#pragma once
struct CircleCollider {
	float radius;
	glm::mat4* position_of_object;
	glm::mat3 offset;
	CircleCollider(float radius, glm::mat4* position, glm::mat3 offset) : radius(radius), position_of_object(position), offset(offset) {}
};


bool checkCollision(CircleCollider* c1, CircleCollider* c2) {
	return false; //todo
}
