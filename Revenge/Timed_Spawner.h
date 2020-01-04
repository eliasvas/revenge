#pragma once
#include "Entity.h"
#include <filesystem>
#include <assert.h>

struct Timed_Spawner {
	float time= 0.0f;
	float interval;
	int spawn_instances;
	float spawn_offset;
	float spawn_queue= 0.0f;
	Pirate* base;
	Timed_Spawner(float interval, int spawn_instances, float spawn_offset, Pirate* base) {
		assert(base != NULL);
		this->interval = interval;
		time = interval;
		this->spawn_instances = spawn_instances;
		this->spawn_offset = spawn_offset;
		this->base = base;
		base->active = false;
	}
	void Update(float dt) {
		if (spawn_queue > 0.f) { //<- there is something in the queue
			//put the right component in constructor down
			if ((int)(spawn_queue - dt) != (int)spawn_queue)Entity* skeleton_no_anim = new Pirate(base->geometry, base->geometry_arm, base->geometry_lleg, base->geometry_rleg, base->transformation_matrix,base->transformation_normal_matrix, new CircleCollider(1.0f, glm::vec3(0, 0.5, 0)), "pirate", base->path);
			spawn_queue-=dt;
		}
		time += dt;
		if (time - interval > 0.0f) {
			//SpawnWave();
			spawn_queue = spawn_instances;
			time = 0.0f;
		}
	}
	~Timed_Spawner() {}
};
