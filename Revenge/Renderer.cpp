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
#include "Timed_Spawner.h"

int tilemap[]={ 1, 0, 0, 0 ,0, 0, 1, 0, 0, 0,
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
std::vector<glm::vec2> path = { {0,0},{1,0},{2,0},{3,0},{3,1},{4,1},{5,1},{6,1},{7,1},{7,2}, {7,3}, {8,3}, {8,4}, {8,5}, {8,6}, {7,6}, 
{6,6}, {6,7}, {5,7},{4,7},{3,7}, {3,8},{3,9},{2,9}, {1,9}, {1,8}, {1,7}, {1,6}, {0,6} }; //just a test must be replaced by the real routing function
int tilemap_width = 10;
int tilemap_height = 10;
Entity* skeleton_no_anim = NULL;
Entity* tile = NULL;
Entity* cannonball = NULL;
Entity* chest = NULL;
Timed_Spawner* t;
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

bool Renderer::Init(int SCREEN_WIDTH, int SCREEN_HEIGHT)
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
	//print_vec2_arr(path);

	//If everything initialized
	return techniques_initialization && items_initialization && buffers_initialization;
}

void Renderer::Update(float dt)
{
	float movement_speed = 2.0f;
	glm::vec3 direction = glm::normalize(m_camera_target_position - m_camera_position);

	m_camera_position += m_camera_movement.x *  movement_speed * direction * dt;
	m_camera_target_position += m_camera_movement.x * movement_speed * direction * dt;

	glm::vec3 right = glm::normalize(glm::cross(direction, m_camera_up_vector));
	m_camera_position += m_camera_movement.y *  movement_speed * right * dt;
	m_camera_target_position += m_camera_movement.y * movement_speed * right * dt;

	glm::mat4 rotation = glm::mat4(1.0f);
	float angular_speed = glm::pi<float>() * 0.0025f;
	
	rotation *= glm::rotate(glm::mat4(1.0), m_camera_look_angle_destination.y * angular_speed, right);
	rotation *= glm::rotate(glm::mat4(1.0), -m_camera_look_angle_destination.x  * angular_speed, m_camera_up_vector);
	m_camera_look_angle_destination = glm::vec2(0);
	
	direction = rotation * glm::vec4(direction, 0);
	float dist = glm::distance(m_camera_position, m_camera_target_position);
	m_camera_target_position = m_camera_position + direction * dist;
	
	//IMPORTANT
	m_view_matrix = glm::lookAt(m_camera_position, m_camera_target_position, m_camera_up_vector);

	m_continous_time += dt;

	tile->Update(dt);
	for (auto ball : CannonBall::balls)if (ball!=NULL)ball->Update(dt, speed);
	for (auto pirate : Pirate::pirates)if (pirate!=NULL)pirate->Update(dt, speed);
	for (auto tower : Tower::towers)if (tower!=NULL)tower->Update(dt);
	chest->Update(dt);
	t->Update(dt);
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
	m_geometry_rendering_program.LoadUniform("uniform_light_position");
	m_geometry_rendering_program.LoadUniform("uniform_light_direction");
	m_geometry_rendering_program.LoadUniform("uniform_light_color");
	m_geometry_rendering_program.LoadUniform("uniform_light_umbra");
	m_geometry_rendering_program.LoadUniform("uniform_light_penumbra");

	// Create and Compile Particle Shader
	vertex_shader_path = "../Data/Shaders/particle_rendering.vert";
	fragment_shader_path = "../Data/Shaders/particle_rendering.frag";
	m_particle_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_particle_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = initialized && m_particle_rendering_program.CreateProgram();
	m_particle_rendering_program.LoadUniform("uniform_view_matrix");
	m_particle_rendering_program.LoadUniform("uniform_projection_matrix");


	//skybox rendering program
	vertex_shader_path = "../Data/Shaders/skybox_rendering.vert";
	fragment_shader_path = "../Data/Shaders/skybox_rendering.frag";
	skybox_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	skybox_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = initialized && skybox_rendering_program.CreateProgram();
	skybox_rendering_program.LoadUniform("uniform_view_matrix");
	skybox_rendering_program.LoadUniform("uniform_projection_matrix");

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

bool Renderer::ResizeBuffers(int width, int height)
{
	m_screen_width = width;
	m_screen_height = height;

	m_projection_matrix = glm::perspective(glm::radians(60.f), width / (float)height, 0.1f, 1500.0f);
	m_view_matrix = glm::lookAt(m_camera_position, m_camera_target_position, m_camera_up_vector);

	return true;
}

bool Renderer::InitLightSources()
{
	m_spotlight_node.SetPosition(glm::vec3(12, 18, -3));
	m_spotlight_node.SetTarget(glm::vec3(0, 2, 0));

	m_spotlight_node.SetColor(80.0f * glm::vec3(155, 155, 255) / 255.f); //HERE IS THE LIGHT COLOR CHANGE!!!!!!!!!!!!!!!!!!!!!
	m_spotlight_node.SetConeSize(500, 700);
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


	skeleton_no_anim = new Pirate(skeleton,skeleton_transformation_matrix, skeleton_transformation_normal_matrix,new CircleCollider(1.0f, glm::vec3(0,0.5,0)), "pirate",path);
	skeleton_no_anim->active = false;
	tile = new Entity(green_plane, green_plane_transformation_matrix, green_plane_transformation_normal_matrix, new CircleCollider(1.0f,glm::vec3(0,0,0)), "tile");
	chest = new Treasure(treasure,treasure_transformation_matrix, treasure_transformation_normal_matrix, new CircleCollider(1.0f,glm::vec3(0,0.5,0)),"treasure");
	

	t = new Timed_Spawner(10.0f, 2, 1.0f,(Pirate*)skeleton_no_anim);

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
	RenderGeometry();

	GLenum error = Tools::CheckGLError();
	if (error != GL_NO_ERROR)
	{
		printf("Reanderer:Draw GL Error\n");
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
	glUniform3f(m_geometry_rendering_program["uniform_light_position"], light_position.x, light_position.y, light_position.z);
	glUniform3f(m_geometry_rendering_program["uniform_light_direction"], light_direction.x, light_direction.y, light_direction.z);
	glUniform3f(m_geometry_rendering_program["uniform_light_color"], light_color.x, light_color.y, light_color.z);
	glUniform1f(m_geometry_rendering_program["uniform_light_umbra"], m_spotlight_node.GetUmbra());
	glUniform1f(m_geometry_rendering_program["uniform_light_penumbra"], m_spotlight_node.GetPenumbra());
	
	// Enable Texture Unit 0
	glUniform1i(m_geometry_rendering_program["uniform_diffuse_texture"], 0);
	glActiveTexture(GL_TEXTURE0);

	//Draw Tiles 
	glm::mat4 transform = terrain_transformation_matrix;
	//TODO should be done with glDrawArraysInstanced
	for (int y = 0; y < tilemap_height; ++y) {
		for (int x = 0; x < tilemap_width; ++x) {
			
			//std::cout << glm::to_string(transform) << std::endl;
			if (tilemap[x + y*tilemap_height] == 0 || tilemap[x+y*tilemap_height] == 2) { //todo fix
				//draw terrain
				glBindVertexArray(terrain->m_vao);
				glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(transform));
				glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(terrain_transformation_normal_matrix));
				for (int j = 0; j < terrain->parts.size(); j++)
				{
					glm::vec3 diffuseColor = terrain->parts[j].diffuseColor;
					glm::vec3 specularColor = terrain->parts[j].specularColor;
					float shininess = terrain->parts[j].shininess;

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
				for (int j = 0; j < road->parts.size(); ++j) {
					glm::vec3 diffuseColor = road->parts[j].diffuseColor;
					glm::vec3 specularColor = road->parts[j].specularColor;
					float shininess = road->parts[j].shininess;

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



	//skeleton_no_anim->Render(m_geometry_rendering_program);
	tile->Render(m_geometry_rendering_program);
	//cannonball->Render(m_geometry_rendering_program);
	for (auto t : Tower::towers)if(t!=NULL)t->Render(m_geometry_rendering_program);
	for (auto p : Pirate::pirates)if(p!=NULL)p->Render(m_geometry_rendering_program);
	for (auto c : CannonBall::balls)if(c!=NULL)c->Render(m_geometry_rendering_program);
	//for (auto c : CannonBall::balls)if(c!=NULL)c->Render(m_geometry_rendering_program);
	chest->Render(m_geometry_rendering_program);










	// unbind the vao
	glBindVertexArray(0);
	// unbind the shader program
	m_geometry_rendering_program.Unbind();

	if (m_rendering_mode != RENDERING_MODE::TRIANGLES)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Render Particles
	// ..

	glDisable(GL_DEPTH_TEST);
	glPointSize(1.0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	int x = (int)std::max(0.0f,(tile->transformation_matrix[3][0] / 2));
	int y = (int)std::max(0.0f,(tile->transformation_matrix[3][2] / 2));
	if (tilemap[x + y * tilemap_width]!=2)return;
	tilemap[x + y * tilemap_width] = 0; //since we are going to make a new tower we are going to prevent further building on the same block
	for (int i = 0; i < Tower::towers.size(); ++i) {
		if (Tower::towers[i] == NULL)continue;
		if (int(Tower::towers[i]->transformation_matrix[3][0]) == (int)(tile->transformation_matrix[3][0])
			&& (int)(Tower::towers[i]->transformation_matrix[3][2]) == (int)(tile->transformation_matrix[3][2])) { //todo check
			delete Tower::towers[i];
			Tower::towers[i] = NULL;
		}
	}
	((Treasure*)chest)->money += 50;
}

void Renderer::BuildTower() {
	int x = (int)std::max(0.0f,(tile->transformation_matrix[3][0] / 2));
	int y = (int)std::max(0.0f,(tile->transformation_matrix[3][2] / 2));
	if (tilemap[x + y * tilemap_width]!=0)return;
	tilemap[x + y * tilemap_width] = 2; //since we are going to make a new tower we are going to prevent further building on the same block
	auto t1 = new Tower(tower,glm::scale(tile->transformation_matrix,glm::vec3(0.2,0.2,0.2)), tile->transformation_normal_matrix,new CircleCollider(3, glm::vec3(0,0,0)),"tower", ball);
	((Treasure*)chest)->money -= 100;
}
void FindPath(std::vector<glm::vec3>& path_arr, int* arr, int width, int height) {
	for (int y = 0; y < tilemap_height; ++y) {
		for (int x = 0; x < tilemap_width; ++x) {
			//do something
		}
	}
}
void Renderer::SpawnPirate() {
	Entity* skeleton_no_anim = new Pirate(skeleton, skeleton_transformation_matrix, skeleton_transformation_normal_matrix, new CircleCollider(1.0f, glm::vec3(0, 0.5, 0)), "pirate", path);
}







