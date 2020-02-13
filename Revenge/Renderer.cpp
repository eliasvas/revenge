#include "Renderer.h"
#include "GeometryNode.h"
#include "Tools.h"
#include <algorithm>
#include <vector>
#include "ShaderProgram.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Entity.h"
#include "OBJLoader.h"
#include "CircleCollider.h"
#include "time.h"
#include <string>
#include "particle_system.h"
#include "Timed_Spawner.h"
#include "glm/gtc/matrix_inverse.hpp"

i32 tilemap[]={ 1, 0, 0, 0 ,0, 0, 1, 0, 0, 0,
				1, 0, 0, 0 ,0, 0, 1, 1, 1, 1,
				1, 0, 0, 0 ,0, 0, 0, 0, 0, 1,
				1, 1, 0, 0 ,0, 0, 0, 1, 1, 1,
				0, 1, 0, 0 ,0, 0, 0, 1, 0, 0,
				0, 1, 0, 0 ,0, 0, 0, 1, 0, 0,
				0, 1, 0, 0 ,0, 0, 1, 1, 0, 0,
				0, 1, 1, 1 ,0, 0, 1, 0, 0, 0,
				0, 0, 0, 1 ,1, 1, 1, 0, 0, 0,
				0, 0, 0, 0 ,0, 0, 0, 0, 0, 0
			   };

std::vector<glm::vec2> path = { {0,0},{0,1},{0,2},{0,3},{1,3},{1,4},{1,5},{1,6},{1,7},{2,7}, {3,7}, {3,8}, {4,8}, {5,8}, {6,8}, {6,7}, 
{6,6}, {7,6}, {7,5},{7,4},{7,3}, {8,3},{9,3},{9,2}, {9,1}, {8,1}, {7,1}, {6,1}, {6,0} }; 

i32 tilemap_width = 10;
i32 tilemap_height = 10;
Entity* skeleton_no_anim = NULL;
Entity* tile = NULL;
Entity* cannonball = NULL;
Entity* chest = NULL;
Timed_Spawner* t;
Pirate* p1;
f32 pirates_to_spawn = 1.0f;
// RENDERER
Renderer::Renderer()
{	
	terrain = nullptr;
	road = nullptr;
	green_plane = nullptr;

	m_rendering_mode = RENDERING_MODE::TRIANGLES;	
	m_continous_time = 0.0;
	m_camera_position = glm::vec3(1, 3, -6);
	m_camera_target_position = glm::vec3(0, 0, 0);
	m_camera_up_vector = glm::vec3(0, 1, 0);
}

Renderer::~Renderer()
{
	delete terrain;
	delete road;
	delete green_plane;
}

bool Renderer::Init(i32 SCREEN_WIDTH, i32 SCREEN_HEIGHT)
{
	this->m_screen_width = SCREEN_WIDTH;
	this->m_screen_height = SCREEN_HEIGHT;

	// Initialize OpenGL functions

	//Set clear color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//This enables depth and stencil testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	// glClearDepth sets the value the depth buffer is set at, when we clear it

	// Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// open the viewport
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); //we set up our viewport

	bool techniques_initialization = InitRenderingTechniques();
	bool buffers_initialization = InitIntermediateShaderBuffers();
	bool items_initialization = InitCommonItems();
	bool lights_sources_initialization = InitLightSources();
	bool meshes_initialization = InitGeometricMeshes();

	//If there was any errors
	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		printf("Exiting with error at Renderer::Init\n");
		return false;
	}
	
	//std::vector<glm::vec2> path = find_path(tilemap, tilemap_width, tilemap_height);
	//pri32_vec2_arr(path);

	//If everything initialized
	return techniques_initialization && items_initialization && buffers_initialization;
}

