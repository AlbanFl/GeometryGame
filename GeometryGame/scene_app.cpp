#include "scene_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>
#include <system/debug_log.h>
#include <graphics/renderer_3d.h>
#include <graphics/mesh.h>
#include <maths/math_utils.h>
#include <input/sony_controller_input_manager.h>
#include <graphics/sprite.h>
#include "load_texture.h"
#include <input/touch_input_manager.h>
#include <input/input_manager.h>
#include <graphics/scene.h>
#include <animation/skeleton.h>
#include <animation/animation.h>
#include <input/keyboard.h>
//#include <math.h>

SceneApp::SceneApp(gef::Platform& platform) :
	Application(platform),
	sprite_renderer_(NULL),
	renderer_3d_(NULL),
	primitive_builder_(NULL),
	input_manager_(NULL),
	audio_manager_(NULL),
	font_(NULL),
	world_(NULL),
	player_body_(NULL),
	sfx_id_(-1),
	sfx_voice_id_(-1),
	button_icon_cross(NULL),
	button_icon_square(NULL),
	button_icon_circle(NULL),
	is_paused(false),
	color("RED"),
	sound_volume_(1.0),
	win(false)
	
{
}

void SceneApp::Init()
{
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	InitFont();

	music_playing_ = false;

	// initialise input manager
	input_manager_ = gef::InputManager::Create(platform_);

	// initialise audio manager
	audio_manager_ = gef::AudioManager::Create();

	//set the initianal state
	game_state_ = FRONTEND;
	FrontendInit();
}

void SceneApp::CleanUp()
{
	delete audio_manager_;
	audio_manager_ = NULL;

	delete input_manager_;
	input_manager_ = NULL;

	CleanUpFont();

	delete sprite_renderer_;
	sprite_renderer_ = NULL;
}

bool SceneApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;


	input_manager_->Update();

	switch (game_state_)
	{
		case FRONTEND:
		{
			FrontendUpdate(frame_time);
		}
		break;
			
		case PLAY_GAME:
		{
			GameUpdate(frame_time);
		}
		break;

		case GAME_OPTIONS:
		{
			GameOptionsUpdate(frame_time);
		}
		break;

		case FINISH_SCREEN:
		{
			FinishUpdate(frame_time);
		}
		break;

		case PAUSE_SCREEN:
		{
			PausescreenUpdate(frame_time);
		}
		break;
	}
	return true;
}




void SceneApp::Render()
{
	switch (game_state_)
		{
		case FRONTEND:
		{
			FrontendRender();
		}
		break;

		case PLAY_GAME:
		{
			GameRender();
		}
		break;

		case GAME_OPTIONS:
		{
			GameOptionsRender();
		}
		break;

		case FINISH_SCREEN:
		{
			FinishRender();
		}
		break;

		case PAUSE_SCREEN:
		{
			PausescreenRender();
		}
		break;
	}
}

void SceneApp::InitPlayer()
{
	// setup the mesh for the player
	player_.set_mesh(primitive_builder_->GetDefaultCubeMesh());

	// create a physics body for the player
	b2BodyDef player_body_def;
	player_body_def.type = b2_dynamicBody;
	player_body_def.position = b2Vec2(0.0f, 4.0f);

	player_body_ = world_->CreateBody(&player_body_def);

	// create the shape for the player
	b2PolygonShape player_shape;
	player_shape.SetAsBox(0.5f, 0.5f);

	// create the fixture
	b2FixtureDef player_fixture_def;
	player_fixture_def.shape = &player_shape;
	player_fixture_def.density = 1.0f;

	// create the fixture on the rigid body
	player_body_->CreateFixture(&player_fixture_def);

	// update visuals from simulation data
	player_.UpdateFromSimulation(player_body_);

	// create a connection between the rigid body and GameObject
	player_body_->SetUserData(&player_);
}

void SceneApp::InitGround()
{
	// ground dimensions
	gef::Vector4 ground_half_dimensions(70.0f, 0.5f, 0.5f);
	gef::Vector4 ground_half_dimensions2(50.0f, 0.5f, 0.5f);
	gef::Vector4 ground_half_dimensions3(50.0f, 0.5f, 0.5f);
	gef::Vector4 ground_half_dimensions4(30.0f, 0.5f, 0.5f);

	// setup the mesh for the ground
	ground_mesh_ = primitive_builder_->CreateBoxMesh(ground_half_dimensions);
	ground_.set_mesh(ground_mesh_);

	ground_mesh_2 = primitive_builder_->CreateBoxMesh(ground_half_dimensions2);
	ground_2.set_mesh(ground_mesh_2);

	ground_mesh_3 = primitive_builder_->CreateBoxMesh(ground_half_dimensions3);
	ground_3.set_mesh(ground_mesh_3);

	ground_mesh_4 = primitive_builder_->CreateBoxMesh(ground_half_dimensions4);
	ground_4.set_mesh(ground_mesh_4);

	// create a physics body
	b2BodyDef body_def;
	body_def.type = b2_staticBody;
	body_def.position = b2Vec2(60.0f, 0.0f);

	b2BodyDef body_def2;
	body_def2.type = b2_staticBody;
	body_def2.position = b2Vec2(195.0f, -3.0f);

	b2BodyDef body_def3;
	body_def3.type = b2_staticBody;
	body_def3.position = b2Vec2(310.0f, -2.5f);

	b2BodyDef body_def4;
	body_def4.type = b2_staticBody;
	body_def4.position = b2Vec2(400.0f, 2.5f);

	//create the bodies
	ground_body_ = world_->CreateBody(&body_def);
	ground_body_2 = world_->CreateBody(&body_def2);
	ground_body_3 = world_->CreateBody(&body_def3);
	ground_body_4 = world_->CreateBody(&body_def4);

	// create the shape
	b2PolygonShape shape;
	shape.SetAsBox(ground_half_dimensions.x(), ground_half_dimensions.y());

	b2PolygonShape shape2;
	shape2.SetAsBox(ground_half_dimensions2.x(), ground_half_dimensions2.y());

	b2PolygonShape shape3;
	shape3.SetAsBox(ground_half_dimensions3.x(), ground_half_dimensions3.y());

	b2PolygonShape shape4;
	shape4.SetAsBox(ground_half_dimensions4.x(), ground_half_dimensions4.y());

	// create the fixture
	b2FixtureDef fixture_def;
	fixture_def.shape = &shape;

	b2FixtureDef fixture_def2;
	fixture_def2.shape = &shape2;

	b2FixtureDef fixture_def3;
	fixture_def3.shape = &shape3;

	b2FixtureDef fixture_def4;
	fixture_def4.shape = &shape4;

	// create the fixture on the rigid body
	ground_body_->CreateFixture(&fixture_def);
	ground_body_2->CreateFixture(&fixture_def2);
	ground_body_3->CreateFixture(&fixture_def3);
	ground_body_4->CreateFixture(&fixture_def4);

	// update visuals from simulation data
	ground_.UpdateFromSimulation(ground_body_);
	ground_2.UpdateFromSimulation(ground_body_2);
	ground_3.UpdateFromSimulation(ground_body_3);
	ground_4.UpdateFromSimulation(ground_body_4);
}

