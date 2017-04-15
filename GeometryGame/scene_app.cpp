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
	sound_volume_(1.0)
{
}

void SceneApp::Init()
{
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	InitFont();

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
	gef::Vector4 ground_half_dimensions(100.0f, 0.5f, 0.5f);

	// setup the mesh for the ground
	ground_mesh_ = primitive_builder_->CreateBoxMesh(ground_half_dimensions);
	ground_.set_mesh(ground_mesh_);

	// create a physics body
	b2BodyDef body_def;
	body_def.type = b2_staticBody;
	body_def.position = b2Vec2(0.0f, 0.0f);

	ground_body_ = world_->CreateBody(&body_def);

	// create the shape
	b2PolygonShape shape;
	shape.SetAsBox(ground_half_dimensions.x(), ground_half_dimensions.y());

	// create the fixture
	b2FixtureDef fixture_def;
	fixture_def.shape = &shape;

	// create the fixture on the rigid body
	ground_body_->CreateFixture(&fixture_def);

	// update visuals from simulation data
	ground_.UpdateFromSimulation(ground_body_);
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
	if(font_)
	{
		// display frame rate
		font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "FPS: %.1f", fps_);
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

			if (gameObjectA)
			{
				if (gameObjectA->type() == PLAYER)
				{
					player = (Player*)bodyA->GetUserData();
				}
			}

			if (gameObjectB)
			{
				if (gameObjectB->type() == PLAYER)
				{
					player = (Player*)bodyB->GetUserData();
				}
			}

			if (player)
			{
				player->DecrementHealth();
			}
		}

		const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
		gef::Keyboard* keyboard = input_manager_->keyboard();

		if (controller->buttons_pressed() && gef_SONY_CTRL_CROSS || (keyboard->IsKeyPressed(gef::Keyboard::KC_X))) {
			player_body_->ApplyLinearImpulse(b2Vec2(0.0f, 1.0f), player_body_->GetPosition(), true);
		}


		// Get next contact point
		contact = contact->GetNext();
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


	DrawHUD();
	sprite_renderer_->End();
}

void SceneApp::GameInit()
{
	camera_pos = 0;
	// create the renderer for draw 3D geometry
	renderer_3d_ = gef::Renderer3D::Create(platform_);

	// initialise primitive builder to make create some 3D geometry easier
	primitive_builder_ = new PrimitiveBuilder(platform_);


	SetupLights();

	// initialise the physics world
	b2Vec2 gravity(5.0f, -9.81f);
	world_ = new b2World(gravity);

	InitPlayer();
	InitGround();

	// load audio assets
	if (audio_manager_)
	{
		// load a sound effect
		sfx_id_ = audio_manager_->LoadSample("box_collected.wav", platform_);

		// load in music
		audio_manager_->LoadMusic("music.wav", platform_);

		// play music
		audio_manager_->PlayMusic();
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
	delete world_;
	world_ = NULL;

	delete ground_mesh_;
	ground_mesh_ = NULL;

	delete primitive_builder_;
	primitive_builder_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

}

void SceneApp::GameUpdate(float frame_time)
{

	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);

	// trigger a sound effect

	UpdateSimulation(frame_time);

	/*if (controller->buttons_pressed && gef_SONY_CTRL_OPTIONS)
	{
		game_state_ = FRONTEND;
	}*/


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
	gef::Vector4 camera_eye(camera_pos, 2.0f, 10.0f);
	gef::Vector4 camera_lookat(camera_pos, 0.0f, 0.0f);
	gef::Vector4 camera_up(0.0f, 1.0f, 0.0f);
	gef::Matrix44 view_matrix;
	view_matrix.LookAt(camera_eye, camera_lookat, camera_up);
	renderer_3d_->set_view_matrix(view_matrix);


	// draw 3d geometry
	renderer_3d_->Begin();

	// draw ground
	renderer_3d_->DrawMesh(ground_);

	// draw player
	if(color == "RED"){
		renderer_3d_->set_override_material(&primitive_builder_->red_material());
	}
	if (color == "BLUE")
	{
		renderer_3d_->set_override_material(&primitive_builder_->blue_material());
	}
	if (color == "GREEN")
	{
		renderer_3d_->set_override_material(&primitive_builder_->green_material());
	}
	renderer_3d_->DrawMesh(player_);
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
		/*if (is_paused == true)
		{
			GameOptionsRelease();
			game_state_ = PAUSE;
			PauseInit();
		}*/
	}

	if (sound_selected == true) {
		if (sound_volume_ >= 0.0f & sound_volume_ <= 1.0f)
		{
			if (controller->buttons_pressed() & gef_SONY_CTRL_RIGHT || (keyboard->IsKeyPressed(gef::Keyboard::KC_RIGHT)))
			{
				sound_volume_ = sound_volume_ + 0.1;
				GameOptionsRender();
				//UpdateAudio(frame_time);
			}

			if (controller->buttons_pressed() & gef_SONY_CTRL_LEFT || (keyboard->IsKeyPressed(gef::Keyboard::KC_LEFT)))
			{
				sound_volume_ = sound_volume_ - 0.1;
				GameOptionsRender();
				//UpdateAudio(frame_time);

			}
		}
		else if (sound_volume_ > 1.0f)
		{
			sound_volume_ = 1.0f;
			GameOptionsRender();
			//UpdateAudio(frame_time);
		}
		else if (sound_volume_ < 0.0f)
		{
			sound_volume_ = 0.0f;
			GameOptionsRender();
			//UpdateAudio(frame_time);
		}
	}
	else {

		if (controller->buttons_pressed() && gef_SONY_CTRL_LEFT && color == "RED" || (keyboard->IsKeyPressed(gef::Keyboard::KC_LEFT)) && color == "RED")
		{
			color = "GREEN";
			GameOptionsRender();
		}
		else if (controller->buttons_pressed() && gef_SONY_CTRL_LEFT && color == "GREEN" || (keyboard->IsKeyPressed(gef::Keyboard::KC_LEFT)) && color == "GREEN")
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
			color = "GREEN";
			GameOptionsRender();
		}
		else if (controller->buttons_pressed() && gef_SONY_CTRL_RIGHT && color == "GREEN" || (keyboard->IsKeyPressed(gef::Keyboard::KC_RIGHT)) && color == "GREEN")
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
	if (sound_selected == true) {


		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
			1.5f,
			0xffffffff,
			gef::TJ_CENTRE,
			"SOUND : %.0f", sound_volume_ * 10);


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
