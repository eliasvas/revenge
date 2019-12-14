#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
struct CircleCollider {
	float radius;
	glm::vec3 offset;
	CircleCollider(float radius, glm::vec3 offset) : radius(radius),offset(offset) {}
	void Update(float dt) {}
	void Collide(CircleCollider* c) {}
};