void SceneApp::InitTrampoline() {
	gef::Vector4 tramp_half_dimensions(0.5f, 0.1f, 0.5f);
	gef::Vector4 tramp_half_dimensions2(0.5f, 0.1f, 0.5f);
	gef::Vector4 tramp_half_dimensions3(0.5f, 0.1f, 0.5f);

	// setup the mesh for the ground
	tramp_mesh_ = primitive_builder_->CreateBoxMesh(tramp_half_dimensions);
	tramp_.set_mesh(tramp_mesh_);

	tramp_mesh_2 = primitive_builder_->CreateBoxMesh(tramp_half_dimensions2);
	tramp_2.set_mesh(tramp_mesh_2);

	tramp_mesh_3 = primitive_builder_->CreateBoxMesh(tramp_half_dimensions3);
	tramp_3.set_mesh(tramp_mesh_3);

	// create a physics body
	b2BodyDef body_def;
	body_def.type = b2_staticBody;
	body_def.position = b2Vec2(270.0f, -2.1f);

	b2BodyDef body_def2;
	body_def2.type = b2_staticBody;
	body_def2.position = b2Vec2(330.0f, -2.1f);

	b2BodyDef body_def3;
	body_def3.type = b2_staticBody;
	body_def3.position = b2Vec2(350.0f, -2.1f);

	//create the bodies
	tramp_body_ = world_->CreateBody(&body_def);
	tramp_body_2 = world_->CreateBody(&body_def2);
	tramp_body_3 = world_->CreateBody(&body_def3);

	// create the shape
	b2PolygonShape shape;
	shape.SetAsBox(tramp_half_dimensions.x(), tramp_half_dimensions.y());

	b2PolygonShape shape2;
	shape2.SetAsBox(tramp_half_dimensions2.x(), tramp_half_dimensions2.y());

	b2PolygonShape shape3;
	shape3.SetAsBox(tramp_half_dimensions3.x(), tramp_half_dimensions3.y());

	// create the fixture
	b2FixtureDef fixture_def;
	fixture_def.shape = &shape;

	b2FixtureDef fixture_def2;
	fixture_def2.shape = &shape2;

	b2FixtureDef fixture_def3;
	fixture_def3.shape = &shape3;

	// create the fixture on the rigid body
	tramp_body_->CreateFixture(&fixture_def);
	tramp_body_2->CreateFixture(&fixture_def2);
	tramp_body_3->CreateFixture(&fixture_def3);

	// update visuals from simulation data
	tramp_.UpdateFromSimulation(tramp_body_);
	tramp_2.UpdateFromSimulation(tramp_body_2);
	tramp_3.UpdateFromSimulation(tramp_body_3);

	tramp_body_->SetUserData(&tramp_);
	tramp_body_2->SetUserData(&tramp_2);
	tramp_body_3->SetUserData(&tramp_3);
}

