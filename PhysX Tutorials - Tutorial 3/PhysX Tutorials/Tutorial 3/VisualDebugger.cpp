#include "VisualDebugger.h"
#include <vector>
#include "Extras\Camera.h"
#include "Extras\Renderer.h"
#include "Extras\HUD.h"

namespace VisualDebugger
{
	using namespace physx;

	enum RenderMode
	{
		DEBUG,
		NORMAL,
		BOTH
	};

	enum HUDState
	{
		EMPTY = 0,
		HELP = 1,
		PAUSE = 2,
		WIN = 3
	};

	//function declarations
	void KeyHold();
	void KeySpecial(int key, int x, int y);
	void KeyRelease(unsigned char key, int x, int y);
	void KeyPress(unsigned char key, int x, int y);

	void motionCallback(int x, int y);
	void mouseCallback(int button, int state, int x, int y);
	void exitCallback(void);

	void RenderScene();
	void ToggleRenderMode();
	void HUDInit();

	///simulation objects
	Camera* camera;
	PhysicsEngine::MyScene* scene;
	PxReal delta_time = 1.f / 60.f;
	PxReal gForceStrength = 20;
	RenderMode render_mode = NORMAL;
	const int MAX_KEYS = 256;
	bool key_state[MAX_KEYS];
	bool hud_show = true;
	HUD hud;
	std::string myForceString; 

	//Init the debugger
	void Init(const char *window_name, int width, int height)
	{
		///Init PhysX
		PhysicsEngine::PxInit();
		scene = new PhysicsEngine::MyScene();
		scene->Init();
	    myForceString = std::to_string(scene->myForce);
		///Init renderer
		Renderer::BackgroundColor(PxVec3(150.f / 255.f, 150.f / 255.f, 150.f / 255.f));
		Renderer::SetRenderDetail(40);
		Renderer::InitWindow(window_name, width, height);
		Renderer::Init();

		camera = new Camera(PxVec3(0.0f, 110.0f, 15.0f), PxVec3(0.f, -100.0f, 1.f), 30.f);

		//initialise HUD
		HUDInit();

		///Assign callbacks
		//render
		glutDisplayFunc(RenderScene);

		//keyboard
		glutKeyboardFunc(KeyPress);
		glutSpecialFunc(KeySpecial);
		glutKeyboardUpFunc(KeyRelease);

		//mouse
		glutMouseFunc(mouseCallback);
		glutMotionFunc(motionCallback);

		//exit
		atexit(exitCallback);

		//init motion callback
		motionCallback(0, 0);
	}

	void HUDInit()
	{
		//initialise HUD
		//add an empty screen
		hud.AddLine(EMPTY, "");
		
		
		hud.AddLine(HELP, "                                                              " + myForceString);

		//add multiple empty lines
		for (int i = 0; i < 17; i++)
		{
			hud.AddLine(HELP, "");
		}
		
		//add a help screen
		hud.AddLine(HELP, "                                                   CRAZY GOLF");
		hud.AddLine(HELP, "");
		hud.AddLine(HELP, "                                                   GAME CONTROLS");
		hud.AddLine(HELP, "                                                   UP Arrow       -    Increase Force");
		hud.AddLine(HELP, "                                                   DOWN Arrow    -    Decrease Force");
		hud.AddLine(HELP, "                                                   R                  -    Apply Force");
		for (int i = 0; i < 5; i++)
		{
			hud.AddLine(HELP, "");
		}
		hud.AddLine(HELP, "                                                   VIEW CONTROLS");
		hud.AddLine(HELP, "                                                   F4 - reset scene");
		hud.AddLine(HELP, "                                                   F5 - help on/off");
		hud.AddLine(HELP, "                                                   F6 - shadows on/off");
		hud.AddLine(HELP, "                                                   F7 - render mode");
		hud.AddLine(HELP, "                                                   F8 - reset view");
		hud.AddLine(HELP, "");
		hud.AddLine(HELP, "                                                   Try to hit the red square!");
		
		

		//add a pause screen
		hud.AddLine(PAUSE, "");
		hud.AddLine(PAUSE, "");
		hud.AddLine(PAUSE, "");
		hud.AddLine(PAUSE, "   Simulation paused. Press F10 to continue.");
		
		
		for (int i = 0; i < 20; i++)
		{
			hud.AddLine(WIN, "");
		}
		hud.AddLine(WIN, "                                                                   YOU WIN!");
		hud.AddLine(WIN, "                                                                F4 - reset scene");



		//set font size for all screens
		hud.FontSize(0.018f);
		//set font color for all screens
		hud.Color(PxVec3(0.f, 0.f, 0.f));
	}

	//Start the main loop
	void Start()
	{
		glutMainLoop();
	}