void Renderer::Update(f32 dt)
{
	if (((Treasure*)chest)->state == 1) {
		((Treasure*)chest)->state = 0;
		CleanUp(); //do all the other stuff
	}
	t->spawn_instances = (i32)pirates_to_spawn; //ugly AF
	if (pirates_to_spawn <5.5)
		pirates_to_spawn += (f32)dt / 20;
	//pri32f("%f\n", pirates_to_spawn);
	f32 movement_speed = 2.0f;
	glm::vec3 direction = glm::normalize(m_camera_target_position - m_camera_position);

	m_camera_position += m_camera_movement.x *  movement_speed * direction * dt;
	m_camera_target_position += m_camera_movement.x * movement_speed * direction * dt;

	glm::vec3 right = glm::normalize(glm::cross(direction, m_camera_up_vector));
	m_camera_position += m_camera_movement.y *  movement_speed * right * dt;
	m_camera_target_position += m_camera_movement.y * movement_speed * right * dt;

	glm::mat4 rotation = glm::mat4(1.0f);
	f32 angular_speed = glm::pi<f32>() * 0.0025f;
	
	rotation *= glm::rotate(glm::mat4(1.0), m_camera_look_angle_destination.y * angular_speed, right);
	rotation *= glm::rotate(glm::mat4(1.0), -m_camera_look_angle_destination.x  * angular_speed, m_camera_up_vector);
	m_camera_look_angle_destination = glm::vec2(0);
	
	direction = rotation * glm::vec4(direction, 0);
	f32 dist = glm::distance(m_camera_position, m_camera_target_position);
	m_camera_target_position = m_camera_position + direction * dist;
	
	//IMPORTANT
	m_view_matrix = glm::lookAt(m_camera_position, m_camera_target_position, m_camera_up_vector);

	m_continous_time += dt;

	tile->Update(dt);
	for (auto ball : CannonBall::balls)if (ball!=NULL)ball->Update(dt, speed);
	for (auto pirate : Pirate::pirates)if (pirate!=NULL)pirate->Update(dt, speed);
	for (auto tower : Tower::towers)if (tower!=NULL)tower->Update(dt, speed);
	for (auto m : Meteor::meteors)if(m!=NULL)m->Update(dt,speed);
	chest->Update(dt);
	t->Update(dt);
	//m_particle_emitter.Update(dt);
	//m_particle_swirl.Update(dt);
	update_textured_particle_swoosh(&particle1, dt, speed);
	// update meshes tranformations
	
	// Update object1 ...

	// Update object2 ...
	// Update object3 ...

}

bool Renderer::InitCommonItems()
{
	return true;
}