void SceneApp::InitPlatforms()
{
	// ground dimensions
	gef::Vector4 plat1_dimensions(0.5f, 0.5f, 0.5f);
	gef::Vector4 plat2_dimensions(0.5f, 1.0f, 0.5f);
	gef::Vector4 plat3_dimensions(0.5f, 0.5f, 0.5f);
	gef::Vector4 plat4_dimensions(4.0f, 1.5f, 0.5f);
	gef::Vector4 plat5_dimensions(0.5f, 2.0f, 0.5f);
	gef::Vector4 plat6_dimensions(0.5f, 1.5f, 0.5f);
	gef::Vector4 plat7_dimensions(0.5f, 1.0f, 0.5f);
	gef::Vector4 plat8_dimensions(10.0f, 1.0f, 0.5f);
	gef::Vector4 plat9_dimensions(5.0f, 1.0f, 0.5f);
	gef::Vector4 plat10_dimensions(1.0f, 1.5f, 0.5f);
	gef::Vector4 plat11_dimensions(2.0f, 1.0f, 0.5f);
	gef::Vector4 plat12_dimensions(0.5f, 0.5f, 0.5f);
	gef::Vector4 plat13_dimensions(0.5f, 0.5f, 0.5f);
	gef::Vector4 plat14_dimensions(8.0f, 1.0f, 0.5f);
	gef::Vector4 plat15_dimensions(1.0f, 3.0f, 0.5f);
	gef::Vector4 plat16_dimensions(0.5f, 0.5f, 0.5f);
	gef::Vector4 plat17_dimensions(0.2f, 0.4f, 0.5f);

	// setup the mesh for the ground
	plat1_mesh_ = primitive_builder_->CreateBoxMesh(plat1_dimensions);
	plat1.set_mesh(plat1_mesh_);

	plat2_mesh_ = primitive_builder_->CreateBoxMesh(plat2_dimensions);
	plat2.set_mesh(plat2_mesh_);

	plat3_mesh_ = primitive_builder_->CreateBoxMesh(plat3_dimensions);
	plat3.set_mesh(plat3_mesh_);

	plat4_mesh_ = primitive_builder_->CreateBoxMesh(plat4_dimensions);
	plat4.set_mesh(plat4_mesh_);

	plat5_mesh_ = primitive_builder_->CreateBoxMesh(plat5_dimensions);
	plat5.set_mesh(plat5_mesh_);

	plat6_mesh_ = primitive_builder_->CreateBoxMesh(plat6_dimensions);
	plat6.set_mesh(plat6_mesh_);

	plat7_mesh_ = primitive_builder_->CreateBoxMesh(plat7_dimensions);
	plat7.set_mesh(plat7_mesh_);

	plat8_mesh_ = primitive_builder_->CreateBoxMesh(plat8_dimensions);
	plat8.set_mesh(plat8_mesh_);

	plat9_mesh_ = primitive_builder_->CreateBoxMesh(plat9_dimensions);
	plat9.set_mesh(plat9_mesh_);

	plat10_mesh_ = primitive_builder_->CreateBoxMesh(plat10_dimensions);
	plat10.set_mesh(plat10_mesh_);

	plat11_mesh_ = primitive_builder_->CreateBoxMesh(plat11_dimensions);
	plat11.set_mesh(plat11_mesh_);

	plat12_mesh_ = primitive_builder_->CreateBoxMesh(plat12_dimensions);
	plat12.set_mesh(plat12_mesh_);

	plat13_mesh_ = primitive_builder_->CreateBoxMesh(plat13_dimensions);
	plat13.set_mesh(plat13_mesh_);

	plat14_mesh_ = primitive_builder_->CreateBoxMesh(plat14_dimensions);
	plat14.set_mesh(plat14_mesh_);

	plat15_mesh_ = primitive_builder_->CreateBoxMesh(plat15_dimensions);
	plat15.set_mesh(plat15_mesh_);

	plat16_mesh_ = primitive_builder_->CreateBoxMesh(plat16_dimensions);
	plat16.set_mesh(plat16_mesh_);

	plat17_mesh_ = primitive_builder_->CreateBoxMesh(plat17_dimensions);
	plat17.set_mesh(plat17_mesh_);

	// create a physics body
	b2BodyDef body_def_plat1;
	body_def_plat1.type = b2_staticBody;
	body_def_plat1.position = b2Vec2(8.0f, 1.0f);

	b2BodyDef body_def_plat2;
	body_def_plat2.type = b2_staticBody;
	body_def_plat2.position = b2Vec2(18.0f, 1.0f);

	b2BodyDef body_def_plat3;
	body_def_plat3.type = b2_staticBody;
	body_def_plat3.position = b2Vec2(25.0f, 2.5f);

	b2BodyDef body_def_plat4;
	body_def_plat4.type = b2_staticBody;
	body_def_plat4.position = b2Vec2(40.0f, 1.0f);

	b2BodyDef body_def_plat5;
	body_def_plat5.type = b2_staticBody;
	body_def_plat5.position = b2Vec2(50.0f, 2.5f);

	b2BodyDef body_def_plat6;
	body_def_plat6.type = b2_staticBody;
	body_def_plat6.position = b2Vec2(56.0f, 3.5f);

	b2BodyDef body_def_plat7;
	body_def_plat7.type = b2_staticBody;
	body_def_plat7.position = b2Vec2(65.0f, 1.25f);

	b2BodyDef body_def_plat8;
	body_def_plat8.type = b2_staticBody;
	body_def_plat8.position = b2Vec2(75.0f, 1.25f);

	b2BodyDef body_def_plat9;
	body_def_plat9.type = b2_staticBody;
	body_def_plat9.position = b2Vec2(100.0f, 1.25f);

	b2BodyDef body_def_plat10;
	body_def_plat10.type = b2_staticBody;
	body_def_plat10.position = b2Vec2(105.0f, 2.0f);

	b2BodyDef body_def_plat11;
	body_def_plat11.type = b2_staticBody;
	body_def_plat11.position = b2Vec2(170.0f, -2.0f);

	b2BodyDef body_def_plat12;
	body_def_plat12.type = b2_staticBody;
	body_def_plat12.position = b2Vec2(190.0f, -2.0f);

	b2BodyDef body_def_plat13;
	body_def_plat13.type = b2_staticBody;
	body_def_plat13.position = b2Vec2(235.0f, 0.25f);

	b2BodyDef body_def_plat14;
	body_def_plat14.type = b2_staticBody;
	body_def_plat14.position = b2Vec2(220.0f, -2.0f);

	b2BodyDef body_def_plat15;
	body_def_plat15.type = b2_staticBody;
	body_def_plat15.position = b2Vec2(280.0f, 1.0f);

	b2BodyDef body_def_plat16;
	body_def_plat16.type = b2_staticBody;
	body_def_plat16.position = b2Vec2(310.0f, -1.5f);

	b2BodyDef body_def_plat17;
	body_def_plat17.type = b2_staticBody;
	body_def_plat17.position = b2Vec2(400.0f, 2.6f);	


	//create bodies
	plat1_body_ = world_->CreateBody(&body_def_plat1);
	plat2_body_ = world_->CreateBody(&body_def_plat2);
	plat3_body_ = world_->CreateBody(&body_def_plat3);
	plat4_body_ = world_->CreateBody(&body_def_plat4);
	plat5_body_ = world_->CreateBody(&body_def_plat5);
	plat6_body_ = world_->CreateBody(&body_def_plat6);
	plat7_body_ = world_->CreateBody(&body_def_plat7);
	plat8_body_ = world_->CreateBody(&body_def_plat8);
	plat9_body_ = world_->CreateBody(&body_def_plat9);
	plat10_body_ = world_->CreateBody(&body_def_plat10);
	plat11_body_ = world_->CreateBody(&body_def_plat11);
	plat12_body_ = world_->CreateBody(&body_def_plat12);
	plat13_body_ = world_->CreateBody(&body_def_plat13);
	plat14_body_ = world_->CreateBody(&body_def_plat14);
	plat15_body_ = world_->CreateBody(&body_def_plat15);
	plat16_body_ = world_->CreateBody(&body_def_plat16);
	plat17_body_ = world_->CreateBody(&body_def_plat17);

	// create the shape
	b2PolygonShape shape_plat1;
	shape_plat1.SetAsBox(plat1_dimensions.x(), plat1_dimensions.y());

	b2PolygonShape shape_plat2;
	shape_plat2.SetAsBox(plat2_dimensions.x(), plat2_dimensions.y());

	b2PolygonShape shape_plat3;
	shape_plat3.SetAsBox(plat3_dimensions.x(), plat3_dimensions.y());

	b2PolygonShape shape_plat4;
	shape_plat4.SetAsBox(plat4_dimensions.x(), plat4_dimensions.y());

	b2PolygonShape shape_plat5;
	shape_plat5.SetAsBox(plat5_dimensions.x(), plat5_dimensions.y());

	b2PolygonShape shape_plat6;
	shape_plat6.SetAsBox(plat6_dimensions.x(), plat6_dimensions.y());

	b2PolygonShape shape_plat7;
	shape_plat7.SetAsBox(plat7_dimensions.x(), plat7_dimensions.y());

	b2PolygonShape shape_plat8;
	shape_plat8.SetAsBox(plat8_dimensions.x(), plat8_dimensions.y());

	b2PolygonShape shape_plat9;
	shape_plat9.SetAsBox(plat9_dimensions.x(), plat9_dimensions.y());

	b2PolygonShape shape_plat10;
	shape_plat10.SetAsBox(plat10_dimensions.x(), plat10_dimensions.y());

	b2PolygonShape shape_plat11;
	shape_plat11.SetAsBox(plat11_dimensions.x(), plat11_dimensions.y());

	b2PolygonShape shape_plat12;
	shape_plat12.SetAsBox(plat12_dimensions.x(), plat12_dimensions.y());

	b2PolygonShape shape_plat13;
	shape_plat13.SetAsBox(plat13_dimensions.x(), plat13_dimensions.y());

	b2PolygonShape shape_plat14;
	shape_plat14.SetAsBox(plat14_dimensions.x(), plat14_dimensions.y());

	b2PolygonShape shape_plat15;
	shape_plat15.SetAsBox(plat15_dimensions.x(), plat15_dimensions.y());

	b2PolygonShape shape_plat16;
	shape_plat16.SetAsBox(plat16_dimensions.x(), plat16_dimensions.y());

	b2PolygonShape shape_plat17;
	shape_plat17.SetAsBox(plat17_dimensions.x(), plat17_dimensions.y());

	// create the fixture
	b2FixtureDef fixture_def_plat1;
	fixture_def_plat1.shape = &shape_plat1;

	b2FixtureDef fixture_def_plat2;
	fixture_def_plat2.shape = &shape_plat2;

	b2FixtureDef fixture_def_plat3;
	fixture_def_plat3.shape = &shape_plat3;

	b2FixtureDef fixture_def_plat4;
	fixture_def_plat4.shape = &shape_plat4;

	b2FixtureDef fixture_def_plat5;
	fixture_def_plat5.shape = &shape_plat5;

	b2FixtureDef fixture_def_plat6;
	fixture_def_plat6.shape = &shape_plat6;

	b2FixtureDef fixture_def_plat7;
	fixture_def_plat7.shape = &shape_plat7;

	b2FixtureDef fixture_def_plat8;
	fixture_def_plat8.shape = &shape_plat8;

	b2FixtureDef fixture_def_plat9;
	fixture_def_plat9.shape = &shape_plat9;

	b2FixtureDef fixture_def_plat10;
	fixture_def_plat10.shape = &shape_plat10;

	b2FixtureDef fixture_def_plat11;
	fixture_def_plat11.shape = &shape_plat11;

	b2FixtureDef fixture_def_plat12;
	fixture_def_plat12.shape = &shape_plat12;

	b2FixtureDef fixture_def_plat13;
	fixture_def_plat13.shape = &shape_plat13;

	b2FixtureDef fixture_def_plat14;
	fixture_def_plat14.shape = &shape_plat14;

	b2FixtureDef fixture_def_plat15;
	fixture_def_plat15.shape = &shape_plat15;

	b2FixtureDef fixture_def_plat16;
	fixture_def_plat16.shape = &shape_plat16;

	b2FixtureDef fixture_def_plat17;
	fixture_def_plat17.shape = &shape_plat17;

	// create the fixture on the rigid body
	plat1_body_->CreateFixture(&fixture_def_plat1);
	plat2_body_->CreateFixture(&fixture_def_plat2);
	plat3_body_->CreateFixture(&fixture_def_plat3);
	plat4_body_->CreateFixture(&fixture_def_plat4);
	plat5_body_->CreateFixture(&fixture_def_plat5);
	plat6_body_->CreateFixture(&fixture_def_plat6);
	plat7_body_->CreateFixture(&fixture_def_plat7);
	plat8_body_->CreateFixture(&fixture_def_plat8);
	plat9_body_->CreateFixture(&fixture_def_plat9);
	plat10_body_->CreateFixture(&fixture_def_plat10);
	plat11_body_->CreateFixture(&fixture_def_plat11);
	plat12_body_->CreateFixture(&fixture_def_plat12);
	plat13_body_->CreateFixture(&fixture_def_plat13);
	plat14_body_->CreateFixture(&fixture_def_plat14);
	plat15_body_->CreateFixture(&fixture_def_plat15);
	plat16_body_->CreateFixture(&fixture_def_plat16);
	plat17_body_->CreateFixture(&fixture_def_plat17);

	// update visuals from simulation data
	plat1.UpdateFromSimulation(plat1_body_);
	plat2.UpdateFromSimulation(plat2_body_);
	plat3.UpdateFromSimulation(plat3_body_);
	plat4.UpdateFromSimulation(plat4_body_);
	plat5.UpdateFromSimulation(plat5_body_);
	plat6.UpdateFromSimulation(plat6_body_);
	plat7.UpdateFromSimulation(plat7_body_);
	plat8.UpdateFromSimulation(plat8_body_);
	plat9.UpdateFromSimulation(plat9_body_);
	plat10.UpdateFromSimulation(plat10_body_);
	plat11.UpdateFromSimulation(plat11_body_);
	plat12.UpdateFromSimulation(plat12_body_);
	plat13.UpdateFromSimulation(plat13_body_);
	plat14.UpdateFromSimulation(plat14_body_);
	plat15.UpdateFromSimulation(plat15_body_);
	plat16.UpdateFromSimulation(plat16_body_);
	plat17.UpdateFromSimulation(plat17_body_);
}


void SceneApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void SceneApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void SceneApp::DrawHUD()
{
	time += 1 / fps_;
	if(font_)
	{
		// display frame rate
		font_->RenderText(sprite_renderer_, gef::Vector4(810.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "TIME: %.1f", time);
	}
}

void SceneApp::SetupLights()
{
	// grab the data for the default shader used for rendering 3D geometry
	gef::Default3DShaderData& default_shader_data = renderer_3d_->default_shader_data();

	// set the ambient light
	default_shader_data.set_ambient_light_colour(gef::Colour(0.25f, 0.25f, 0.25f, 1.0f));

	// add a point light that is almost white, but with a blue tinge
	// the position of the light is set far away so it acts light a directional light
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 1.0f, 1.0f));
	default_point_light.set_position(gef::Vector4(-500.0f, 400.0f, 700.0f));
	default_shader_data.AddPointLight(default_point_light);
}

void SceneApp::UpdateSimulation(float frame_time)
{

	//gef::DebugOut("%.f \n", player_body_->GetLinearVelocity().x);
	//gef::DebugOut("%.11f \n", player_body_->GetPosition().y);

	player_body_->SetAngularVelocity(0);

	if (player_body_->GetLinearVelocity().x < 4) {
		player_body_->ApplyLinearImpulseToCenter(b2Vec2(0.5f, 0.0f), true);
	}

	else if (player_body_->GetLinearVelocity().x < 15) {
		player_body_->ApplyLinearImpulseToCenter(b2Vec2(0.04f, 0.0f), true);
	}

	camera_pos = player_body_->GetPosition().x;
	camera_pos = player_body_->GetPosition().x;
	// update physics world

	float32 timeStep = 1.0f / 60.0f;

	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	world_->Step(timeStep, velocityIterations, positionIterations);

	// update object visuals from simulation data
	player_.UpdateFromSimulation(player_body_);

	// don't have to update the ground visuals as it is static

	// collision detection
	// get the head of the contact list
	b2Contact* contact = world_->GetContactList();
	// get contact count
	int contact_count = world_->GetContactCount();
	
	if (player_body_->GetPosition().y < -8) {
		game_state_ = FINISH_SCREEN;
		FinishInit();
	}

	if (player_body_->GetPosition().x > 404) {
		win = true;
		game_state_ = FINISH_SCREEN;
		FinishInit();
	}

	for (int contact_num = 0; contact_num<contact_count; ++contact_num)
	{
		if (contact->IsTouching())
		{
			// get the colliding bodies
			b2Body* bodyA = contact->GetFixtureA()->GetBody();
			b2Body* bodyB = contact->GetFixtureB()->GetBody();

			// DO COLLISION RESPONSE HERE
			Player* player = NULL;

			GameObject* gameObjectA = NULL;
			GameObject* gameObjectB = NULL;

			gameObjectA = (GameObject*)bodyA->GetUserData();
			gameObjectB = (GameObject*)bodyB->GetUserData();

			if (gameObjectA != NULL && gameObjectB != NULL)
			{
				if (gameObjectA->type() == TRAMPOLINE && gameObjectB->type() == PLAYER)
				{
					player_body_->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
					player_body_->ApplyLinearImpulseToCenter(b2Vec2(12.0f, 12.0f), true);
				}
				else if (gameObjectB->type() == TRAMPOLINE && gameObjectA->type() == PLAYER)
				{
					player_body_->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
					player_body_->ApplyLinearImpulseToCenter(b2Vec2(12.0f, 12.0f), true);
				}
			}
		}

		// Get next contact point
		contact = contact->GetNext();

		const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
		gef::Keyboard* keyboard = input_manager_->keyboard();

		if (controller->buttons_pressed() && gef_SONY_CTRL_CROSS || (keyboard->IsKeyPressed(gef::Keyboard::KC_X))) {
			player_body_->ApplyLinearImpulseToCenter(b2Vec2(0.0f, 7.0f), true);
		}
	}
}

