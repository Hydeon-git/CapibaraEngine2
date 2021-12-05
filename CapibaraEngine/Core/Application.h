#ifndef _APPLICATION_
#define _APPLICATION_

#include <vector>
#include <string>

#include "Globals.h"
#include "Timer.h"
#include "PerfTimer.h"

//Forward declarations

class Module;
class ModuleWindow;
class ModuleInput;
class ModuleScene;
class ModuleRenderer3D;
class ModuleCamera3D;
class ModuleEditor;
class ModuleViewportFrameBuffer;
class ModuleImport;
class ModuleFileSystem;
class ModuleTextures;

class Application
{
public:

	// Modules
	ModuleWindow* window { nullptr };
	ModuleInput* input { nullptr };
	ModuleScene* scene { nullptr };
	ModuleRenderer3D* renderer3D { nullptr };
	ModuleCamera3D* camera { nullptr };
	ModuleEditor* editor { nullptr };
	ModuleViewportFrameBuffer* viewportBuffer { nullptr };
	ModuleViewportFrameBuffer* viewportBufferGame { nullptr };
	ModuleImport* import { nullptr };
	ModuleFileSystem* fileSystem { nullptr };
	ModuleTextures* textures { nullptr };

	Application();
	~Application();

	bool Init();
	void LoadEngineConfig();
	update_status Update();
	bool CleanUp();


	void SaveEngineConfig();
	void Save();

	void DrawFPSDiagram();
	void DrawHardwareConsole();


	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();

	void RequestBrowser(const std::string& website);

	void OnGui();

	// FPS core
	float				fps;
	float				dt;
	int					cap;
	PerfTimer			ptimer;
	Timer				ms_timer;
	Timer				frame_time;
	uint64				frame_count = 0;
	Timer				startup_time;
	Timer				last_sec_frame_time;
	uint32				last_sec_frame_count = 0;
	uint32				prev_last_sec_frame_count = 0;
	

	std::vector<float> fps_log;
	std::vector<float> ms_log;

	//Engine configuration
	bool closeEngine;
	bool vsync;
	bool saveAction;

private: 
	std::vector<Module*> modules;
};

extern Application* App;

#endif //_APPLICATION_