bool Renderer::InitRenderingTechniques()
{
	bool initialized = true;

	std::string vertex_shader_path = "../Data/Shaders/basic_rendering.vert";
    std::string fragment_shader_path = "../Data/Shaders/basic_rendering.frag";
    m_geometry_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
    m_geometry_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
    m_geometry_rendering_program.CreateProgram();
    m_geometry_rendering_program.LoadUniform("uniform_projection_matrix");
    m_geometry_rendering_program.LoadUniform("uniform_view_matrix");
    m_geometry_rendering_program.LoadUniform("uniform_model_matrix");
    m_geometry_rendering_program.LoadUniform("uniform_normal_matrix");
    m_geometry_rendering_program.LoadUniform("uniform_diffuse");
    m_geometry_rendering_program.LoadUniform("uniform_specular");
    m_geometry_rendering_program.LoadUniform("uniform_shininess");
    m_geometry_rendering_program.LoadUniform("uniform_has_texture");
    m_geometry_rendering_program.LoadUniform("diffuse_texture");
    m_geometry_rendering_program.LoadUniform("uniform_camera_position");
    // Light Source Uniforms
    m_geometry_rendering_program.LoadUniform("uniform_light_projection_matrix");
    m_geometry_rendering_program.LoadUniform("uniform_light_view_matrix");
    m_geometry_rendering_program.LoadUniform("uniform_light_position");
    m_geometry_rendering_program.LoadUniform("uniform_light_direction");
    m_geometry_rendering_program.LoadUniform("uniform_light_color");
    m_geometry_rendering_program.LoadUniform("uniform_light_umbra");
    m_geometry_rendering_program.LoadUniform("uniform_light_penumbra");
    m_geometry_rendering_program.LoadUniform("uniform_cast_shadows");
    m_geometry_rendering_program.LoadUniform("shadowmap_texture");

	vertex_shader_path = "../Data/Shaders/basic_rendering.vert";
	fragment_shader_path = "../Data/Shaders/tower_rendering.frag";
	m_tower_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_tower_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	m_tower_rendering_program.CreateProgram();
	m_tower_rendering_program.LoadUniform("uniform_projection_matrix");
	m_tower_rendering_program.LoadUniform("uniform_view_matrix");
	m_tower_rendering_program.LoadUniform("uniform_model_matrix");
	m_tower_rendering_program.LoadUniform("uniform_normal_matrix");
	m_tower_rendering_program.LoadUniform("uniform_diffuse");
	m_tower_rendering_program.LoadUniform("uniform_specular");
	m_tower_rendering_program.LoadUniform("uniform_shininess");
	m_tower_rendering_program.LoadUniform("uniform_has_texture");
	m_tower_rendering_program.LoadUniform("diffuse_texture");
	m_tower_rendering_program.LoadUniform("uniform_camera_position");
	m_tower_rendering_program.LoadUniform("fade_alpha");
	m_tower_rendering_program.LoadUniform("color");
	// Light Source Uniforms
	m_tower_rendering_program.LoadUniform("uniform_light_position");
	m_tower_rendering_program.LoadUniform("uniform_light_direction");
	m_tower_rendering_program.LoadUniform("uniform_light_color");
	m_tower_rendering_program.LoadUniform("uniform_light_umbra");
	m_tower_rendering_program.LoadUniform("uniform_light_penumbra");


	// Create and Compile Particle Shader
	vertex_shader_path = "../Data/Shaders/particle_rendering.vert";
	fragment_shader_path = "../Data/Shaders/particle_rendering.frag";
	m_particle_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_particle_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = initialized && m_particle_rendering_program.CreateProgram();
	m_particle_rendering_program.LoadUniform("uniform_view_matrix");
	m_particle_rendering_program.LoadUniform("uniform_projection_matrix");
	m_particle_rendering_program.LoadUniform("alpha");


	//skybox rendering program
	vertex_shader_path = "../Data/Shaders/skybox_rendering.vert";
	fragment_shader_path = "../Data/Shaders/skybox_rendering.frag";
	skybox_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	skybox_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = initialized && skybox_rendering_program.CreateProgram();
	skybox_rendering_program.LoadUniform("uniform_view_matrix");
	skybox_rendering_program.LoadUniform("uniform_projection_matrix");

	//textured particle rendering program
	vertex_shader_path = "../Data/Shaders/textured_particle_rendering.vert";
	fragment_shader_path = "../Data/Shaders/textured_particle_rendering.frag";
	textured_particle_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	textured_particle_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = initialized && textured_particle_rendering_program.CreateProgram();
	textured_particle_rendering_program.LoadUniform("uniform_view_matrix");
	textured_particle_rendering_program.LoadUniform("uniform_projection_matrix");
	textured_particle_rendering_program.LoadUniform("alpha");
	textured_particle_rendering_program.LoadUniform("offset");
	textured_particle_rendering_program.LoadUniform("scale");

	//ui rendering program
	vertex_shader_path = "../Data/Shaders/ui_rendering.vert";
	fragment_shader_path = "../Data/Shaders/ui_rendering.frag";
	ui_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	ui_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = initialized && ui_rendering_program.CreateProgram();
	ui_rendering_program.LoadUniform("uniform_view_matrix");
	ui_rendering_program.LoadUniform("uniform_projection_matrix");
	ui_rendering_program.LoadUniform("alpha");
	ui_rendering_program.LoadUniform("offset");
	ui_rendering_program.LoadUniform("scale");


	vertex_shader_path = "../Data/Shaders/shadow_map_rendering.vert";
    fragment_shader_path = "../Data/Shaders/shadow_map_rendering.frag";
    shadow_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
    shadow_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
    initialized = initialized && shadow_rendering_program.CreateProgram();
    shadow_rendering_program.LoadUniform("uniform_projection_matrix");
    shadow_rendering_program.LoadUniform("uniform_view_matrix");
    shadow_rendering_program.LoadUniform("uniform_model_matrix");
	

	m_particle_emitter.Init();
	m_particle_swirl.Init();
	init_textured_particle_swoosh(&particle1,"../Data/Various/coin.png");
	particle1.active = 0;
	init_text2D(&font, "../Data/Various/font.png");

	return initialized;
}

bool Renderer::ReloadShaders()
{
	bool reloaded = true;
	// rendering techniques
	reloaded = m_geometry_rendering_program.ReloadProgram();
	// Reload Particle Shader
	reloaded = reloaded && m_particle_rendering_program.ReloadProgram();

	return reloaded;
}

bool Renderer::InitIntermediateShaderBuffers()
{
	return ResizeBuffers(m_screen_width, m_screen_height);
}

bool Renderer::ResizeBuffers(i32 width, i32 height)
{
	m_screen_width = width;
	m_screen_height = height;

	m_projection_matrix = glm::perspective(glm::radians(60.f), width / (f32)height, 0.1f, 1500.0f);
	m_view_matrix = glm::lookAt(m_camera_position, m_camera_target_position, m_camera_up_vector);

	return true;
}