void SceneApp::FrontendInit()
{
	button_icon_cross = CreateTextureFromPNG("playstation-cross-dark-icon.png", platform_);
	start_selected = true;
}

void SceneApp::FrontendRelease()
{
	delete button_icon_cross;
	button_icon_cross = NULL;
}

void SceneApp::FrontendUpdate(float frame_time)
{
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
	gef::Keyboard* keyboard = input_manager_->keyboard();

	if (controller->buttons_pressed() && gef_SONY_CTRL_CROSS || (keyboard->IsKeyPressed(gef::Keyboard::KC_X)))
	{	
		if (start_selected == true )
		{
			FrontendRelease();
			game_state_ = PLAY_GAME;
			GameInit();
		}
		else {
			FrontendRelease();
			game_state_ = GAME_OPTIONS;
			GameOptionsInit();
		}
	}




	if (controller->buttons_pressed() && gef_SONY_CTRL_DOWN && start_selected == true || (keyboard->IsKeyPressed(gef::Keyboard::KC_DOWN)) && start_selected == true)
	{
		start_selected = false;
	}

	if (controller->buttons_pressed() && gef_SONY_CTRL_UP & start_selected == false || (keyboard->IsKeyPressed(gef::Keyboard::KC_UP)) && start_selected == false)
	{
		start_selected = true;
	}
}

void SceneApp::FrontendRender()
{
	sprite_renderer_->Begin();

	if (start_selected == true) {
	// render selected START THE GAME
	font_->RenderText(
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
		1.5f,
		0xffffffff,
		gef::TJ_CENTRE,
		"START THE GAME");

	// render unselected OPTIONS
	font_->RenderText(
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 32.0f, -0.99f),
		1.0f,
		0xff000000,
		gef::TJ_CENTRE,
		"OPTIONS");
	}
	else{
			// render unselected START THE GAME
			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
				1.0f,
				0xff000000,
				gef::TJ_CENTRE,
				"START THE GAME");

			// render selected OPTIONS
			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 32.0f, -0.99f),
				1.5f,
				0xffffffff,
				gef::TJ_CENTRE,
				"OPTIONS");
		}
	sprite_renderer_->End();
}

