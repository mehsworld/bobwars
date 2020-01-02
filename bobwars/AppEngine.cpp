#include "AppEngine.hpp"
#include "AppState.hpp"

#include "Util/Logger.hpp"

#include <SFUI/Theme.hpp>

#include <iostream>
#include <filesystem>

namespace fs = std::experimental::filesystem;

// INFO: keep this bare minimums
// AppEngine is the core of the application,
// and should rely on as little external
// resources as reasonably possible.

void AppEngine::Init(AppSettings settings_)
{
	std::cout << "AppEngine Init" << std::endl;

	settings = settings_;

	for (int i = 0; i < settings.launchOptions.argc; i++)
	{
		if (std::string(settings.launchOptions.argv[i]) == "-fullscreen")
			settings.fullscreen = true;

		if (std::string(settings.launchOptions.argv[i]) == "-vsync")
			settings.vsync = true;

		if (std::string(settings.launchOptions.argv[i]) == "-maxfps")
			// next arg should be an int (the maxfps)
			settings.maxfps = std::stoi(settings.launchOptions.argv[i += 1]);
	}

	if (!fs::exists("./bobwars"))
		fs::create_directory("./bobwars");

	logger::INFO("Preparing SFUI");

	SFUI::Theme::loadFont("bobwars/resource/interface/tahoma.ttf");
	SFUI::Theme::loadTexture("bobwars/resource/interface/texture_square.png");
	SFUI::Theme::textCharacterSize = 11;
	SFUI::Theme::click.textColor = SFUI::Theme::hexToRgb("#191B18");
	SFUI::Theme::click.textColorHover = SFUI::Theme::hexToRgb("#191B18");
	SFUI::Theme::click.textColorFocus = SFUI::Theme::hexToRgb("#000000");
	SFUI::Theme::input.textColor = SFUI::Theme::hexToRgb("#000000");
	SFUI::Theme::input.textColorHover = SFUI::Theme::hexToRgb("#CC7A00");
	SFUI::Theme::input.textColorFocus = SFUI::Theme::hexToRgb("#000000");
	SFUI::Theme::windowBgColor = SFUI::Theme::hexToRgb("#dddbde");
	SFUI::Theme::PADDING = 2.f;


	window = new sf::RenderWindow(sf::VideoMode(settings.width, settings.height), settings.title);
	window->setVerticalSyncEnabled(true);

	resMan = new ResourceManager;

	dRPC.Init();

	running = true;

	std::cout << "AppEngine ready" << std::endl;
}

void AppEngine::Cleanup()
{
	std::cout << "Cleaning up AppEngine." << std::endl;
	
	// cleanup the all states
	while (!states.empty())
	{
		states.back()->Cleanup();

		delete states.back();
		states.pop_back();
	}

	window->close();
	delete window;

	resMan->freeAllTextures();
	delete resMan;

	dRPC.Shutdown();

	std::cout << "AppEngine cleaned up." << std::endl;
}

void AppEngine::ChangeState(AppState* state)
{
	if (!states.empty()) 
	{
		states.back()->Cleanup();

		delete states.back();
		states.pop_back();
	}

	std::cout << "changing state" << std::endl;

	states.push_back(state);
	states.back()->Init(this);
}

void AppEngine::PushState(AppState* state)
{
	// pause current state
	if (!states.empty())
		states.back()->Pause();

	std::cout << "pushing state" << std::endl;

	// store and init the new state
	states.push_back(state);
	states.back()->Init(this);
}

void AppEngine::PopState()
{
	// cleanup the current state
	if (!states.empty())
	{
		states.back()->Cleanup();

		delete states.back();
		states.pop_back();
	}

	// resume previous state
	if (!states.empty())
		states.back()->Resume();
}

void AppEngine::PopState(int amount)
{
	for (size_t i = 0; i < amount; i++)
	{
		// cleanup the current state
		if (!states.empty())
		{
			states.back()->Cleanup();

			delete states.back();
			states.pop_back();
		}

		// resume previous state
		if (!states.empty())
			states.back()->Resume();
	}
}

void AppEngine::HandleEvents()
{
	if (running)
		states.back()->HandleEvents();
}

void AppEngine::Update()
{
	// let the state update the game
	if (running)
	{
		dRPC.update();
		states.back()->Update();
	}
}

void AppEngine::Draw()
{
	// let the state draw the screen
	if (running)
		states.back()->Draw();
}

void AppEngine::Quit()
{
	for (size_t i = 0; i < states.size(); i++)
	{
		states.back()->Cleanup();
		delete states.back();
		states.pop_back();
	}

	states.clear();

	running = false;
}