bool Renderer::InitLightSources()
{
	m_spotlight_node.SetPosition(glm::vec3(12, 18, -3));
	m_spotlight_node.SetTarget(glm::vec3(0, 2, 0));

	m_spotlight_node.SetColor(80.0f * glm::vec3(155, 155, 255) / 255.f); //HERE IS THE LIGHT COLOR CHANGE!!!!!!!!!!!!!!!!!!!!!
	m_spotlight_node.SetConeSize(130,130);
	m_spotlight_node.CastShadow(true);

	return true;
}

bool Renderer::InitGeometricMeshes()
{
	bool initialized = true;
	OBJLoader loader;
	//transform = glm::vec3(i*2,j*2,0); obj = tiles[j*width + i]
	// load geometric object 1
	auto mesh = loader.load("../Data/Terrain/terrain.obj");
	if (mesh != nullptr)
	{
		terrain = new GeometryNode();
		terrain->Init(mesh);
	}
	else
		initialized = false;

	// load geometric object 2
	// ..
	mesh = nullptr;
	mesh = loader.load("../Data/Terrain/road.obj");
	if (mesh != nullptr) {
		road = new GeometryNode();
		road->Init(mesh);
		road_transformation_matrix = glm::translate(road_transformation_matrix,glm::vec3(2,0,0));
	}

	// load geometric object 3
	// ..
	mesh = nullptr;
	mesh = loader.load("../Data/Various/plane_green.obj");
	if (mesh != nullptr) {
		green_plane = new GeometryNode();
		green_plane->Init(mesh);
	}
	//green_plane_transformation_matrix = glm::translate(green_plane_transformation_matrix, glm::vec3(0,1,0));

	mesh = nullptr;
	mesh = loader.load("../Data/Pirates/skeleton.obj");
	if (mesh != nullptr) {
		skeleton = new GeometryNode();
		skeleton->Init(mesh);
	}
	skeleton_transformation_matrix = glm::scale(skeleton_transformation_matrix, glm::vec3(0.05, 0.05, 0.05));
	skeleton_transformation_matrix = glm::rotate(skeleton_transformation_matrix,3.1415f, glm::vec3(0,1,0));
	skeleton_transformation_matrix = glm::translate(skeleton_transformation_matrix, glm::vec3(0,4,0));
	
	mesh = nullptr;
	mesh = loader.load("../Data/MedievalTower/tower.obj");
	if (mesh != nullptr) {
		tower = new GeometryNode();
		tower->Init(mesh);
	}
	
	mesh = nullptr;
	mesh = loader.load("../Data/Various/cannonball.obj");
	if (mesh != nullptr) {
		ball = new GeometryNode();
		ball->Init(mesh);
	}
	ball_transformation_matrix = glm::scale(ball_transformation_matrix, glm::vec3(0.1f));
	ball_transformation_matrix = glm::translate(ball_transformation_matrix, glm::vec3(0,1,0));

	mesh = nullptr;
	mesh = loader.load("../Data/Treasure/treasure_chest.obj");
	if (mesh != nullptr) {
		treasure = new GeometryNode();
		treasure->Init(mesh);
	}
	treasure_transformation_matrix = glm::scale(treasure_transformation_matrix,glm::vec3(0.05));
	treasure_transformation_matrix = glm::translate(treasure_transformation_matrix, glm::vec3(240,0,0));
	
	mesh = nullptr;
	mesh = loader.load("../Data/Pirate/pirate_arm.obj");
	if (mesh != nullptr) {
		hand = new GeometryNode();
		hand->Init(mesh);
	}
	mesh = nullptr;
    mesh = loader.load("../Data/meteor1/Asteroid1.obj");
    if (mesh != nullptr) {
        meteor = new GeometryNode();
        meteor->Init(mesh);
    }

	mesh = nullptr;
	mesh = loader.load("../Data/Pirate/pirate_left_foot.obj");
	if (mesh != nullptr) {
		left_foot = new GeometryNode();
		left_foot->Init(mesh);
	}

	mesh = nullptr;
	mesh = loader.load("../Data/Pirate/pirate_right_foot.obj");
	if (mesh != nullptr) {
		right_foot = new GeometryNode();
		right_foot->Init(mesh);
	}

	mesh = nullptr;
	mesh = loader.load("../Data/Pirate/pirate_body.obj");
	if (mesh != nullptr) {
		body = new GeometryNode();
		body->Init(mesh);
	}


	skeleton_no_anim = new Pirate(body, hand, left_foot, right_foot,skeleton_transformation_matrix, skeleton_transformation_normal_matrix,new CircleCollider(1.0f, glm::vec3(0,0.5,0)), "pirate",path);
	skeleton_no_anim->active = false;
	tile = new Entity(green_plane, green_plane_transformation_matrix, green_plane_transformation_normal_matrix, new CircleCollider(1.0f,glm::vec3(0,0,0)), "tile");
	chest = new Treasure(treasure,treasure_transformation_matrix, treasure_transformation_normal_matrix, new CircleCollider(1.0f,glm::vec3(0,0.5,0)),"treasure", &particle1);
	p1 = new Pirate(body, hand, left_foot,right_foot,skeleton_transformation_matrix, skeleton_transformation_normal_matrix,new CircleCollider(1.0f, glm::vec3(0,0.5,1)), "pirate",path);
	p1->active = false;


	t = new Timed_Spawner(10.0f, (i32)pirates_to_spawn, 1.0f,(Pirate*)p1); //change to size of horde

	std::vector<std::string> faces = {
		"../Data/Various/ame_nebula/purplenebula_rt.tga",
		"../Data/Various/ame_nebula/purplenebula_lf.tga",
		"../Data/Various/ame_nebula/purplenebula_up.tga",
		"../Data/Various/ame_nebula/purplenebula_dn.tga",
		"../Data/Various/ame_nebula/purplenebula_bk.tga",
		"../Data/Various/ame_nebula/purplenebula_ft.tga"
	};
	skybox = new Skybox(faces);
	return initialized;
}

