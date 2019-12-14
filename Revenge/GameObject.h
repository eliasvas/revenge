#pragma once
#include <vector>
#include <string>
#include "ShaderProgram.h"
#include "SpotlightNode.h"
#include "ParticleSystem.h"
#include "GeometryNode.h"
#include "glm/glm.hpp"
#include "GLEW/glew.h"
#include "glm/gtc/type_ptr.hpp"
#include "CircleCollider.h"
#include <vector>


struct GameObject {
	static std::vector<GameObject*> objects;
	CircleCollider* col;
	GeometryNode*								geometry;
	ShaderProgram*								m_geometry_rendering_program;
	glm::mat4									transformation_matrix;
	glm::mat4									transformation_normal_matrix;
	std::string tag;
	bool active = true;
	GameObject(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, CircleCollider* col, std::string tag) {
		this->geometry = geometry;
		this->tag = tag;
		transformation_matrix = transform;
		transformation_normal_matrix = normal;
		this->col = col;
		objects.push_back(this); //push in GameObject array
	}

	virtual void Render(ShaderProgram& shader) {
		glBindVertexArray(geometry->m_vao);
		glUniformMatrix4fv(shader["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(transformation_matrix));
		glUniformMatrix4fv(shader["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(transformation_normal_matrix));
		for (int j = 0; j < geometry->parts.size(); j++)
		{
			glm::vec3 diffuseColor = geometry->parts[j].diffuseColor;
			glm::vec3 specularColor = geometry->parts[j].specularColor;
			float shininess = geometry->parts[j].shininess;

			glUniform3f(shader["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			glUniform3f(shader["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
			glUniform1f(shader["uniform_shininess"], shininess);
			glUniform1f(shader["uniform_has_texture"], (geometry->parts[j].textureID > 0) ? 1.0f : 0.0f);
			glBindTexture(GL_TEXTURE_2D, geometry->parts[j].textureID);

			glDrawArrays(GL_TRIANGLES, geometry->parts[j].start_offset, geometry->parts[j].count);
		}

	}
	virtual void Update(float dt, int speed = 1.0f) {};
};
bool CheckCollision(GameObject* g1, GameObject* g2);
struct Pirate : public GameObject {
	static std::vector<Pirate*> pirates;
	Pirate(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, CircleCollider* col,std::string tag): GameObject(geometry,transform, normal,col,tag) {
		pirates.push_back(this);
	}
	void Update(float dt,int speed = 1) {
		transformation_matrix = glm::translate(transformation_matrix, glm::vec3(0, 0, -0)*dt*(float)speed);
	}
};

struct CannonBall : public GameObject {
	static std::vector<CannonBall*> balls;
	glm::vec3 dir;
	float active_time; //tpdp implement logic
	CannonBall(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, glm::vec3 dir,CircleCollider* col,std::string tag): GameObject(geometry,transform, normal,col,tag), dir(dir) {
		CannonBall::balls.push_back(this);
	}
	void Update(float dt,int speed = 1) {
		transformation_matrix = glm::translate(transformation_matrix, dir*dt*(float)speed);
		for (Pirate* p : Pirate::pirates) {
			if (p == NULL)continue;
			if (CheckCollision(this, (GameObject*)p))printf("collision!\n");
			else printf("no collision\n");
		}
	}
};
