#pragma once
#include "Tools.h"
#include <vector>
#include <string>
#include "ShaderProgram.h"
#include "SpotlightNode.h"
#include "ParticleSystem.h"
#include "GeometryNode.h"
#include "glm/glm.hpp"
#include "GLEW/glew.h"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "CircleCollider.h"
#include "OBJLoader.h"
#include <iostream>
#include <vector>
#include "glm/gtx/string_cast.hpp"
#include "SDL2/SDL_timer.h"
#include "algorithm"
#include "tgmath.h"

enum {
	UP = 0,
	LEFT,
	DOWN,
	RIGHT,
};

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
		for (i32 j = 0; j < geometry->parts.size(); j++)
		{
			glm::vec3 diffuseColor = geometry->parts[j].diffuseColor;
			glm::vec3 specularColor = geometry->parts[j].specularColor;
			f32 shininess = geometry->parts[j].shininess;

			glUniform3f(shader["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			glUniform3f(shader["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
			glUniform1f(shader["uniform_shininess"], shininess);
			glUniform1f(shader["fade_alpha"], 1.0f);
			glUniform1f(shader["uniform_has_texture"], (geometry->parts[j].textureID > 0) ? 1.0f : 0.0f);
			glBindTexture(GL_TEXTURE_2D, geometry->parts[j].textureID);

			glDrawArrays(GL_TRIANGLES, geometry->parts[j].start_offset, geometry->parts[j].count);
		}

	}
	virtual void Update(f32 dt, i32 speed = 1.0f) {};
	~Entity() {
	}
};

bool CheckCollision(Entity* g1, Entity* g2);

struct Pirate : public Entity {
	GeometryNode* geometry_arm;
	GeometryNode* geometry_lleg;
	GeometryNode* geometry_rleg;
	i32 facing_direction = 0;
	std::vector<glm::vec2> path;
	f32 time = 0.0f;	
	f32 time_elapsed = 0.0f;
	i32 tile = 0;
	i32 life = 2;


	//walking animation 

	f32 x = 0;
	f32 y = 0;
	f32 rl = 0;
	f32 fl = 0;
	f32 b = 0;
	
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
	void Update(f32 dt, i32 speed = 1) {
		if (!active)return;
		time += dt;
		if (life <= 0)active = false;//must destroy but wtv
		time_elapsed += dt;
		if (time_elapsed > (f32)speed) {
			time_elapsed = 0.0f;
			tile++;

			if (tile != 0 && tile < path.size() - 1) {
				//turning is handled at rendering
				glm::vec3 current_direction = glm::vec3(path[tile+1].r,0,path[tile+1].g) - glm::vec3(path[tile].r,0,path[tile].g);
				if (std::abs(current_direction.z - 1.f) < 0.1f)
					facing_direction = UP;
				else if (current_direction.z < -1.f * 0.1f)
					facing_direction = DOWN;
				else if (current_direction.x < -1.f * 0.1f)
					facing_direction = RIGHT;
				else
					facing_direction = LEFT;

			}
		}
		if (tile < path.size() - 1 && active) {
			glm::vec3 start = glm::vec3(path[tile].r, 0, path[tile].g);
			glm::vec3 end = glm::vec3(path[tile + 1].r, 0, path[tile + 1].g);
			glm::vec3 offset = glm::mix(start, end, std::min(time_elapsed, 1.0f));
			transformation_matrix[3][0] = 2 * offset.x;
			transformation_matrix[3][2] = 2 * offset.z;
		}else {
			for (i32 i = 0; i < pirates.size(); ++i) {
				if (pirates[i] == this) {
					delete pirates[i];
					pirates[i] = NULL;
				}
			}

		}
	

		if (true) {
			//transformation_matrix = glm::translate(transformation_matrix, glm::vec3(0, 0, -6) * dt * (f32)speed);
			if (rl >= 25 || rl <= -25) {
				leg = !leg;
			}
			if (x >= 45 || x <= 0) {
				hand = !hand;
			}
			if (b >= 5 || b <= -5) {
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
				b = b + dt * 16;
			}
			else {
				b = b - dt * 16;
			}

		}
	}

