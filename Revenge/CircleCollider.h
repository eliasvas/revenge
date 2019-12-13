#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "GameObject.h"
#pragma once
struct CircleCollider {
	float radius;
	bool active = true;
	glm::vec3 offset;
	GameObject* g;
	CircleCollider(float radius, GameObject* g, glm::vec3 offset) : radius(radius),g(g), offset(offset) {}
	void Update(float dt);
	void Collide(CircleCollider* c);
};
void CircleCollider::Update(float dt) {
	if (g == NULL)active = false;
}
void CircleCollider::Collide(CircleCollider* c) {
	printf("collision!!!!\n");
}
bool CheckCollision(CircleCollider* c1, CircleCollider* c2) {
	if (c1 == NULL || c2 == NULL)return false;
	glm::vec3 c1_pos = { c1->g->transformation_matrix[3][0] + c1->offset[0], c1->g->transformation_matrix[3][0] + c1->offset[1], c1->g->transformation_matrix[3][2] + c1->offset[2] };
	glm::vec3 c2_pos = { c2->g->transformation_matrix[3][0] + c2->offset[0], c2->g->transformation_matrix[3][0] + c2->offset[1], c2->g->transformation_matrix[3][2] + c2->offset[2] };
	if (c1->radius > sqrt((c1_pos.x - c2_pos.x)*(c1_pos.x - c2_pos.x)+(c2_pos.y-c2_pos.y)*(c1_pos.y - c2_pos.y)+(c1_pos.z - c2_pos.z)*(c1_pos.z - c2_pos.z))) {
		return true;
	}
	return false;
}
