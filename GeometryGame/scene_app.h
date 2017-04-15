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
	GameObject ground_;
	b2Body* ground_body_;

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

	float camera_pos;
};

#endif // _SCENE_APP_H