	void Render(ShaderProgram& shader) {
		if (!active)return;
		transformation_matrix = glm::rotate(transformation_matrix,facing_direction*((f32)M_PI/2), glm::vec3(0,1,0));
		glBindVertexArray(geometry->m_vao);
		glUniformMatrix4fv(shader["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::rotate(transformation_matrix,glm::radians(b),glm::vec3(0,0,1))));
		glUniformMatrix4fv(shader["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(transformation_normal_matrix));
		for (i32 j = 0; j < geometry->parts.size(); j++)
		{
			glm::vec3 diffuseColor = geometry->parts[j].diffuseColor;
			glm::vec3 specularColor = geometry->parts[j].specularColor;
			f32 shininess = geometry->parts[j].shininess;

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
		for (i32 j = 0; j < geometry_arm->parts.size(); j++)
		{
			glm::vec3 diffuseColor = geometry_arm->parts[j].diffuseColor;
			glm::vec3 specularColor = geometry_arm->parts[j].specularColor;
			f32 shininess = geometry_arm->parts[j].shininess;

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
		for (i32 j = 0; j < geometry_lleg->parts.size(); j++)
		{
			glm::vec3 diffuseColor = geometry_lleg->parts[j].diffuseColor;
			glm::vec3 specularColor = geometry_lleg->parts[j].specularColor;
			f32 shininess = geometry_lleg->parts[j].shininess;

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
		for (i32 j = 0; j < geometry_rleg->parts.size(); j++)
		{
			glm::vec3 diffuseColor = geometry_rleg->parts[j].diffuseColor;
			glm::vec3 specularColor = geometry_rleg->parts[j].specularColor;
			f32 shininess = geometry_rleg->parts[j].shininess;

			glUniform3f(shader["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			glUniform3f(shader["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
			glUniform1f(shader["uniform_shininess"], shininess);
			glUniform1f(shader["uniform_has_texture"], (geometry_rleg->parts[j].textureID > 0) ? 1.0f : 0.0f);
			glBindTexture(GL_TEXTURE_2D, geometry_rleg->parts[j].textureID);
			glDrawArrays(GL_TRIANGLES, geometry_rleg->parts[j].start_offset, geometry_rleg->parts[j].count);
		}

		transformation_matrix = glm::rotate(transformation_matrix,(-1.f)*facing_direction*((f32)M_PI/2), glm::vec3(0,1,0));
	}
};

struct CannonBall : public Entity {
	static std::vector<CannonBall*> balls;
	glm::vec3 down = {0.0f, -1.5f, 0.0f};
	glm::vec3 dir;
	i32 last_pirate_to_collide = 0;
	f32 active_time; //todo implement logic
	CannonBall(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, glm::vec3 dir,CircleCollider* col,std::string tag): Entity(geometry,transform, normal,col,tag), dir(dir) {
		CannonBall::balls.push_back(this);
	}
	void Update(f32 dt,i32 speed = 1) {
		if (!active)return;
		transformation_matrix = glm::translate(transformation_matrix, dir*dt*(f32)speed*30.0f); //find a true relation in terms of speed
		transformation_matrix = glm::translate(transformation_matrix, down*dt*(f32)speed*30.0f);

		for (Pirate* p : Pirate::pirates) {
				if (p == NULL)continue;
				if (CheckCollision(this, (Entity*)p) && last_pirate_to_collide != (i32)p) { //HUGE bottleneck
					last_pirate_to_collide = (i32)p;
					//std::cout << "Collision!!" << std::endl;
					p->life--;
				}
		}

		if (transformation_matrix[3][1] < -2) {
			for (i32 i = 0; i < balls.size(); ++i) {
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
struct Meteor : public Entity {
	static std::vector<Meteor*> meteors;
	glm::vec3 down = {0.0f, -1.5f, 0.0f};
	glm::vec3 dir;
	i32 last_pirate_to_collide = 0;
	f32 active_time; //todo implement logic
	Meteor(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, glm::vec3 dir,CircleCollider* col,std::string tag): Entity(geometry,transform, normal,col,tag), dir(dir) {
		Meteor::meteors.push_back(this);
	}
	void Update(f32 dt,i32 speed = 1) {
		if (!active)return;
		transformation_matrix = glm::translate(transformation_matrix, dir*dt*(f32)speed*30.0f); //find a true relation in terms of speed

		for (Pirate* p : Pirate::pirates) {
				if (p == NULL)continue;
				if (CheckCollision(this, (Entity*)p) && last_pirate_to_collide != (i32)p) { //HUGE bottleneck
					last_pirate_to_collide = (i32)p;
					//std::cout << "Collision!!" << std::endl;
					p->life= 0;
				}
		}

		if (transformation_matrix[3][1] < -2) {
			for (i32 i = 0; i < meteors.size(); ++i) {
				if (meteors[i] == this) {
					delete meteors[i];
					meteors[i] = NULL;
				}
			}
		}
	}
	~Meteor() {
//		balls.erase(this);
	}

};

struct Tower : public Entity {
	static std::vector<Tower*> towers;
	f32 rate;
	GeometryNode* ball_mesh;
	f32 local_time = 0.0f;
	glm::vec3 color;
	Tower(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, CircleCollider* col, std::string tag, GeometryNode* b_m) : Entity(geometry, transform, normal, col, tag) {
		rate = 0.0f;
		ball_mesh = b_m;
		color = glm::vec3(0.5f);
		towers.push_back(this);
	}
	void Update(f32 dt, i32 speed = 1) {
		local_time += dt;
		if (!active|| local_time < 1.0f)return;
		rate -= dt;
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
	void Render(ShaderProgram& shader) {
		if (!active)return;
		//if (transformation_matrix[3][2] < 0)return; //MUST REMOVE
		glBindVertexArray(geometry->m_vao);
		glUniformMatrix4fv(shader["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(transformation_matrix));
		glUniformMatrix4fv(shader["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(transformation_normal_matrix));
		for (i32 j = 0; j < geometry->parts.size(); j++)
		{
			glm::vec3 diffuseColor = geometry->parts[j].diffuseColor;
			glm::vec3 specularColor = geometry->parts[j].specularColor;
			f32 shininess = geometry->parts[j].shininess;

			glUniform3f(shader["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			glUniform3f(shader["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
			glUniform1f(shader["uniform_shininess"], shininess);
			glUniform3f(shader["color"], color.r, color.g, color.b);
			glUniform1f(shader["fade_alpha"], (float)std::min(1.0f, local_time));
			glUniform1f(shader["uniform_has_texture"], (geometry->parts[j].textureID > 0) ? 1.0f : 0.0f);
			glBindTexture(GL_TEXTURE_2D, geometry->parts[j].textureID);

			glDrawArrays(GL_TRIANGLES, geometry->parts[j].start_offset, geometry->parts[j].count);
		}

	}

	~Tower() {
	}
};

struct Treasure : public Entity {
	i32 last_pirate_to_collide = 0;
	i32 money = 1000;
	Treasure(GeometryNode* geometry, glm::mat4 transform, glm::mat4 normal, CircleCollider* col, std::string tag) : Entity(geometry, transform, normal, col, tag) {
	}
	void Update(f32 dt, i32 speed = 1) {
		//pri32f("%i\n", money);
		if (money <= 0) {
			printf("Game Over!");
			exit(0);
		}
		for (Pirate* p : Pirate::pirates) {
				if (p == NULL)continue;
				if (CheckCollision(this, (Entity*)p) && last_pirate_to_collide != (i32)p) { //HUGE bottleneck
					money -= 100;
					last_pirate_to_collide = (i32)p;
				}
		}
	}
	//void Render();
	~Treasure() {
	}
};

