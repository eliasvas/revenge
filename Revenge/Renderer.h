#ifndef BIM_ENGINE_RENDERER_H
#define BIM_ENGINE_RENDERER_H

#include "GLEW\glew.h"
#include "glm\glm.hpp"
#include <vector>
#include "ShaderProgram.h"
#include "SpotlightNode.h"
#include "ParticleSystem.h"

class Renderer
{
public:
	enum RENDERING_MODE
	{
		TRIANGLES,
		LINES,
		POINTS
	};

protected:
	int												m_screen_width, m_screen_height;
	glm::mat4										m_view_matrix;
	glm::mat4										m_projection_matrix;
	glm::vec3										m_camera_position;
	glm::vec3										m_camera_target_position;
	glm::vec3										m_camera_up_vector;
	glm::vec2										m_camera_movement;
	glm::vec2										m_camera_look_angle_destination;

	// skeleton movement direction
	glm::vec2										m_skeleton_movement;
	
	float m_continous_time;

	// Rendering Mode
	RENDERING_MODE m_rendering_mode;

	// Lights
	SpotLightNode m_spotlight_node;

	// Meshes
	class GeometryNode*								terrain;
	glm::mat4										terrain_transformation_matrix;
	glm::mat4										terrain_transformation_normal_matrix;
	class GeometryNode*								road;
	glm::mat4										road_transformation_matrix;
	glm::mat4										road_transformation_normal_matrix;
	class GeometryNode*								green_plane;
	glm::mat4										green_plane_transformation_matrix;
	glm::mat4										green_plane_transformation_normal_matrix;
	class GeometryNode*								skeleton;
	glm::mat4										skeleton_transformation_matrix;
	glm::mat4										skeleton_transformation_normal_matrix;


	// Protected Functions
	bool InitRenderingTechniques();
	bool InitIntermediateShaderBuffers();
	bool InitCommonItems();
	bool InitLightSources();
	bool InitGeometricMeshes();

	ShaderProgram								m_geometry_rendering_program;

	ParticleEmitter								m_particle_emitter;
	ParticleSwirl								m_particle_swirl;
	ShaderProgram								m_particle_rendering_program;

public:
	Renderer();
	~Renderer();
	bool										Init(int SCREEN_WIDTH, int SCREEN_HEIGHT);
	void										Update(float dt);
	bool										ResizeBuffers(int SCREEN_WIDTH, int SCREEN_HEIGHT);
	bool										ReloadShaders();
	void										Render();

	// Passes
	void										RenderGeometry();
	
	// Set functions
	void										SetRenderingMode(RENDERING_MODE mode);

	// Camera Function
	void										CameraMoveForward(bool enable);
	void										CameraMoveBackWard(bool enable);
	void										CameraMoveLeft(bool enable);
	void										CameraMoveRight(bool enable);
	void										CameraLook(glm::vec2 lookDir);	

	// Skeleton Move Functions
	void										SkeletonMoveForward(bool enable);
	void										SkeletonMoveBackWard(bool enable);
	void										SkeletonMoveLeft(bool enable);
	void										SkeletonMoveRight(bool enable);
};

#endif
