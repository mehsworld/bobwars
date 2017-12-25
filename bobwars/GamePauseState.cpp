#include "GamePauseState.hpp"

#include "AppEngine.hpp"
#include "AppState.hpp"

#include <ENGINE\Logger.hpp>
#include <ENGINE\Graphics\Text.hpp>
#include "GamePlayState.hpp"

GamePauseState GamePauseState::GameCreationState_dontfuckwithme;

void GamePauseState::Init(AppEngine* app_)
{
	logger::INFO("GamePauseState Init");

	app = app_;

	sf::Vector2u windowSize = app->window->getSize();
	backgroundTexture.create(windowSize.x, windowSize.y);
	backgroundTexture.update(*app->window); // give texture window

	background.setSize(sf::Vector2f(windowSize.x, windowSize.y));
	background.setTexture(&backgroundTexture);
	background.setFillColor(sf::Color(255, 255, 255, 155));

	font = new sf::Font;
	font->loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
	text.setFont(*font);

	std::vector<SFUI::TextButton*> buttons;

	SFUI::TextButton* continueButton = new SFUI::TextButton("Continue");
	continueButton->setSizeMultiplier(2);
	SFUI::TextButton* saveButton = new SFUI::TextButton("Save Game");
	saveButton->setSizeMultiplier(2);
	saveButton->disable();
	SFUI::TextButton* optionsButton = new SFUI::TextButton("Options");
	optionsButton->setSizeMultiplier(2);
	optionsButton->disable();
	SFUI::TextButton* exitButton = new SFUI::TextButton("Exit");
	exitButton->setSizeMultiplier(2);
	buttons.push_back(continueButton);
	buttons.push_back(saveButton);
	buttons.push_back(optionsButton);
	buttons.push_back(exitButton);

	pauseMenu = new Menu(app->window, "Pause", buttons);

	logger::INFO("GamePauseState ready.");
}

void GamePauseState::Cleanup()
{
	logger::INFO("GamePauseState Cleaningup");

	delete font;
	delete pauseMenu;

	logger::INFO("GamePauseState Cleanedup");
}

void GamePauseState::Pause()
{
	logger::INFO("GamePauseState Paused");
}

void GamePauseState::Resume()
{
	logger::INFO("GamePauseState Resumed");
}

void GamePauseState::HandleEvents()
{
	sf::Event event;
	while (app->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			app->Quit();
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::Escape)
			{
				app->PopState();
			}
		}
	}

	pauseMenu->HandleEvents(event);
}

void GamePauseState::Update()
{
}

void GamePauseState::Draw()
{
	app->window->clear();

	app->window->draw(background);

	engine::text::draw(*app->window, text, "game paused", app->window->getDefaultView().getCenter());

	pauseMenu->Draw();

	app->window->display();
}
