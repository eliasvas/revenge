#include "GameObject.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>


std::vector<CannonBall*> CannonBall::balls;
std::vector<Pirate*> Pirate::pirates;
std::vector<Tower*> Tower::towers;

bool CheckCollision(GameObject* g1, GameObject* g2) {
	if (g1 == NULL || g2 == NULL)return false;
	glm::vec3 g1_pos = { g1->transformation_matrix[3][0] + g1->col->offset[0], g1->transformation_matrix[3][1] + g1->col->offset[1], g1->transformation_matrix[3][2] + g1->col->offset[2] };
	glm::vec3 g2_pos = { g2->transformation_matrix[3][0] + g2->col->offset[0], g2->transformation_matrix[3][1] + g2->col->offset[1], g2->transformation_matrix[3][2] + g2->col->offset[2] };
	float l = glm::length(g2_pos - g1_pos);
	//std::cout << "radius = "<< g1->col->radius << ", l = " << l << std::endl;

	if (g1->col->radius > l) {
		return true;
	}
	return false;

}