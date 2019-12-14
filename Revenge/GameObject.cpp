#include "GameObject.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>


std::vector<CannonBall*> CannonBall::balls;
std::vector<Pirate*> Pirate::pirates;
std::vector<GameObject*> GameObject::objects;

bool CheckCollision(GameObject* g1, GameObject* g2) {
	if (g1 == NULL || g2 == NULL)return false;
	glm::vec3 g1_pos = { g1->transformation_matrix[3][0] + g1->col->offset[0], g1->transformation_matrix[3][1] + g1->col->offset[1], g1->transformation_matrix[3][2] + g1->col->offset[2] };
	glm::vec3 g2_pos = { g2->transformation_matrix[3][0] + g2->col->offset[0], g2->transformation_matrix[3][1] + g2->col->offset[1], g2->transformation_matrix[3][2] + g2->col->offset[2] };
	std::cout << "g1_pos = "<< glm::to_string(g1_pos) << ", g2_pos = " << glm::to_string(g2_pos)<< std::endl;
	if (g1->col->radius > sqrt((g2_pos.x - g1_pos.x)*(g2_pos.x - g1_pos.x)+(g2_pos.y-g1_pos.y)*(g2_pos.y - g1_pos.y)+(g2_pos.z - g1_pos.z)*(g2_pos.z - g1_pos.z))) {
		return true;
	}
	return false;


}