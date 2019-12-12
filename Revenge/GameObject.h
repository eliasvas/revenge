#include <vector>
#include <string>
#include "ShaderProgram.h"
#include "SpotlightNode.h"
#include "ParticleSystem.h"
#include "GeometryNode.h"
#include "glm/glm.hpp"
#include "GLEW/glew.h"
#include "glm/gtc/type_ptr.hpp"
#pragma once
struct GameObject {
	GeometryNode*								geometry;
	ShaderProgram*								m_geometry_rendering_program;
	glm::mat4									transformation_matrix;
	glm::mat4									transformation_normal_matrix;
	std::string tag;
	GameObject(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal,std::string tag);
	virtual void Render(ShaderProgram& shader);
	virtual void Update(float dt, int speed = 1.0f);
};
GameObject::GameObject(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, std::string tag = "") {
	this->geometry = geometry;
	this->tag = tag;
	transformation_matrix = transform;
	transformation_normal_matrix = normal;

}
void GameObject::Update(float dt, int speed) {
}
void GameObject::Render(ShaderProgram& shader) {
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

struct Pirate : public GameObject {
	Pirate(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, std::string tag): GameObject(geometry,transform, normal,tag) {}
	void Update(float dt,int speed = 1) {
		transformation_matrix = glm::translate(transformation_matrix, glm::vec3(0, 0, -3)*dt*(float)speed);
	}
};