void SceneApp::GameInit()
{
	time = 0;
	camera_pos = 0;

	// create the renderer for draw 3D geometry
	renderer_3d_ = gef::Renderer3D::Create(platform_);

	// initialise primitive builder to make create some 3D geometry easier
	primitive_builder_ = new PrimitiveBuilder(platform_);


	SetupLights();

	// initialise the physics world
	b2Vec2 gravity(0.0f, -9.81f);
	world_ = new b2World(gravity);

	InitPlayer();
	InitGround();
	InitPlatforms();
	InitTrampoline();

	// load audio assets
	if (audio_manager_)
	{
		// load a sound effect
		sfx_id_ = audio_manager_->LoadSample("box_collected.wav", platform_);

		// load in music
		audio_manager_->LoadMusic("music.wav", platform_);

		// play music
		audio_manager_->PlayMusic();

		music_playing_ = true;
	}
}

void SceneApp::GameRelease()
{
	
	// unload audio resources
	if (audio_manager_)
	{
		audio_manager_->StopMusic();
		audio_manager_->UnloadAllSamples();
		sfx_id_ = -1;
		sfx_voice_id_ = -1;
	}

	// destroying the physics world also destroys all the objects within it
	delete ground_mesh_;
	ground_mesh_ = NULL;
	
	delete ground_mesh_2;
	ground_mesh_2 = NULL;

	delete ground_mesh_3;
	ground_mesh_3 = NULL;

	delete ground_mesh_4;
	ground_mesh_4 = NULL;

	delete ground_mesh_5;
	ground_mesh_5 = NULL;

	delete tramp_mesh_;
	tramp_mesh_ = NULL;

	delete tramp_mesh_2;
	tramp_mesh_2 = NULL;

	delete tramp_mesh_3;
	tramp_mesh_3 = NULL;

	delete world_;
	world_ = NULL;	

	delete plat1_mesh_;
	plat1_mesh_ = NULL;

	delete plat2_mesh_;
	plat2_mesh_ = NULL;

	delete plat3_mesh_;
	plat3_mesh_ = NULL;

	delete plat4_mesh_;
	plat4_mesh_ = NULL;

	delete plat5_mesh_;
	plat5_mesh_ = NULL;

	delete plat6_mesh_;
	plat6_mesh_ = NULL;

	delete plat7_mesh_;
	plat7_mesh_ = NULL;

	delete plat8_mesh_;
	plat8_mesh_ = NULL;

	delete plat9_mesh_;
	plat9_mesh_ = NULL;

	delete plat10_mesh_;
	plat10_mesh_ = NULL;

	delete plat11_mesh_;
	plat11_mesh_ = NULL;

	delete plat12_mesh_;
	plat12_mesh_ = NULL;
	
	delete plat13_mesh_;
	plat13_mesh_ = NULL;
	
	delete plat14_mesh_;
	plat14_mesh_ = NULL;
	
	delete plat15_mesh_;
	plat15_mesh_ = NULL;
	
	delete plat16_mesh_;
	plat16_mesh_ = NULL;
	
	delete plat17_mesh_;
	plat17_mesh_ = NULL;
	
	delete plat18_mesh_;
	plat18_mesh_ = NULL;
	
	delete plat19_mesh_;
	plat19_mesh_ = NULL;

	delete plat20_mesh_;
	plat20_mesh_ = NULL;

	delete plat21_mesh_;
	plat21_mesh_ = NULL;

	delete plat22_mesh_;
	plat22_mesh_ = NULL;

	delete plat23_mesh_;
	plat23_mesh_ = NULL;

	delete plat24_mesh_;
	plat24_mesh_ = NULL;

	delete plat25_mesh_;
	plat25_mesh_ = NULL;

	delete plat26_mesh_;
	plat26_mesh_ = NULL;

	delete plat27_mesh_;
	plat27_mesh_ = NULL;

	delete plat28_mesh_;
	plat28_mesh_ = NULL;

	delete plat29_mesh_;
	plat29_mesh_ = NULL;

	delete primitive_builder_;
	primitive_builder_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

}

void SceneApp::GameUpdate(float frame_time)
{
	gef::Keyboard* keyboard = input_manager_->keyboard();
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);

	// trigger a sound effect

	UpdateSimulation(frame_time);
	UpdateAudio(frame_time);

	if (controller->buttons_pressed() && gef_SONY_CTRL_START || (keyboard->IsKeyPressed(gef::Keyboard::KC_P)))
	{
		is_paused = true;
		game_state_ = PAUSE_SCREEN;
		PausescreenInit();
	}


}

void SceneApp::GameRender()
{
	// setup camera

	// projection
	float fov = gef::DegToRad(45.0f);
	float aspect_ratio = (float)platform_.width() / (float)platform_.height();
	gef::Matrix44 projection_matrix;
	projection_matrix = platform_.PerspectiveProjectionFov(fov, aspect_ratio, 0.1f, 100.0f);
	renderer_3d_->set_projection_matrix(projection_matrix);

	// view
	gef::Vector4 camera_eye(camera_pos, 2.0f, 15.0f);
	gef::Vector4 camera_lookat(camera_pos, 0.0f, 0.0f);
	gef::Vector4 camera_up(0.0f, 1.0f, 0.0f);
	gef::Matrix44 view_matrix;
	view_matrix.LookAt(camera_eye, camera_lookat, camera_up);
	renderer_3d_->set_view_matrix(view_matrix);


	// draw 3d geometry
	renderer_3d_->Begin();

	// draw ground
	renderer_3d_->DrawMesh(ground_);
	renderer_3d_->DrawMesh(ground_2);
	renderer_3d_->DrawMesh(ground_3);
	renderer_3d_->DrawMesh(ground_4);
	renderer_3d_->DrawMesh(ground_5);

	//draw platforms
	renderer_3d_->DrawMesh(plat1);
	renderer_3d_->DrawMesh(plat2);
	renderer_3d_->DrawMesh(plat3);
	renderer_3d_->DrawMesh(plat4);
	renderer_3d_->DrawMesh(plat5);
	renderer_3d_->DrawMesh(plat6);
	renderer_3d_->DrawMesh(plat7);
	renderer_3d_->DrawMesh(plat8);
	renderer_3d_->DrawMesh(plat9);
	renderer_3d_->DrawMesh(plat10);
	renderer_3d_->DrawMesh(plat11);
	renderer_3d_->DrawMesh(plat12);
	renderer_3d_->DrawMesh(plat13);
	renderer_3d_->DrawMesh(plat14);
	renderer_3d_->DrawMesh(plat15);
	renderer_3d_->DrawMesh(plat16);

	// draw player
	if(color == "RED"){
		renderer_3d_->set_override_material(&primitive_builder_->red_material());
	}
	if (color == "BLUE")
	{
		renderer_3d_->set_override_material(&primitive_builder_->blue_material());
	}

	renderer_3d_->DrawMesh(player_);
	renderer_3d_->DrawMesh(tramp_);
	renderer_3d_->DrawMesh(tramp_2);
	renderer_3d_->DrawMesh(tramp_3);
	renderer_3d_->set_override_material(NULL);

	renderer_3d_->set_override_material(&primitive_builder_->green_material());
	renderer_3d_->DrawMesh(plat17);
	renderer_3d_->set_override_material(NULL);




	renderer_3d_->End();

	// start drawing sprites, but don't clear the frame buffer
	sprite_renderer_->Begin(false);
	DrawHUD();
	sprite_renderer_->End();
}

