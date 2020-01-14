#pragma once
#include "Entity.h"
#include <filesystem>
#include <assert.h>

struct Timed_Spawner {
	f32 time= 0.0f;
	f32 i32erval;
	i32 spawn_instances;
	f32 spawn_offset;
	f32 spawn_queue= 0.0f;
	Pirate* base;
	Timed_Spawner(f32 i32erval, i32 spawn_instances, f32 spawn_offset, Pirate* base) {
		assert(base != NULL);
		this->i32erval = i32erval;
		time = i32erval;
		this->spawn_instances = spawn_instances;
		this->spawn_offset = spawn_offset;
		this->base = base;
		base->active = false;
	}
	void Update(f32 dt) {
		if (spawn_queue > 0.f) { //<- there is something in the queue
			//put the right component in constructor down
			if ((i32)(spawn_queue - dt) != (i32)spawn_queue)Entity* skeleton_no_anim = new Pirate(base->geometry, base->geometry_arm, base->geometry_lleg, base->geometry_rleg, base->transformation_matrix,base->transformation_normal_matrix, new CircleCollider(1.0f, glm::vec3(0, 0.5, 0)), "pirate", base->path);
			spawn_queue-=dt;
		}
		time += dt;
		if (time - i32erval > 0.0f) {
			//SpawnWave();
			spawn_queue = spawn_instances;
			time = 0.0f;
		}
	}
	~Timed_Spawner() {}
};