	//Render the scene and perform a single simulation step
	void RenderScene()
	{
		//handle pressed keys
		KeyHold();

		//start rendering
		Renderer::Start(camera->getEye(), camera->getDir());

		if ((render_mode == DEBUG) || (render_mode == BOTH))
		{
			Renderer::Render(scene->Get()->getRenderBuffer());
		}

		if ((render_mode == NORMAL) || (render_mode == BOTH))
		{
			std::vector<PxActor*> actors = scene->GetAllActors();
			if (actors.size())
				Renderer::Render(&actors[0], (PxU32)actors.size());
		}



		//adjust the HUD state
		if (hud_show)
		{
			if (scene->Pause())
				hud.ActiveScreen(PAUSE);
			else
				hud.ActiveScreen(HELP);
		}
		else
			hud.ActiveScreen(EMPTY);
	    

		 if (scene->hasWon)
		{
			hud.ActiveScreen(WIN);
			
		}
		//render HUD
		hud.Render();
		hud.Clear(); 
		HUDInit();
		myForceString = "Force: " + std::to_string(scene->myForce); 


		//finish rendering
		Renderer::Finish();

		//perform a single simulation step
		scene->Update(delta_time);
	}

	//user defined keyboard handlers
	void UserKeyPress(int key)
	{
		switch (toupper(key))
		{
			//implement your own
		case 'R':
			//add force when 'R' is pressed 
			scene->push();
			break;
		default:
			break;
		}
	}

	void UserKeyRelease(int key)
	{
		switch (toupper(key))
		{
		
		}
	}

	void UserKeyHold(int key)
	{
	}

	//handle camera control keys
	void CameraInput(int key)
	{
		switch (toupper(key))
		{
		case 'W':
			camera->MoveForward(delta_time);
			break;
		case 'S':
			camera->MoveBackward(delta_time);
			break;
		case 'A':
			camera->MoveLeft(delta_time);
			break;
		case 'D':
			camera->MoveRight(delta_time);
			break;
		case 'Z':
			camera->MoveUp(delta_time);
			break;
		case 'Q':
			camera->MoveDown(delta_time);
			break;
		default:
			break;
		}
	}

	///handle special keys
	void KeySpecial(int key, int x, int y)
	{
		//simulation control
		switch (key)
		{
			
			//Add force
		case GLUT_KEY_UP:
			scene->myForce += 0.1f; 
			break;
			//Subtract force
		case GLUT_KEY_DOWN:
			scene->myForce -= 0.1f; 
			break; 

			//display control
		case GLUT_KEY_F5:
			//hud on/off
			hud_show = !hud_show;
			break;
		case GLUT_KEY_F6:
			//shadows on/off
			Renderer::ShowShadows(!Renderer::ShowShadows());
			break;
		case GLUT_KEY_F7:
			//toggle render mode
			ToggleRenderMode();
			break;
		case GLUT_KEY_F8:
			//reset camera view
			camera->Reset();
			break;

			//simulation control
		case GLUT_KEY_F9:
			//select next actor
			scene->SelectNextActor();
			break;
		case GLUT_KEY_F10:
			//toggle scene pause
			scene->Pause(!scene->Pause());
			break;
		case GLUT_KEY_F4:
			//resect scene
			scene->myForce = 0; 
			scene->hasWon = false;
			scene->swichBoxPosition();
			scene->Reset();
			
			
			break;
		default:
			break;
		}
	}

	//handle single key presses
	void KeyPress(unsigned char key, int x, int y)
	{
		//do it only once
		if (key_state[key] == true)
			return;

		key_state[key] = true;

		//exit
		if (key == 27)
			exit(0);

		UserKeyPress(key);
	}

	//handle key release
	void KeyRelease(unsigned char key, int x, int y)
	{
		key_state[key] = false;
		UserKeyRelease(key);
	}

	//handle holded keys
	void KeyHold()
	{
		for (int i = 0; i < MAX_KEYS; i++)
		{
			if (key_state[i]) // if key down
			{
				CameraInput(i);
				UserKeyHold(i);
			}
		}
	}

	///mouse handling
	int mMouseX = 0;
	int mMouseY = 0;

	void motionCallback(int x, int y)
	{
		int dx = mMouseX - x;
		int dy = mMouseY - y;

		camera->Motion(dx, dy, delta_time);

		mMouseX = x;
		mMouseY = y;
	}

	void mouseCallback(int button, int state, int x, int y)
	{
		mMouseX = x;
		mMouseY = y;
	}

	void ToggleRenderMode()
	{
		if (render_mode == NORMAL)
			render_mode = DEBUG;
		else if (render_mode == DEBUG)
			render_mode = BOTH;
		else if (render_mode == BOTH)
			render_mode = NORMAL;
	}

	///exit callback
	void exitCallback(void)
	{
		delete camera;
		delete scene;
		PhysicsEngine::PxRelease();
	}
}

