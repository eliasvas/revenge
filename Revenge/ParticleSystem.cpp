#include "ParticleSystem.h"
#include "GeometryNode.h"
#include "Tools.h"
#include <algorithm>
#include "ShaderProgram.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "OBJLoader.h"
#include "TextureManager.h"

// RENDERER
ParticleSwirl::ParticleSwirl()
{
	m_continous_time = 0.0;

}

ParticleSwirl::~ParticleSwirl()
{

}

bool ParticleSwirl::Init()
{
	m_particles_position.resize(100);
	m_particles_life.resize(100);
	for (int i = 0; i < m_particles_position.size(); ++i)
	{
		m_particles_position[i] = glm::vec3(0, 2, 0);
		m_particles_life[i] = (float)i;
	}


	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_particles_position.size() * sizeof(glm::vec3), &m_particles_position[0], GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,				// attribute index
		3,              // number of elements per vertex, here (x,y,z)
		GL_FLOAT,       // the type of each element
		GL_FALSE,       // take our values as-is
		0,		         // no extra data between each position
		0				// pointer to the C array or an offset to our buffer
	);

	//If there was any errors
	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		printf("Exiting with error at Renderer::Init\n");
		return false;
	}

	//If everything initialized
	return true;
}

void ParticleSwirl::Update(float dt)
{
	float movement_speed = 2.f;
	const glm::vec3 center(0, 4, 0);

	for (int i = 0; i < m_particles_position.size(); ++i)
	{
		m_particles_life[i] += dt;
		float life = m_particles_life[i];
		
		if (life > 2.f) 
		{
			float random_pos = rand() / (float)RAND_MAX;
			random_pos *= 2 * 3.14159f;
			m_particles_position[i] = 0.1f * glm::vec3(sin(random_pos), 0, cos(random_pos)) + center;
			m_particles_life[i] = rand() / (float)RAND_MAX;
		}
		else
		{
			float life = m_particles_life[i] + 0*m_continous_time;
			
			float random_pos = (i / 20) / 5.f;
			random_pos *= 2.f * 3.14159f;
			m_particles_position[i] = life * glm::vec3(sin(random_pos), 0, cos(random_pos)) + center;
			m_particles_position[i].x = cos(life) * sin(random_pos) + sin(life) * cos(random_pos);
			m_particles_position[i].y = 0.f;
			m_particles_position[i].z = -sin(life) * sin(random_pos) + cos(life) * cos(random_pos);
			m_particles_position[i] = m_particles_life[i] * m_particles_position[i] + center;

			glm::mat3 rot = glm::rotate(glm::mat4(1.f), -2*m_continous_time, glm::vec3(0, 1, 0));
			m_particles_position[i] = rot * m_particles_position[i];

		}

		if (m_particles_position[i].y > 10)
			m_particles_position[i].y = 1;
	}
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_particles_position.size() * sizeof(glm::vec3), &m_particles_position[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	m_continous_time += dt;
}

void ParticleSwirl::Render()
{
	glPointSize(10);
	glBindVertexArray(m_vao);
	glDrawArrays(GL_POINTS, 0, (GLsizei)m_particles_position.size());
}

ParticleEmitter::ParticleEmitter()
{
	m_continous_time = 0.0;
	m_vbo = 0;
	m_vao = 0;
}

ParticleEmitter::~ParticleEmitter()
{

}

bool ParticleEmitter::Init()
{
	m_particles_position.resize(120);
	m_particles_velocity.resize(120);
	m_particles_life.resize(120, 0.f);

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_particles_position.size() * sizeof(glm::vec3), &m_particles_position[0], GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,				// attribute index
		3,              // number of elements per vertex, here (x,y,z)
		GL_FLOAT,       // the type of each element
		GL_FALSE,       // take our values as-is
		0,		         // no extra data between each position
		0				// pointer to the C array or an offset to our buffer
	);

	//If there was any errors
	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		printf("Exiting with error at Renderer::Init\n");
		return false;
	}

	//If everything initialized
	return true;
}

void ParticleEmitter::Update(float dt)
{
	float movement_speed = 1.f;

	for (int i = 0; i < m_particles_position.size(); ++i)
	{
		if (m_particles_life[i] <= 0.f)
		{
			// the life will be a random value between [0.5 ... 1.0]
			m_particles_life[i] = 0.5f * rand() / float(RAND_MAX) + 0.5f;
			// Create a random velocity. XZ components of velocity will be random numbers between [0...1].
			m_particles_velocity[i] = glm::vec3(rand(), RAND_MAX, rand()) / float(RAND_MAX);
			// Change velocity (X,Y,Z) from [0...1] range to [-1...1]
			m_particles_velocity[i] = 2.f * m_particles_velocity[i] - 1.f;
			// Make the velocity cone smaller
			m_particles_velocity[i] *= 0.5f;
			m_particles_velocity[i].y = 1.f;
			// normalize the velocity vector
			m_particles_velocity[i] = glm::normalize(m_particles_velocity[i]);

			// we have 120 particles that will be emitted from 3 points.
			float pos = (i / 40) / 5.f * 3.14159f;
			// each emitter will be positioned on a circle with radius 1.5
			m_particles_position[i] = 1.5f * glm::vec3(sin(pos), 1, cos(pos));
		}
		else
		{
			// reduce the life
			m_particles_life[i] -= dt;
			// update the position using the velocity vector
			m_particles_position[i] += m_particles_velocity[i] * movement_speed * dt;
		}
	}

	// Reupload data to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_particles_position.size() * sizeof(glm::vec3), &m_particles_position[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	m_continous_time += dt;
}

void ParticleEmitter::Render()
{
	glPointSize(10);
	glBindVertexArray(m_vao);
	glDrawArrays(GL_POINTS, 0, (GLsizei)m_particles_position.size());
}