void Renderer::SetRenderingMode(RENDERING_MODE mode)
{
	m_rendering_mode = mode;
}

void Renderer::Render()
{
	//render the skybox
	//then draw the geometry
	RenderShadowMap();
	RenderGeometry();

	GLenum error = Tools::CheckGLError();
	if (error != GL_NO_ERROR)
	{
		printf("Renderer:Draw GL Error %d\n", error);
		system("pause");
	}
}


void Renderer::RenderGeometry()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_screen_width, m_screen_height);
	
	// clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glEnable(GL_DEPTH_TEST);
	
	// render only points, lines, triangles
	switch (m_rendering_mode)
	{
	case RENDERING_MODE::TRIANGLES:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case RENDERING_MODE::LINES:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case RENDERING_MODE::POINTS:
		glPointSize(2);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	};
	//we need to first disable the glDepthTest so as to draw only 
	//on the color buffer
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	skybox_rendering_program.Bind();
	glUniformMatrix4fv(skybox_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
	glUniformMatrix4fv(skybox_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_view_matrix));
	
	skybox->Render(skybox_rendering_program);

	// Bind the shader program
	m_geometry_rendering_program.Bind();
	
	// pass the camera properties
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_view_matrix));
	glUniform3f(m_geometry_rendering_program["uniform_camera_position"], m_camera_position.x, m_camera_position.y, m_camera_position.z);
	
	// pass the light source parameters
	glm::vec3 light_position = m_spotlight_node.GetPosition();
	glm::vec3 light_direction = m_spotlight_node.GetDirection();
	glm::vec3 light_color = m_spotlight_node.GetColor();
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_light_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetProjectionMatrix()));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_light_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetViewMatrix()));
	glUniform3f(m_geometry_rendering_program["uniform_light_position"], light_position.x, light_position.y, light_position.z);
	glUniform3f(m_geometry_rendering_program["uniform_light_direction"], light_direction.x, light_direction.y, light_direction.z);
	glUniform3f(m_geometry_rendering_program["uniform_light_color"], light_color.x, light_color.y, light_color.z);
	glUniform1f(m_geometry_rendering_program["uniform_light_umbra"], m_spotlight_node.GetUmbra());
	glUniform1f(m_geometry_rendering_program["uniform_light_penumbra"], m_spotlight_node.GetPenumbra());
	glUniform1i(m_geometry_rendering_program["uniform_cast_shadows"], (m_spotlight_node.GetCastShadowsStatus()) ? 1 : 0);
	// Set the sampler2D uniform to use texture unit 1
	glUniform1i(m_geometry_rendering_program["shadowmap_texture"], 1);
	// Bind the shadow map texture to texture unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, (m_spotlight_node.GetCastShadowsStatus()) ? m_spotlight_node.GetShadowMapDepthTexture() : 0);
	// Enable Texture Unit 0
	glUniform1i(m_geometry_rendering_program["uniform_diffuse_texture"], 0);
	glActiveTexture(GL_TEXTURE0);



	//Draw Tiles 
	glm::mat4 transform = terrain_transformation_matrix;
	//TODO should be done with glDrawArraysInstanced
	for (i32 y = 0; y < tilemap_height; ++y) {
		for (i32 x = 0; x < tilemap_width; ++x) {
			
			//std::cout << glm::to_string(transform) << std::endl;
			if (tilemap[x + y*tilemap_height] == 0 || tilemap[x+y*tilemap_height] == 2) { //todo fix
				//draw terrain
				glBindVertexArray(terrain->m_vao);
				glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(transform));
				glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(terrain_transformation_normal_matrix));
				for (i32 j = 0; j < terrain->parts.size(); j++)
				{
					glm::vec3 diffuseColor = terrain->parts[j].diffuseColor;
					glm::vec3 specularColor = terrain->parts[j].specularColor;
					f32 shininess = terrain->parts[j].shininess;

					glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
					glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
					glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
					glUniform1f(m_geometry_rendering_program["uniform_has_texture"], (terrain->parts[j].textureID > 0) ? 1.0f : 0.0f);
					glBindTexture(GL_TEXTURE_2D, terrain->parts[j].textureID);

					glDrawArrays(GL_TRIANGLES, terrain->parts[j].start_offset, terrain->parts[j].count);
				}

			}
			else if (tilemap[x + y * tilemap_height] == 1) {
				glBindVertexArray(road->m_vao);
				glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(transform));
				glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(road_transformation_normal_matrix));
				for (i32 j = 0; j < road->parts.size(); ++j) {
					glm::vec3 diffuseColor = road->parts[j].diffuseColor;
					glm::vec3 specularColor = road->parts[j].specularColor;
					f32 shininess = road->parts[j].shininess;

					glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
					glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
					glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
					glUniform1f(m_geometry_rendering_program["uniform_has_texture"], (road->parts[j].textureID > 0) ? 1.0f : 0.0f);
					glBindTexture(GL_TEXTURE_2D, road->parts[j].textureID);

					glDrawArrays(GL_TRIANGLES, road->parts[j].start_offset, road->parts[j].count);

				}
			}
			transform = glm::translate(transform, glm::vec3(2, 0, 0));//y += 2; //2 is arbitary due to the [-1,1] size of our tiles
		}
		transform = terrain_transformation_matrix;
		transform = glm::translate(transform,glm::vec3(0, 0,2*(y+1)));//x += 2;
	}

	if (((Treasure*)chest)->state == 1)return; //breaks game------


	tile->Render(m_geometry_rendering_program);
	for (auto p : Pirate::pirates)if(p!=NULL)p->Render(m_geometry_rendering_program);
	for (auto c : CannonBall::balls)if(c!=NULL)c->Render(m_geometry_rendering_program);
	for (auto m : Meteor::meteors)if(m!=NULL)m->Render(m_geometry_rendering_program);
	chest->Render(m_geometry_rendering_program);


	m_tower_rendering_program.Bind();
	// pass the camera properties
	glUniformMatrix4fv(m_tower_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
	glUniformMatrix4fv(m_tower_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_view_matrix));
	glUniform3f(m_tower_rendering_program["uniform_camera_position"], m_camera_position.x, m_camera_position.y, m_camera_position.z);
	
	// pass the light source parameters
	light_position = m_spotlight_node.GetPosition();
	light_direction = m_spotlight_node.GetDirection();
	light_color = m_spotlight_node.GetColor();
	glUniform3f(m_tower_rendering_program["uniform_light_position"], light_position.x, light_position.y, light_position.z);
	glUniform3f(m_tower_rendering_program["uniform_light_direction"], light_direction.x, light_direction.y, light_direction.z);
	glUniform3f(m_tower_rendering_program["uniform_light_color"], light_color.x, light_color.y, light_color.z);
	glUniform1f(m_tower_rendering_program["uniform_light_umbra"], m_spotlight_node.GetUmbra());
	glUniform1f(m_tower_rendering_program["uniform_light_penumbra"], m_spotlight_node.GetPenumbra());
	
	// Enable Texture Unit 0
	glUniform1i(m_tower_rendering_program["uniform_diffuse_texture"], 0);
	glActiveTexture(GL_TEXTURE0);

	for (auto t : Tower::towers)if(t!=NULL)t->Render(m_tower_rendering_program);

	if (m_rendering_mode != RENDERING_MODE::TRIANGLES)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	// Render Particles
	///* 
	textured_particle_rendering_program.Bind();
	glm::mat4 view = m_view_matrix;
	//view[0][0] = -1.0f;
	//view[0][1] = 0.0f;
	//view[0][2] = 0.0f;
	//view[1][0] = 0.0f;
	//view[1][1] = 1.0f;
	//view[1][2] = 0.0f;
	//view[2][0] = 0.0f;
	//view[2][1] = 0.0f;
	//view[2][2] = 1.0f;

	glUniformMatrix4fv(textured_particle_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
	glUniformMatrix4fv(textured_particle_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(view));
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D,particle1.texture);

	
	glUniformMatrix4fv(textured_particle_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(view));


	render_textured_particle(&particle1, textured_particle_rendering_program);

	//view[3][0] = -0.5f;
	//view[3][1] = -1.5f;
	//view[3][2] = -1.f;

	//view[2][3] = 0;
	glUniformMatrix4fv(textured_particle_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(view));
	//glm::mat4 proj = glm::mat4(1.0f);
	//proj[3][2] = 1.f;
	//glm::mat4 proj = glm::ortho(1.f,-1.f, 0.1f, 1500.0f); //kati tetoio prepei na gunei
	glUniformMatrix4fv(textured_particle_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
	glDisable(GL_DEPTH_TEST);   //so we can just write straight to the color buffer

	int money = ((Treasure*)chest)->money;
	std::string s("MONEY: ");
	s.append(std::to_string(money));
	ui_rendering_program.Bind();
	glUniformMatrix4fv(ui_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
	glUniformMatrix4fv(ui_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(view));
	
	render_text2D(&font,s.c_str(),0,0,256, ui_rendering_program);

	glPointSize(1.0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//*/
}

void Renderer::CameraMoveForward(bool enable)
{
	m_camera_movement.x = (enable)? 1.f : 0.f;
}
void Renderer::CameraMoveBackWard(bool enable)
{
	m_camera_movement.x = (enable) ? -1.f : 0.f;
}

void Renderer::CameraMoveLeft(bool enable)
{
	m_camera_movement.y = (enable) ? -1.f : 0.f;
}
void Renderer::CameraMoveRight(bool enable)
{
	m_camera_movement.y = (enable) ? 1.f : 0.f;
}

void Renderer::CameraLook(glm::vec2 lookDir)
{
	m_camera_look_angle_destination = glm::vec2(1, -1) * lookDir;
}

void Renderer::move_green_plane(glm::vec3 mov) {
	if (tile == NULL)return;
	if (tile->transformation_matrix[3][0] + mov[0] >= 0 && tile->transformation_matrix[3][0] + mov[0] < tilemap_width * 2
		&& tile->transformation_matrix[3][2] + mov[2] >= 0 && tile->transformation_matrix[3][2] + mov[2] <tilemap_height * 2)
	tile->transformation_matrix= glm::translate(tile->transformation_matrix, mov);
}

void Renderer::RemoveTower() {
	i32 x = (i32)std::max(0.0f,(tile->transformation_matrix[3][0] / 2));
	i32 y = (i32)std::max(0.0f,(tile->transformation_matrix[3][2] / 2));
	if (tilemap[x + y * tilemap_width]!=2)return;
	tilemap[x + y * tilemap_width] = 0; //since we are going to make a new tower we are going to prevent further building on the same block
	for (i32 i = 0; i < Tower::towers.size(); ++i) {
		if (Tower::towers[i] == NULL)continue;
		if (i32(Tower::towers[i]->transformation_matrix[3][0]) == (i32)(tile->transformation_matrix[3][0])
			&& (i32)(Tower::towers[i]->transformation_matrix[3][2]) == (i32)(tile->transformation_matrix[3][2])) { //todo check
			delete Tower::towers[i];
			Tower::towers[i] = NULL;
		}
	}
	((Treasure*)chest)->money += 50;
}

void Renderer::BuildTower() {
	i32 x = (i32)std::max(0.0f,(tile->transformation_matrix[3][0] / 2));
	i32 y = (i32)std::max(0.0f,(tile->transformation_matrix[3][2] / 2));
	if (tilemap[x + y * tilemap_width]!=0)return;
	tilemap[x + y * tilemap_width] = 2; //since we are going to make a new tower we are going to prevent further building on the same block
	auto t1 = new Tower(tower,glm::scale(tile->transformation_matrix,glm::vec3(0.2,0.2,0.2)), tile->transformation_normal_matrix,new CircleCollider(3, glm::vec3(0,0,0)),"tower", ball);
	((Treasure*)chest)->money -= 100;
}

void Renderer::SpawnMeteor() {
    i32 x = (i32)std::max(0.0f,(tile->transformation_matrix[3][0] / 2));
    i32 y = (i32)std::max(0.0f,(tile->transformation_matrix[3][2] / 2));
    if (tilemap[x + y * tilemap_width] != 1)return;
    CircleCollider* c= new CircleCollider(1.0f, glm::vec3(0, 0, 0));
    glm::vec3 t_pos= {tile->transformation_matrix[3][0],tile->transformation_matrix[3][1],tile->transformation_matrix[3][2] };
    glm::vec3 m_pos = {9,10,9};
    auto m = new Meteor(meteor,glm::scale(glm::translate(tile->transformation_matrix,glm::vec3(9-x*2,10,9-y*2)), glm::vec3(0.5,0.5,0.5)),tile->transformation_normal_matrix,t_pos-m_pos,c,"meteor");
    ((Treasure*)chest)->money -= 100;
}

void FindPath(std::vector<glm::vec3>& path_arr, i32* arr, i32 width, i32 height) {
	for (i32 y = 0; y < tilemap_height; ++y) {
		for (i32 x = 0; x < tilemap_width; ++x) {
			//do something
		}
	}
}
void Renderer::SpawnPirate() {
	Entity* skeleton_no_anim = new Pirate(body, hand, left_foot, right_foot, skeleton_transformation_matrix, skeleton_transformation_normal_matrix, new CircleCollider(1.0f, glm::vec3(0, 0.5, 0)), "pirate", path);
}

void Renderer::RenderShadowMap() {
    if (m_spotlight_node.GetCastShadowsStatus()) {

        int m_depth_texture_resolution = m_spotlight_node.GetShadowMapResolution();

        // bind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_spotlight_node.GetShadowMapFBO());
        glViewport(0, 0, m_depth_texture_resolution, m_depth_texture_resolution);
        GLenum drawbuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawbuffers);

        // clear depth buffer
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Bind the shadow mapping program
        shadow_rendering_program.Bind();

        // pass the projection and view matrix to the uniforms
        glUniformMatrix4fv(shadow_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetProjectionMatrix()));
        glUniformMatrix4fv(shadow_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetViewMatrix()));

        DrawGeometryNodeToShadowMap();

        glBindVertexArray(0);

        // Unbind shadow mapping program
        shadow_rendering_program.Unbind();

        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
void Renderer::DrawGeometryNodeToShadowMap(){
    for (auto t : Tower::towers)if (t != NULL) {
        glBindVertexArray(t->geometry->m_vao);
        glUniformMatrix4fv(shadow_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(t->transformation_matrix));
        for (int j = 0; j < t->geometry->parts.size(); j++)
        {
            glDrawArrays(GL_TRIANGLES, t->geometry->parts[j].start_offset, t->geometry->parts[j].count);
        }
    }
    for (auto p : Pirate::pirates)if (p != NULL) {
		if (!p->active)continue;
        glBindVertexArray(p->geometry->m_vao);
        glUniformMatrix4fv(shadow_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(p->transformation_matrix));
        for (int j = 0; j < p->geometry->parts.size(); j++)
        {
            glDrawArrays(GL_TRIANGLES, p->geometry->parts[j].start_offset, p->geometry->parts[j].count);
        }
    }
    for (auto c : CannonBall::balls)if (c != NULL){
        glBindVertexArray(c->geometry->m_vao);
        glUniformMatrix4fv(shadow_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(c->transformation_matrix));
        for (int j = 0; j < c->geometry->parts.size(); j++)
        {
            glDrawArrays(GL_TRIANGLES, c->geometry->parts[j].start_offset, c->geometry->parts[j].count);
        }
    }
    for (auto m : Meteor::meteors)if (m != NULL) {
        glBindVertexArray(m->geometry->m_vao);
        glUniformMatrix4fv(shadow_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m->transformation_matrix));
        for (int j = 0; j < m->geometry->parts.size(); j++)
        {
            glDrawArrays(GL_TRIANGLES, m->geometry->parts[j].start_offset, m->geometry->parts[j].count);
        }
    }

}


void Renderer::CleanUp() {
	Pirate::pirates.clear();
	CannonBall::balls.clear();
	Tower::towers.clear();
	Meteor::meteors.clear();
	t->time = 0.0f;
	pirates_to_spawn = 1.0f;
	particle1.active = false;
	//score = 0;
}