void SceneApp::GameOptionsInit()
{
	button_icon_circle = CreateTextureFromPNG("playstation-circle-dark-icon.png", platform_);
	sound_selected = true;
}

void SceneApp::GameOptionsRelease()
{
	delete button_icon_circle;
	button_icon_circle = NULL;
}

void SceneApp::GameOptionsUpdate(float frame_time)
{
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
	gef::Keyboard* keyboard = input_manager_->keyboard();

	if (controller->buttons_pressed() & gef_SONY_CTRL_CIRCLE || (keyboard->IsKeyPressed(gef::Keyboard::KC_B))) {
		if (is_paused == false)
		{
			GameOptionsRelease();
			game_state_ = FRONTEND;
			FrontendInit();
		}
		if (is_paused == true)
		{
			GameOptionsRelease();
			game_state_ = PAUSE_SCREEN;
			PausescreenInit();
		}
	}

	if (sound_selected == true) {
		if (sound_volume_ >= -0.5f & sound_volume_ <= 1.0f)
		{
			if (sound_volume_ < 1.0f) {
				if (controller->buttons_pressed() & gef_SONY_CTRL_RIGHT || (keyboard->IsKeyPressed(gef::Keyboard::KC_RIGHT)))
				{
					sound_volume_ = sound_volume_ + 0.1;
					GameOptionsRender();
				}
			}
			
			if (controller->buttons_pressed() & gef_SONY_CTRL_LEFT || (keyboard->IsKeyPressed(gef::Keyboard::KC_LEFT)))
			{
				if (sound_volume_ > 0.0f)
				{
					sound_volume_ = sound_volume_ - 0.1;
					GameOptionsRender();
		
				}
				

			}
		}

	}
	else {

		if (controller->buttons_pressed() && gef_SONY_CTRL_LEFT && color == "RED" || (keyboard->IsKeyPressed(gef::Keyboard::KC_LEFT)) && color == "RED")
		{
			color = "BLUE";
			GameOptionsRender();
		}
	
		else if (controller->buttons_pressed() && gef_SONY_CTRL_LEFT && color == "BLUE" || (keyboard->IsKeyPressed(gef::Keyboard::KC_LEFT)) && color == "BLUE")
		{
			color = "RED";
			GameOptionsRender();
		}

		if (controller->buttons_pressed() && gef_SONY_CTRL_RIGHT && color == "RED" || (keyboard->IsKeyPressed(gef::Keyboard::KC_RIGHT)) && color == "RED")
		{
			color = "BLUE";
			GameOptionsRender();
		}
		else if (controller->buttons_pressed() && gef_SONY_CTRL_RIGHT && color == "BLUE" || (keyboard->IsKeyPressed(gef::Keyboard::KC_RIGHT)) && color == "BLUE")
		{
			color = "RED";
			GameOptionsRender();
		}

	}

	if (controller->buttons_pressed() && gef_SONY_CTRL_DOWN && sound_selected == true || (keyboard->IsKeyPressed(gef::Keyboard::KC_DOWN)) && sound_selected == true)
	{
		sound_selected = false;
	}

	if (controller->buttons_pressed() && gef_SONY_CTRL_UP & sound_selected == false || (keyboard->IsKeyPressed(gef::Keyboard::KC_UP)) && sound_selected == false)
	{
		sound_selected = true;

	}
}

void SceneApp::GameOptionsRender()
{
	sprite_renderer_->Begin();
	if (is_paused == false) {
		if (sound_selected == true) {


			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
				1.5f,
				0xffffffff,
				gef::TJ_CENTRE,
				"SOUND : %.0f", fabs(sound_volume_ * 10));


			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 16.0f, -0.99f),
				1.0f,
				0xff000000,
				gef::TJ_CENTRE,
				"CUBE COLOR: %s", color);
		}
		else {
			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
				1.0f,
				0xff000000,
				gef::TJ_CENTRE,
				"SOUND : %.0f", sound_volume_ * 10);

			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 16.0f, -0.99f),
				1.5f,
				0xffffffff,
				gef::TJ_CENTRE,
				"CUBE COLOR: %s", color);
		}
	}
	else {

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
			1.5f,
			0xffffffff,
			gef::TJ_CENTRE,
			"SOUND : %.0f", fabs(sound_volume_ * 10));
	}

	//render circle icon
	gef::Sprite button_back;
	button_back.set_texture(button_icon_circle);
	button_back.set_position(gef::Vector4(platform_.width()*0.5f + 200.0f, platform_.height()*0.5f + 170.0f, -0.99f));
	button_back.set_height(32.0f);
	button_back.set_width(32.0f);
	sprite_renderer_->DrawSprite(button_back);

	font_->RenderText(
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.5f + 250.0f, platform_.height()*0.5f + 150.0f, -0.99f),
		1.0f,
		0xffffffff,
		gef::TJ_CENTRE,
		"BACK");

	sprite_renderer_->End();
}

void SceneApp::FinishInit()
{
	button_icon_cross = CreateTextureFromPNG("playstation-cross-dark-icon.png", platform_);
	retry_selected = true;
}

void SceneApp::FinishRelease()
{
	delete button_icon_circle;
	button_icon_circle = NULL;

	win = false;
}

