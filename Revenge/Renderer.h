#ifndef BIM_ENGINE_RENDERER_H
#define BIM_ENGINE_RENDERER_H

#include "GLEW\glew.h"
#include "glm\glm.hpp"
#include <vector>
#include "ShaderProgram.h"
#include "SpotlightNode.h"
#include "ParticleSystem.h"
#include "Skybox.h"
#include "particle_system.h"
#include "text_rendering.h"


class Renderer
{
public:
	enum RENDERING_MODE
	{
		TRIANGLES,
		LINES,
		POINTS
	};
	int speed = 1;
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
	class GeometryNode* left_foot;
	class GeometryNode* right_foot;
	class GeometryNode* hand;
	class GeometryNode* body;
	class GeometryNode*	rock;
	class GeometryNode*								treasure;
	glm::mat4										treasure_transformation_matrix;
	glm::mat4										treasure_transformation_normal_matrix;
	class GeometryNode*								ball;
	glm::mat4										ball_transformation_matrix;
	glm::mat4										ball_transformation_normal_matrix;
	class GeometryNode*								tower;
	glm::mat4										tower_transformation_matrix;
	glm::mat4										tower_transformation_normal_matrix;
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

	GLuint m_fbo;
	GLuint m_fbo_depth_texture;
	GLuint m_fbo_texture;
	
	GLuint m_vao_fbo, m_vbo_fbo_vertices;

	// Protected Functions
	bool InitRenderingTechniques();
	bool InitIntermediateShaderBuffers();
	bool InitCommonItems();
	bool InitLightSources();
	bool InitGeometricMeshes();

	ShaderProgram								m_geometry_rendering_program;
	ShaderProgram								m_tower_rendering_program;
	ShaderProgram								skybox_rendering_program;
	ShaderProgram								textured_particle_rendering_program;
	ShaderProgram								shadow_rendering_program;
	ShaderProgram								m_postprocess_program;

	ParticleEmitter								m_particle_emitter;
	ParticleSwirl								m_particle_swirl;
	ShaderProgram								m_particle_rendering_program;
	Skybox*										skybox;
	particle_array								particle1;
	font_map									font;

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
	void										RenderShadowMap();
	void										DrawGeometryNodeToShadowMap();

	// Set functions
	void										SetRenderingMode(RENDERING_MODE mode);

	// Camera Function
	void										CameraMoveForward(bool enable);
	void										CameraMoveBackWard(bool enable);
	void										CameraMoveLeft(bool enable);
	void										CameraMoveRight(bool enable);
	void										CameraLook(glm::vec2 lookDir);	

	void										move_green_plane(glm::vec3 mov);
	void BuildTower();
	void RemoveTower();
	void SpawnPirate();
	void SpawnMeteor();
	void RenderToOutFB();
	void CleanUp();
};

#endif
