#ifndef _SCENE_APP_H
#define _SCENE_APP_H

#include <system/application.h>
#include <maths/vector2.h>
#include "primitive_builder.h"
#include <graphics/mesh_instance.h>
#include <audio/audio_manager.h>
#include <input/input_manager.h>
#include <box2d/Box2D.h>
#include "game_object.h"


// FRAMEWORK FORWARD DECLARATIONS

enum GAMESTATE 
{
	PLAY_GAME,
	FRONTEND,
	GAME_OPTIONS,
};


namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class InputManager;
	class Renderer3D;
}

class SceneApp : public gef::Application
{
public:
	SceneApp(gef::Platform& platform);
	void Init();
	void CleanUp();
	bool Update(float frame_time);
	void Render();
private:
	void InitPlayer();
	void InitGround();
	void InitFont();
	void InitPlatforms();
	void CleanUpFont();
	void DrawHUD();
	void SetupLights();
	void UpdateSimulation(float frame_time);
    
	gef::SpriteRenderer* sprite_renderer_;
	gef::Font* font_;
	gef::InputManager* input_manager_;
	gef::AudioManager* audio_manager_;

	GAMESTATE game_state_;
	//
	// FRONTEND DECLARATIONS
	//
	gef::Texture* button_icon_cross;
	gef::Texture* button_icon_circle;
	gef::Texture* button_icon_square;
	//
	// GAME DECLARATIONS
	//
	gef::Renderer3D* renderer_3d_;
	PrimitiveBuilder* primitive_builder_;

	// create the physics world
	b2World* world_;

	// player variables
	Player player_;
	b2Body* player_body_;

	// ground variables
	gef::Mesh* ground_mesh_;
	gef::Mesh* ground_mesh_2;

	GameObject ground_;
	GameObject ground_2;

	b2Body* ground_body_;
	b2Body* ground_body_2;

	//platforms variables
	gef::Mesh* plat1_mesh_;
	GameObject plat1;
	b2Body* plat1_body_;

	gef::Mesh* plat2_mesh_;
	GameObject plat2;
	b2Body* plat2_body_;

	gef::Mesh* plat3_mesh_;
	GameObject plat3;
	b2Body* plat3_body_;

	gef::Mesh* plat4_mesh_;
	GameObject plat4;
	b2Body* plat4_body_;

	gef::Mesh* plat5_mesh_;
	GameObject plat5;
	b2Body* plat5_body_;

	gef::Mesh* plat6_mesh_;
	GameObject plat6;
	b2Body* plat6_body_;

	gef::Mesh* plat7_mesh_;
	GameObject plat7;
	b2Body* plat7_body_;

	gef::Mesh* plat8_mesh_;
	GameObject plat8;
	b2Body* plat8_body_;

	gef::Mesh* plat9_mesh_;
	GameObject plat9;
	b2Body* plat9_body_;

	gef::Mesh* plat10_mesh_;
	GameObject plat10;
	b2Body* plat10_body_;

	// Audio variables
	int sfx_id_;
	int sfx_voice_id_;
	float sound_volume_;

	// Menu Variables
	bool start_selected;
	bool sound_selected;
	bool is_paused;
	float fps_;

	char* color;

	void FrontendInit();
	void FrontendRelease();
	void FrontendUpdate(float frame_time);
	void FrontendRender();

	void GameInit();
	void GameRelease();
	void GameUpdate(float frame_time);
	void GameRender();

	void GameOptionsInit();
	void GameOptionsRelease();
	void GameOptionsUpdate(float frame_time);
	void GameOptionsRender();

	// audio update function
	void UpdateAudio(float frame_time);
	int arrondi(float nombre);

	float camera_pos;
	float game_speed;

};

#endif // _SCENE_APP_H
