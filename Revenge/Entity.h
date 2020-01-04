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
	Entity() {}
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
	GeometryNode* geometry_arm;
	GeometryNode* geometry_lleg;
	GeometryNode* geometry_rleg;
	std::vector<glm::vec2> path;
	float time = 0.0f;	
	float time_elapsed = 0.0f;
	int tile = 0;
	int life = 2;


	//walking animation 

	float x = 0;
	float y = 0;
	float rl = 0;
	float fl = 0;
	float b = 0;
	
	//limit values
	bool hand = true;
	bool leg = true;
	bool body = true;

	static std::vector<Pirate*> pirates;
	Pirate(GeometryNode* geometry, GeometryNode* geometry_arm, GeometryNode* geometry_lleg, GeometryNode* geometry_rleg, glm::mat4 transform, glm::mat4 normal,CircleCollider* col, std::string tag, std::vector<glm::vec2>& path) : Entity(geometry, transform, normal, col,tag) {
			
		this->geometry_arm = geometry_arm;
		this->geometry_lleg = geometry_lleg;
		this->geometry_rleg = geometry_rleg;
		this->path = path;
		transformation_matrix = glm::translate(transformation_matrix, glm::vec3(0, -2, 0));
		pirates.push_back(this);
	}
	void Update(float dt, int speed = 1) {
		if (!active)return;
		time += dt;
		if (life < 0) {
			active = false;//must destroy but wtv
			
		}
		time_elapsed += dt;
		if (time_elapsed > (float)speed) {
			//std::cout << "next tile" << std::endl;
			time_elapsed = 0.0f;
			transformation_matrix[3][0] = 2*path[tile+1].g;
			transformation_matrix[3][2] = 2*path[tile+1].r;
			tile++;
			if (tile < path.size() - 1 && false) {
				//atan2(direction.y, -direction.x)
				float angle = atan2(path[tile+1].g - path[tile].g, path[tile].r - path[tile+1].r) + glm::radians(90.0f);
				transformation_matrix = glm::rotate(transformation_matrix,angle, glm::vec3(0, 1, 0));
			}
		}
		glm::vec3 start = glm::vec3(path[tile].g, 0, path[tile].r);
		glm::vec3 end;
		if (tile < path.size() - 1 && active) //omg
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


		if (true) {
			//transformation_matrix = glm::translate(transformation_matrix, glm::vec3(0, 0, -6) * dt * (float)speed);
			if (rl >= 25 || rl <= -25) {
				leg = !leg;
			}
			if (x >= 45 || x <= 0) {
				hand = !hand;
			}
			if (b >= 4 || b <= -4) {
				body = !body;
			}

			if (hand) {
				x = x + dt * 32;
				y = y + dt * 1;
			}
			else {
				x = x - dt * 32;
				y = y - dt * 1;
			}

			if (leg == true) {
				rl = rl + dt * 32;
				fl = fl + dt * 1;
			}
			else {
				rl = rl - dt * 32;
				fl = fl - dt * 1;
			}

			if (body == true) {
				b = b + dt * 8;
			}
			else {
				b = b - dt * 8;
			}

		}
	}

	void Render(ShaderProgram& shader) {
		if (!active)return;
	
		glBindVertexArray(geometry->m_vao);
		glUniformMatrix4fv(shader["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::rotate(transformation_matrix,glm::radians(b),glm::vec3(1,0,0))));
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



		
		glBindVertexArray(geometry_arm->m_vao);
		glUniformMatrix4fv(shader["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::translate(transformation_matrix, glm::vec3(5, 10.5+y,-y))*glm::rotate(glm::mat4(1.0f),glm::radians(x),glm::vec3(1,0,0))));
		glUniformMatrix4fv(shader["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(transformation_normal_matrix));
		for (int j = 0; j < geometry_arm->parts.size(); j++)
		{
			glm::vec3 diffuseColor = geometry_arm->parts[j].diffuseColor;
			glm::vec3 specularColor = geometry_arm->parts[j].specularColor;
			float shininess = geometry_arm->parts[j].shininess;

			glUniform3f(shader["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			glUniform3f(shader["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
			glUniform1f(shader["uniform_shininess"], shininess);
			glUniform1f(shader["uniform_has_texture"], (geometry_arm->parts[j].textureID > 0) ? 1.0f : 0.0f);
			glBindTexture(GL_TEXTURE_2D, geometry_arm->parts[j].textureID);

			glDrawArrays(GL_TRIANGLES, geometry_arm->parts[j].start_offset, geometry_arm->parts[j].count);
		}

		


		glBindVertexArray(geometry_lleg->m_vao);
		glUniformMatrix4fv(shader["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr((glm::translate(transformation_matrix, glm::vec3(-4, 1+fl/2, -3+fl))) * glm::rotate(glm::mat4(1.0f), glm::radians(-rl), glm::vec3(1,0,0))));
		glUniformMatrix4fv(shader["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(transformation_normal_matrix));
		for (int j = 0; j < geometry_lleg->parts.size(); j++)
		{
			glm::vec3 diffuseColor = geometry_lleg->parts[j].diffuseColor;
			glm::vec3 specularColor = geometry_lleg->parts[j].specularColor;
			float shininess = geometry_lleg->parts[j].shininess;

			glUniform3f(shader["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			glUniform3f(shader["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
			glUniform1f(shader["uniform_shininess"], shininess);
			glUniform1f(shader["uniform_has_texture"], (geometry_lleg->parts[j].textureID > 0) ? 1.0f : 0.0f);
			glBindTexture(GL_TEXTURE_2D, geometry_lleg->parts[j].textureID);

			glDrawArrays(GL_TRIANGLES, geometry_lleg->parts[j].start_offset, geometry_lleg->parts[j].count);
		}

		glBindVertexArray(geometry_rleg->m_vao);
		glUniformMatrix4fv(shader["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr((glm::translate(transformation_matrix, glm::vec3(2.5, 1-fl/2, -3-fl))) * glm::rotate(glm::mat4(1.0f), glm::radians(rl), glm::vec3(1,0,0))));
		glUniformMatrix4fv(shader["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(transformation_normal_matrix));
		for (int j = 0; j < geometry_rleg->parts.size(); j++)
		{
			glm::vec3 diffuseColor = geometry_rleg->parts[j].diffuseColor;
			glm::vec3 specularColor = geometry_rleg->parts[j].specularColor;
			float shininess = geometry_rleg->parts[j].shininess;

			glUniform3f(shader["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			glUniform3f(shader["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
			glUniform1f(shader["uniform_shininess"], shininess);
			glUniform1f(shader["uniform_has_texture"], (geometry_rleg->parts[j].textureID > 0) ? 1.0f : 0.0f);
			glBindTexture(GL_TEXTURE_2D, geometry_rleg->parts[j].textureID);
			glDrawArrays(GL_TRIANGLES, geometry_rleg->parts[j].start_offset, geometry_rleg->parts[j].count);
		}

	}
};

struct CannonBall : public Entity {
	static std::vector<CannonBall*> balls;
	glm::vec3 down = {0.0f, -1.5f, 0.0f};
	glm::vec3 dir;
	int last_pirate_to_collide = 0;
	float active_time; //todo implement logic
	CannonBall(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, glm::vec3 dir,CircleCollider* col,std::string tag): Entity(geometry,transform, normal,col,tag), dir(dir) {
		CannonBall::balls.push_back(this);
	}
	void Update(float dt,int speed = 1) {
		if (!active)return;
		transformation_matrix = glm::translate(transformation_matrix, dir*dt*(float)speed*30.0f); //find a true relation in terms of speed
		transformation_matrix = glm::translate(transformation_matrix, down*dt*(float)speed*30.0f);

		for (Pirate* p : Pirate::pirates) {
				if (p == NULL)continue;
				if (CheckCollision(this, (Entity*)p) && last_pirate_to_collide != (int)p) { //HUGE bottleneck
					last_pirate_to_collide = (int)p;
					//std::cout << "Collision!!" << std::endl;
					p->life--;
				}
		}

		if (transformation_matrix[3][1] < -1) {
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
				if (CheckCollision(this, (Entity*)p) && rate < 0&& p->active) { //HUGE bottleneck
					rate = 2.0f;
					CircleCollider* c= new CircleCollider(0.5f, glm::vec3(0, 0, 0));
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
	int money = 1000;
	Treasure(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, CircleCollider* col, std::string tag) : Entity(geometry, transform, normal, col, tag) {
	}
	void Update(float dt, int speed = 1) {
		//printf("%i\n", money);
		if (money <= 0) {
			printf("Game Over!");
			exit(0);
		}
		for (Pirate* p : Pirate::pirates) {
				if (p == NULL)continue;
				if (CheckCollision(this, (Entity*)p) && last_pirate_to_collide != (int)p) { //HUGE bottleneck
					money -= 100;
					last_pirate_to_collide = (int)p;
				}
		}
	}
	//void Render();
	~Treasure() {
	}
};

