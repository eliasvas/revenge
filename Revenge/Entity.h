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
#include "OBJLoader.h"
#include <iostream>
#include <vector>
#include "glm/gtx/string_cast.hpp"
#include "SDL2/SDL_timer.h"
#include "algorithm"
#include "tgmath.h"

struct Entity {
	CircleCollider* col;
	GeometryNode*								geometry;
	ShaderProgram*								m_geometry_rendering_program;
	glm::mat4									transformation_matrix;
	glm::mat4									transformation_normal_matrix;
	std::string tag;
	bool active = true;
	Entity(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, CircleCollider* col, std::string tag) {
		this->geometry = geometry;
		this->tag = tag;
		transformation_matrix = transform;
		transformation_normal_matrix = normal;
		this->col = col;
	}

	virtual void Render(ShaderProgram& shader) {
		if (!active)return;
		//if (transformation_matrix[3][2] < 0)return; //MUST REMOVE
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
	~Entity() {
	}
};

bool CheckCollision(Entity* g1, Entity* g2);

struct Pirate : public Entity {
	std::vector<glm::vec2> path;
	float speed = 1.0f;
	float time_elapsed = 0.0f;
	int tile = 0;
	static std::vector<Pirate*> pirates;
	Pirate(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, CircleCollider* col,std::string tag,std::vector<glm::vec2>& path): Entity(geometry,transform, normal,col,tag) {
		this->path = path;
		pirates.push_back(this);
	}
	//V should be removed
	Pirate(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, CircleCollider* col,std::string tag): Entity(geometry,transform, normal,col,tag) {
		pirates.push_back(this);
	}
	void Update(float dt,int speed = 1) {
		if (!active)return;
		time_elapsed += dt;
		if (time_elapsed > (float)speed) {
			//std::cout << "next tile" << std::endl;
			time_elapsed = 0.0f;
			transformation_matrix[3][0] = 2*path[tile+1].g;
			transformation_matrix[3][2] = 2*path[tile+1].r;
			tile++;
			if (tile < path.size() - 1) {
				//atan2(direction.y, -direction.x)
				float angle = atan2(path[tile+1].g - path[tile].g, path[tile+1].r - path[tile].r);
				transformation_matrix = glm::rotate(transformation_matrix,angle, glm::vec3(0, 1, 0));
			}
		}
		glm::vec3 start = glm::vec3(path[tile].g, 0, path[tile].r);
		glm::vec3 end;
		if (tile < path.size() - 1) //omg
			end = glm::vec3(path[tile + 1].g, 0, path[tile + 1].r);
		else {
			for (int i = 0; i < pirates.size(); ++i) {
				if (pirates[i] == this) {
					delete pirates[i];
					pirates[i] = NULL;
				}
			}

		}
		glm::vec3 offset = glm::mix(start, end, std::min(time_elapsed,1.0f));
		transformation_matrix[3][0] = 2*offset.x;
		transformation_matrix[3][2] = 2*offset.z;
	}
	~Pirate() {
	//	pirates.erase(this);
	}
};

struct CannonBall : public Entity {
	static std::vector<CannonBall*> balls;
	float life = 10.0f;
	glm::vec3 down = {0.0f, -1.5f, 0.0f};
	glm::vec3 dir;
	float active_time; //todo implement logic
	CannonBall(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, glm::vec3 dir,CircleCollider* col,std::string tag): Entity(geometry,transform, normal,col,tag), dir(dir) {
		CannonBall::balls.push_back(this);
	}
	void Update(float dt,int speed = 1) {
		if (!active)return;
		life -= dt;
		transformation_matrix = glm::translate(transformation_matrix, dir*dt*(float)speed*9.0f);
		transformation_matrix = glm::translate(transformation_matrix, down*dt*(float)speed*9.0f);

		for (Pirate* p : Pirate::pirates) {
			if (p == NULL)continue;
			//if (CheckCollision(this, (Entity*)p))printf("collision!\n");
			//else printf("no collision\n");
		}
		if (life < 0.0f) {
			for (int i = 0; i < balls.size(); ++i) {
				if (balls[i] == this) {
					delete balls[i];
					balls[i] = NULL;
				}
			}
		}
	}
	~CannonBall() {
//		balls.erase(this);
	}

};

struct Tower : public Entity {
	static std::vector<Tower*> towers;
	float rate;
	GeometryNode* ball_mesh;
	Tower(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, CircleCollider* col, std::string tag, GeometryNode* b_m) : Entity(geometry, transform, normal, col, tag) {
		rate = 0.0f;
		ball_mesh = b_m;
		towers.push_back(this);
	}
	void Update(float dt, int speed = 1) {
		if (!active)return;
		rate -= dt;
		//std::cout << towers.size()<< std::endl;
		for (Pirate* p : Pirate::pirates) {
				if (p == NULL)continue;
				if (CheckCollision(this, (Entity*)p) && rate < 0) { //HUGE bottleneck
					rate = 2.0f;
					CircleCollider* c= new CircleCollider(1.0f, glm::vec3(0, 6, 0));
					glm::vec3 p_pos = { p->transformation_matrix[3][0] ,p->transformation_matrix[3][1], p->transformation_matrix[3][2]};
					glm::vec3 pos = { this->transformation_matrix[3][0] ,this->transformation_matrix[3][1], this->transformation_matrix[3][2]};
					auto ballp = new CannonBall(ball_mesh,glm::scale(glm::translate(Entity::transformation_matrix,glm::vec3(0,9,0)), glm::vec3(0.7,0.7,0.7)),Entity::transformation_normal_matrix,(p_pos-pos),c,Entity::tag);
				}
		}
	}
	~Tower() {
	}
};

struct Treasure : public Entity {
	int last_pirate_to_collide = 0;
	Treasure(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, CircleCollider* col, std::string tag) : Entity(geometry, transform, normal, col, tag) {
	}
	void Update(float dt, int speed = 1) {
		for (Pirate* p : Pirate::pirates) {
				if (p == NULL)continue;
				if (CheckCollision(this, (Entity*)p) && last_pirate_to_collide != (int)p) { //HUGE bottleneck
					//currency -1
					last_pirate_to_collide = (int)p;
				}
		}
	}
	//void Render();
	~Treasure() {
	}
};

