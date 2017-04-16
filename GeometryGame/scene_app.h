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
	FINISH_SCREEN,
	PAUSE_SCREEN
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
	void InitTrampoline();
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
	gef::Mesh* ground_mesh_3;
	gef::Mesh* ground_mesh_4;
	gef::Mesh* ground_mesh_5;

	GameObject ground_;
	GameObject ground_2;
	GameObject ground_3;
	GameObject ground_4;
	GameObject ground_5;

	b2Body* ground_body_;
	b2Body* ground_body_2;
	b2Body* ground_body_3;
	b2Body* ground_body_4;
	b2Body* ground_body_5;

	//trampolines variable
	gef::Mesh* tramp_mesh_;
	gef::Mesh* tramp_mesh_2;
	gef::Mesh* tramp_mesh_3;

	Trampoline tramp_;
	Trampoline tramp_2;
	Trampoline tramp_3;

	b2Body* tramp_body_;
	b2Body* tramp_body_2;
	b2Body* tramp_body_3;

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

	gef::Mesh* plat11_mesh_;
	GameObject plat11;
	b2Body* plat11_body_;

	gef::Mesh* plat12_mesh_;
	GameObject plat12;
	b2Body* plat12_body_;

	gef::Mesh* plat13_mesh_;
	GameObject plat13;
	b2Body* plat13_body_;

	gef::Mesh* plat14_mesh_;
	GameObject plat14;
	b2Body* plat14_body_;

	gef::Mesh* plat15_mesh_;
	GameObject plat15;
	b2Body* plat15_body_;

	gef::Mesh* plat16_mesh_;
	GameObject plat16;
	b2Body* plat16_body_;

	gef::Mesh* plat17_mesh_;
	GameObject plat17;
	b2Body* plat17_body_;

	gef::Mesh* plat18_mesh_;
	GameObject plat18;
	b2Body* plat18_body_;

	gef::Mesh* plat19_mesh_;
	GameObject plat19;
	b2Body* plat19_body_;

	gef::Mesh* plat20_mesh_;
	GameObject plat20;
	b2Body* plat20_body_;

	gef::Mesh* plat21_mesh_;
	GameObject plat21;
	b2Body* plat21_body_;

	gef::Mesh* plat22_mesh_;
	GameObject plat22;
	b2Body* plat22_body_;

	gef::Mesh* plat23_mesh_;
	GameObject plat23;
	b2Body* plat23_body_;

	gef::Mesh* plat24_mesh_;
	GameObject plat24;
	b2Body* plat24_body_;

	gef::Mesh* plat25_mesh_;
	GameObject plat25;
	b2Body* plat25_body_;

	gef::Mesh* plat26_mesh_;
	GameObject plat26;
	b2Body* plat26_body_;

	gef::Mesh* plat27_mesh_;
	GameObject plat27;
	b2Body* plat27_body_;

	gef::Mesh* plat28_mesh_;
	GameObject plat28;
	b2Body* plat28_body_;

	gef::Mesh* plat29_mesh_;
	GameObject plat29;
	b2Body* plat29_body_;

	// Audio variables
	int sfx_id_;
	int sfx_voice_id_;
	float sound_volume_;

	// Menu Variables
	bool start_selected;
	bool sound_selected;
	bool is_paused;
	bool continue_selected;
	bool options_selected;
	bool retry_selected;
	float fps_;
	bool win;

	

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

	void FinishInit();
	void FinishRelease();
	void FinishUpdate(float frame_time);
	void FinishRender();

	void PausescreenInit();
	void PausescreenRelease();
	void PausescreenUpdate(float frame_time);
	void PausescreenRender();

	// audio update function
	void UpdateAudio(float frame_time);

	float camera_pos;
	float time;

};

#endif // _SCENE_APP_H