void SceneApp::FinishUpdate(float frame_time)
{
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
	gef::Keyboard* keyboard = input_manager_->keyboard();
	if (win == false) {
		if (controller->buttons_pressed() && gef_SONY_CTRL_DOWN && retry_selected == true || (keyboard->IsKeyPressed(gef::Keyboard::KC_DOWN)) && retry_selected == true)
		{
			retry_selected = false;
		}

		else if (controller->buttons_pressed() && gef_SONY_CTRL_UP && retry_selected == false || (keyboard->IsKeyPressed(gef::Keyboard::KC_UP)) && retry_selected == false)
		{
			retry_selected = true;

		}
	}

	if (controller->buttons_pressed() && gef_SONY_CTRL_CROSS || (keyboard->IsKeyPressed(gef::Keyboard::KC_X)))
	{
		if (win == true)
		{
			FinishRelease();
			game_state_ = FRONTEND;
			FrontendInit();
		}
		else {
			if (retry_selected == true) {

				FinishRelease();
				game_state_ = PLAY_GAME;
				GameInit();
			}
			else {
				FinishRelease();
				game_state_ = FRONTEND;
				FrontendInit();
			}
		}
	}

}

void SceneApp::FinishRender()
{
	sprite_renderer_->Begin();
	if (win == true)
	{
		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
			1.5f,
			0xffffffff,
			gef::TJ_CENTRE,
			"You have reached the finish line in %.1fs, congratulations!", time);

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 16.0f, -0.99f),
			1.5f,
			0xffffffff,
			gef::TJ_CENTRE,
			"Developer's record is 30.8s, try to beat it !");

		gef::Sprite button_continue;
		button_continue.set_texture(button_icon_cross);
		button_continue.set_position(gef::Vector4(platform_.width()*0.5f + 240.0f, platform_.height()*0.5f + 130.0f, -0.99f));
		button_continue.set_height(32.0f);
		button_continue.set_width(32.0f);
		sprite_renderer_->DrawSprite(button_continue);

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f + 250.0f, platform_.height()*0.5f + 150.0f, -0.99f),
			1.0f,
			0xffffffff,
			gef::TJ_CENTRE,
			"Continue");
	}
	else if (win == false) {
		if (retry_selected == true) {
			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 86.0f, -0.99f),
				1.25f,
				0xff000000,
				gef::TJ_CENTRE,
				"You have lost, too bad!");

			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 25.0f, -0.99f),
				1.5f,
				0xffffffff,
				gef::TJ_CENTRE,
				"RETRY");

			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 14.0f, -0.99f),
				1.0f,
				0xff000000,
				gef::TJ_CENTRE,
				"QUIT");
		}
		else if (retry_selected == false) {

			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 86.0f, -0.99f),
				1.25f,
				0xff000000,
				gef::TJ_CENTRE,
				"You have lost, too bad!");

			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 25.0f, -0.99f),
				1.0f,
				0xff000000,
				gef::TJ_CENTRE,
				"RETRY");

			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 14.0f, -0.99f),
				1.5f,
				0xffffffff,
				gef::TJ_CENTRE,
				"QUIT");

		}
	}
	sprite_renderer_->End();
}

void SceneApp::PausescreenInit()
{
	continue_selected = true;
}

void SceneApp::PausescreenRelease()
{
}

void SceneApp::PausescreenUpdate(float frame_time)
{
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
	gef::Keyboard* keyboard = input_manager_->keyboard();

	if (controller->buttons_pressed() & gef_SONY_CTRL_DOWN || (keyboard->IsKeyPressed(gef::Keyboard::KC_DOWN)))
	{
		if (continue_selected == true)
		{
			continue_selected = false;
			options_selected = true;
		}
		else if (options_selected == true) {
			options_selected = false;
		}
	}

	else if (controller->buttons_pressed() & gef_SONY_CTRL_UP || (keyboard->IsKeyPressed(gef::Keyboard::KC_UP)))
	{
		if (options_selected == true)
		{
			options_selected = false;
			continue_selected = true;

		}
		if (options_selected == false && continue_selected == false)
		{
			options_selected = true;
		}
	}

	if (controller->buttons_pressed() & gef_SONY_CTRL_CROSS || (keyboard->IsKeyPressed(gef::Keyboard::KC_X)))
	{
		if (continue_selected == true)
		{
			game_state_ = PLAY_GAME;
			//GameInit();
			is_paused = false;
		}
		else if (options_selected == true) {

			game_state_ = GAME_OPTIONS;
			GameOptionsInit();
		}
		else if (options_selected == false && continue_selected == false)
		{
			game_state_ = FRONTEND;
			FrontendInit();
			is_paused = false;
		}
	}

}
	


void SceneApp::PausescreenRender()
{

	sprite_renderer_->Begin();
	if (continue_selected == true) {


		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
			1.5f,
			0xffffffff,
			gef::TJ_CENTRE,
			"CONTINUE");


		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 14.0f, -0.99f),
			1.0f,
			0xff000000,
			gef::TJ_CENTRE,
			"OPTIONS");

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 20.0f, -0.99f),
			1.0f,
			0xff000000,
			gef::TJ_CENTRE,
			"QUIT");

	}
	else if(options_selected == true) {

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
			1.0f,
			0xff000000,
			gef::TJ_CENTRE,
			"CONTINUE");

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 25.0f, -0.99f),
			1.5f,
			0xffffffff,
			gef::TJ_CENTRE,
			"OPTIONS");

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 20.0f, -0.99f),
			1.0f,
			0xff000000,
			gef::TJ_CENTRE,
			"QUIT");
	}
	else if (options_selected == false && continue_selected == false)
	{
		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
			1.0f,
			0xff000000,
			gef::TJ_CENTRE,
			"CONTINUE");

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 14.0f, -0.99f),
			1.0f,
			0xff000000,
			gef::TJ_CENTRE,
			"OPTIONS");

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 20.0f, -0.99f),
			1.5f,
			0xffffffff,
			gef::TJ_CENTRE,
			"QUIT");
	}
	

	sprite_renderer_->End();
}

void SceneApp::UpdateAudio(float frame_time) {

	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);

	if (music_playing_ == true)
	{
		gef::VolumeInfo volume_info;
		volume_info.volume = sound_volume_;
		volume_info.pan = sound_volume_;

		if (sfx_id_ != -1) {
			audio_manager_->SetSampleVoiceVolumeInfo(sfx_voice_id_, volume_info);
		}

		audio_manager_->SetMusicVolumeInfo(volume_info);
	}

}
