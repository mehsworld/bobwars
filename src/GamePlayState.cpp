#include "GamePlayState.hpp"
#include "GamePauseState.hpp"
#include "GameEndState.hpp"
#include "MainMenuState.hpp"

#include "GameSimulation.hpp"
#include "Interface.hpp"
#include "EntityManager.hpp"
#include "Bob.hpp"
#include "CommentSection.hpp"
#include "GooglePlus.hpp"
#include "Boomer.hpp"

#include "Util/Util.hpp"
#include "Util/Logger.hpp"
#include "Util/Graphics/Line.hpp"
#include "Util/Graphics/Graphics.hpp"
#include "Util/Graphics/Text.hpp"

#include <algorithm>
#include <random>

// TODO: what the fuck is this
sf::CircleShape test;

void GamePlayState::Init(AppEngine* app_)
{
	app = app_;

	logger::DEBUG("Initialising GamePlayState");

	logger::INFO("Pre-game setup.");

	simulation = new GameSimulation;

	debugText.setFont(SFUI::Theme::getFont());
	debugText.setCharacterSize(14);

	// TODO: actually "create" the gameworld in GameCreationState, or maybe GameWorldLoadState

	logger::INFO("Creating simulation->..");

	simulation->entMan = new EntityManager;
	simulation->world.generate();

	logger::INFO("Preparing user interface elements...");

	test.setRadius(3);
	test.setOrigin(test.getLocalBounds().width / 2, test.getLocalBounds().height / 2);

	rectSelect.setOutlineThickness(1);
	rectSelect.setOutlineColor(sf::Color(51, 153, 255));
	rectSelect.setFillColor(sf::Color(51, 153, 255, 100));

	debugFrameCounter.setFont(SFUI::Theme::getFont());
	debugFrameCounter.setPosition(20, 40);
	debugFrameCounter.setCharacterSize(14);

	sf::Vector2f screendimensions;
	screendimensions.x = app->window->getSize().x / 2;
	screendimensions.y = app->window->getSize().y / 2;

	mainView2 = new Camera;

	sf::View tempViewBecuaseIDonTKnowTheCorrectFunctionCallsToAchieveWhatTheConstructorDoes(screendimensions, screendimensions);
	mainView2->view = tempViewBecuaseIDonTKnowTheCorrectFunctionCallsToAchieveWhatTheConstructorDoes;

	// TODO: clean up old viewanchor stuff
//	viewAnchor = new sf::View(screendimensions, sf::Vector2f(app->window->getSize().x, app->window->getSize().y));

	ui = new Interface(app->window, &mainView2->view);
	ui->unitCounter->setMax(simulation->entMan->maxEntsPerTeam);

	mainView2->setPosition(sf::Vector2f(0, 0));

	baseViewSpeed = 500;

	simulation->entMan->addEnt(new CommentSection(simulation->entMan->getNextID(), simulation->entMan));

	simulation->entMan->addEnt(new Bob(simulation->entMan->getNextID(), simulation->entMan))->setPosition(sf::Vector2f(0, 51));
	simulation->entMan->addEnt(new Bob(simulation->entMan->getNextID(), simulation->entMan))->setPosition(sf::Vector2f(0, -51));
	simulation->entMan->addEnt(new Bob(simulation->entMan->getNextID(), simulation->entMan))->setPosition(sf::Vector2f(51, 0));
	simulation->entMan->addEnt(new Bob(simulation->entMan->getNextID(), simulation->entMan))->setPosition(sf::Vector2f(-51, 0));

	ui->unitCounter->setCount(5);

	app->dRPC.clearPresence();
	app->dRPC.setState("in a game");
	app->dRPC.setLargeImage("standard-game", "standard game");
	app->dRPC.setSmallImage("side-bob", "team bob");
	app->dRPC.setStartTime(time(0));
	app->dRPC.updatePresence();

	logger::DEBUG("GamePlayState ready!");
}

void GamePlayState::Cleanup()
{
	logger::DEBUG("GamePlayState cleaning up");

	app->network.sendCommand("Disconnecting");

	app->window->setView(app->window->getDefaultView());

	delete mainView2;
	delete simulation->entMan;
	delete ui;

	logger::DEBUG("GamePlayState cleaned up");
}

// Public

void GamePlayState::Pause()
{
	app->window->setView(app->window->getDefaultView());

	logger::DEBUG("GamePlayState paused");
}

void GamePlayState::Resume()
{
	logger::DEBUG("GamePlayState resumed");
}

void GamePlayState::HandleEvents()
{
	if (app->serverHost)
		app->server->Update();

	if (app->network.ready())
	{
		sf::Packet packet;

		if (app->network.receive(packet) == sf::Socket::Status::Disconnected)
		{
			logger::ERROR("Connection to server unexepectedly terminated.");
			app->TerminateNetworking();
			app->ChangeState(new MainMenuState);
			return;
		}

		std::string command;
		packet >> command;

		if (command == "Disconnected")
		{
			std::string reason;
			packet >> reason;
			logger::INFO("Disconnected from server: " + reason);

			app->TerminateNetworking();
			app->ChangeState(new MainMenuState);
			return;
		}
		else if (command == "GameUpdate")
		{
			logger::DEBUG("Updating Game");
		}
		else
		{
			logger::ERROR("Unknown command from server: " + command);
		}
	}

	sf::Time timePerFrame = sf::seconds(1.0f / 60.0f); // 60 frames per second

	//if (sf::milliseconds(app->delta) <= timePerFrame)
	{
		sf::Event event;
		while (app->window->pollEvent(event))
		{
			int actionID = ui->unitActionMenu->onEvent(event);

			if (actionID != -1)
				logger::DEBUG("actionID: " + std::to_string(actionID));

			switch (actionID)
			{
			case MENU_CALLBACKS::DELETE_SELECTION:
				simulation->deleteButton();
				break;
			case MENU_CALLBACKS::NULL_CALLBACK:
				break;
			default:
			{
				if (simulation->entMan->selectedEnts.size() != 1)
					break;

				if (sf::Keyboard::isKeyPressed(app->keys.multipleSelectionModifier))
					for (size_t i = 0; i < 5; i++)
						simulation->entMan->selectedEnts[0]->addTask(static_cast<EntityType>(actionID));
				else
					simulation->entMan->selectedEnts[0]->addTask(static_cast<EntityType>(actionID));

				app->network.sendCommand("AddUnitTask");

				break;
			}
			}

			if (event.type == sf::Event::EventType::Closed)
			{
				app->Quit();
				return;
			}
			else if (event.type == sf::Event::EventType::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Key::Escape)
				{
					app->PushState(new GamePauseState);
					return;
				}
				else if (event.key.code == sf::Keyboard::Key::Space)
				{
					if (!simulation->entMan->selectedEnts.empty())
					{
						logger::INFO("centering mainview on selected entity");

						mainView2->setCenter(simulation->entMan->selectedEnts[0]->sprite.getPosition());

						// TODO: do we want to do this?
						mainView2->view.setRotation(0);
					}
				}
				else if (event.key.code == sf::Keyboard::Key::LShift)
				{
					baseViewSpeed = 250;
				}
				else if (event.key.code == sf::Keyboard::Key::F12)
				{
					util::screenshot(*app->window);

					app->window->clear(sf::Color::White);
					app->window->display();
				}
				else if (event.key.code == sf::Keyboard::Key::Tilde)
				{
					app->settings.debug = !app->settings.debug;

					logger::INFO("cl_debug set to " + std::to_string(app->settings.debug));
				}
				else if (event.key.code == sf::Keyboard::Key::Delete || event.key.code == sf::Keyboard::Key::End)
				{
					simulation->deleteButton();
				}
				else if (event.key.code == sf::Keyboard::Key::LControl || event.key.code == sf::Keyboard::Key::A || event.key.code == sf::Keyboard::Key::Equal || sf::Keyboard::Key::Dash)
				{
					//TODO: this probably isn't the best way to handle key combinations
					//TODO: really find a better way to do this.

					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
					{
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) && !simulation->entMan->entities.empty())
						{
							simulation->entMan->deselectAllEnts();
							simulation->entMan->selectedEnts = simulation->entMan->entities;

							for (size_t i = 0; i < simulation->entMan->entities.size(); i++)
								simulation->entMan->entities[i]->isSelected = true;

							ui->deleteEnabled = true;

							ui->updateSelectionInfo(simulation->entMan->selectedEnts);

							logger::INFO("selected " + std::to_string(simulation->entMan->selectedEnts.size()) + " entities (of " + std::to_string(simulation->entMan->entities.size()) + ")");
						}
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Equal))
						{
							mainView2->zoom(0.5f);
							// zoom camera in
						}
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Dash))
						{
							mainView2->zoom(2.0f);
							// zoom camera out
						}
					}
				}
			}
			else if (event.type == sf::Event::EventType::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Key::LShift)
				{
					baseViewSpeed = 500;
				}
			}
			else if (event.type == sf::Event::EventType::MouseButtonPressed)
			{
				test.setPosition(sf::Vector2f(sf::Mouse::getPosition(*app->window).x, sf::Mouse::getPosition(*app->window).y));

				if (!util::logic::mouseIsOver(ui->topBar, *app->window) && !util::logic::mouseIsOver(ui->bottomBar, *app->window))
				{
					if (event.mouseButton.button == sf::Mouse::Button::Left)
					{
						// if we haven't broken the loop already, it means we either clicked an entity or clicked nothing
						bool selectedNothing(true);
						// TODO: do this in reverse so that we select entities on top first
						//for (size_t i = 0; i < simulation->entMan->entities.size(); i++)
						for (int i = simulation->entMan->entities.size() - 1; i >= 0; i--)
						{
							if (util::logic::mouseIsOver(simulation->entMan->entities[i]->sprite, *app->window, mainView2->view))
							{
								if (simulation->entMan->entities[i]->isSelected) // entity is already selected
								{
									logger::DEBUG("clicked entity is already selected.");

									// this entity is already selected
									// and we're in group select mode
									// so remove the entity from the selection
									if (sf::Keyboard::isKeyPressed(app->keys.multipleSelectionModifier))
									{
										// in Age of Empires II, if you hold control and click
										// on an already selected entity it is deselected
										simulation->entMan->deselectEnt(simulation->entMan->entities[i]);
										selectedNothing = false;

										logger::INFO("removed entity" + std::to_string(simulation->entMan->entities[i]->entityID) + " from selection");
									}
									else // we're going to single out a unit
									{
										// unit is already selected.
										// in AoE2, if you have multiple units selected
										// and then select one of them, it will deselect
										// all entities except that one.

										if (simulation->entMan->selectedEnts.size() == 1)
										{
											logger::INFO("entity " + std::to_string(simulation->entMan->entities[i]->entityID) + " already selected");
										}
										else // singling out one unit
										{
											simulation->entMan->deselectAllEnts();
											simulation->entMan->selectEnt(simulation->entMan->entities[i]);

											logger::INFO("selected entity" + std::to_string(simulation->entMan->entities[i]->entityID) + " (" + simulation->entMan->entities[i]->type + ")");
										}

										selectedNothing = false;
									}
								}
								else // selecting the first unit, or adding an additional unit to the selection
								{
									// if we're not selecting an additional unit, deselect all other entities first
									if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
										simulation->entMan->deselectAllEnts();

									simulation->entMan->selectEnt(simulation->entMan->entities[i]);

									logger::INFO("selected entity" + std::to_string(simulation->entMan->entities[i]->entityID));

									ui->deleteEnabled = true;

									selectedNothing = false;
								}

								break;
							}
						} // what entity did we click

						if (selectedNothing) // selected nothing and didn't already have nothing
						{
							if (!simulation->entMan->entities.empty())
							{
								simulation->entMan->deselectAllEnts();

								ui->deleteEnabled = false;

								logger::INFO("All entities deselected");
								ui->updateSelectionInfo(simulation->entMan->selectedEnts);
							}

							rectSelecting = true;
							rectSelect.setSize(sf::Vector2f(0, 0));
							sf::Vector2f pos(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), mainView2->view));
							rectSelect.setPosition(sf::Vector2f(pos.x, pos.y));

							break;
						}

						ui->updateSelectionInfo(simulation->entMan->selectedEnts);
					}
					else if (event.mouseButton.button == sf::Mouse::Button::Right)
					{
						if (!simulation->entMan->selectedEnts.empty())
						{
							sf::Vector2f movePos(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), mainView2->view));

							// TODO: decide if all points should be set at one. ex:
							// if a building and a bob are selected, and the user right clicks
							// should it move the building's garrison point, and tell the unit to move to that spot?
							// or just move the unit, and ignore the building
							if (!simulation->world.getGlobalBounds().contains(movePos))
								logger::INFO("Cannot set garrison point out of bounds.");
							else
								for (size_t i = 0; i < simulation->entMan->selectedEnts.size(); i++)
									if (simulation->entMan->selectedEnts[i]->isBuilding)
									{
										Building* ent = static_cast<Building*>(simulation->entMan->selectedEnts[i]);
										ent->setGarrisonPoint(movePos);
									}
									else if (simulation->entMan->selectedEnts[i]->isComponentEntity)
									{
										ComponentEntity* entity = static_cast<ComponentEntity*>(simulation->entMan->selectedEnts[i]);
										GroundMoveComponent* move = entity->getComponent<GroundMoveComponent*>();
										
										if (move != nullptr)
											move->setMoveDestination(movePos);
									}
						}
					}
				}
			}
			else if (event.type == sf::Event::EventType::MouseMoved)
			{
				sf::Vector2f pos = rectSelect.getPosition();
				sf::Vector2f mouse(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), mainView2->view));

				sf::Vector2f newSize;

				newSize.x = mouse.x - pos.x;
				newSize.y = mouse.y - pos.y;

				rectSelect.setSize(newSize);
			}
			else if (event.type == sf::Event::EventType::MouseButtonReleased)
			{
				if (rectSelecting)
				{
					rectSelecting = false;

					sf::FloatRect bounds = rectSelect.getGlobalBounds();

					for (size_t i = 0; i < simulation->entMan->entities.size(); i++)
					{
						// TODO: make sure this works for all entities.
						// not all entities may have a sprite, and some may have more than one
						// BaseEntity should implement getGlobalBounds();
						if (bounds.intersects(simulation->entMan->entities[i]->sprite.getGlobalBounds()))
							simulation->entMan->selectEnt(simulation->entMan->entities[i]);
					}
				}
			}
			else if (event.type == sf::Event::EventType::MouseWheelMoved)
			{
				// TODO: make this a variable of the Camera class
				static float zoomlevel = 2.0f;

				if (event.mouseWheel.delta < 0) // up (in)
				{
					if (zoomlevel <= 4)
					{
						mainView2->zoom(2.0f);
						zoomlevel *= 2.0f;
					}
				}
				else if (event.mouseWheel.delta > 0) // down (out)
				{
					if (zoomlevel > 0.5f)
					{
						mainView2->zoom(0.5f);
						zoomlevel *= 0.5f;
					}
				}
			}

			ui->HandleEvents(event);
		} // pollevent

		if (simulation->googleTimer.getElapsedTime().asSeconds() > 5)
		{
			std::random_device dev;
			std::mt19937 rng(dev());

			int halfX = simulation->world.getSize().x / 2;
			int halfY = simulation->world.getSize().y / 2;

			std::uniform_int_distribution<std::mt19937::result_type> xdist(-halfX, halfX);
			std::uniform_int_distribution<std::mt19937::result_type> ydist(-halfY, halfY);

			int x = xdist(rng);
			int y = ydist(rng);

			simulation->entMan->create<GooglePlus>()->getComponent<GroundMoveComponent*>()->setMoveDestination(sf::Vector2f(x, y));

			ui->unitCounter->add(1);
			simulation->googleTimer.restart();
		}

		if (simulation->resourceTimer.getElapsedTime().asSeconds() > 10)
		{
			ui->memesCounter->add(50);
			simulation->resourceTimer.restart();
		}

		for (size_t i = 0; i < simulation->entMan->entities.size(); i++)
		{
			simulation->entMan->entities[i]->Frame(app->delta);

			if (simulation->entMan->entities[i]->health <= 0)
				simulation->entMan->deleteEnt(simulation->entMan->entities[i]);
		}

		if (simulation->entMan->entities.size() <= 0)
		{
			app->ChangeState(new GameEndState);
			return;
		}

		if (app->window->hasFocus())
		{
			updateGameCamera();

			// todo: clamp the mouse and keep it in the window`

			float frames_per_second = framesClock.restart().asSeconds();
			debugFrameCounter.setString("FPS: " + std::to_string(static_cast<int>(1.0f / app->delta)));
		} // app->window.hasFocus()
	}
}

void GamePlayState::Update()
{
}

void GamePlayState::Draw()
{
	app->window->clear();
	
	// ------------ MAIN VIEW
	app->window->setView(mainView2->view);

	app->window->draw(simulation->world);

	for (BaseEntity* entity : simulation->entMan->entities)
		app->window->draw(*entity);

	if (rectSelecting)
		app->window->draw(rectSelect);

	if (app->settings.debug)
	{
		sf::CircleShape originShape;
		originShape.setRadius(2);
		originShape.setOrigin(sf::Vector2f(1, 1));
		originShape.setFillColor(sf::Color::Red);

		for (BaseEntity* entity : simulation->entMan->entities)
		{
			if (!entity->isSelected)
				util::graphics::outline(*app->window, entity->sprite, 2, sf::Color::Red);
			else
				util::graphics::outline(*app->window, entity->sprite, 2, sf::Color::Yellow);

			showObjectCoords(entity->sprite);
			util::text::draw(*app->window, debugText, std::to_string(entity->entityID), sf::Vector2f(entity->sprite.getPosition().x, entity->sprite.getPosition().y - entity->sprite.getLocalBounds().height / 2), sf::Vector2f(.2f, .2f));
			util::text::draw(*app->window, debugText, entity->type, sf::Vector2f(entity->sprite.getPosition().x, entity->sprite.getPosition().y), sf::Vector2f(.2f, .2f));

			originShape.setPosition(entity->getPosition());

			app->window->draw(originShape);

			Line line;
			line.setThickness(1.0f);

			if (entity->isComponentEntity)
			{
				ComponentEntity* ent = static_cast<ComponentEntity*>(entity);

				if (entity->isSelected)
					line.setColor(sf::Color::Yellow);
				else
					line.setColor(sf::Color::Red);

				if (ent->hasComponent("GroundMove"))
				{
					GroundMoveComponent* move = static_cast<GroundMoveComponent*>(ent->getComponent("GroundMove"));
					line.setPoints(entity->sprite.getPosition(), move->getMoveDesintation());
				}
			}
			else if (entity->isBuilding)
			{
				Building* building = static_cast<Building*>(entity);

				if (building->hasGarrisonPoint)
					line.setPoints(entity->sprite.getPosition(), building->getGarrisonPoint());
			}

			app->window->draw(line.vertices, 4, sf::Quads);
		}
	}
	else
		if (!simulation->entMan->selectedEnts.empty())
			for	(BaseEntity* entity : simulation->entMan->selectedEnts)
				util::graphics::outline(*app->window, entity->sprite, 2, sf::Color::Yellow);

	// ------------- ANCHOR
	app->window->setView(*ui->getViewAnchor());

	ui->Draw();

	// debug info like coordinates and stuff
	if (app->settings.debug)
	{
		float width = app->settings.mouseMoveBorderWidth;
		sf::RectangleShape outline;
		outline.setSize(sf::Vector2f(app->window->getSize().x - (width * 2), app->window->getSize().y - (width * 2)));
		outline.setOutlineColor(sf::Color(255, 200, 0, 50));
		outline.setOutlineThickness(width);
		outline.setFillColor(sf::Color::Transparent);
		outline.setPosition(sf::Vector2f(width, width));

		app->window->draw(outline);

		// TODO: we might use a different method to render text.
		// perhaps create a string manager type class, and use only one piece of text to render everything with line breaks
		// this could yeild a generous performance improvement.

		app->window->draw(debugFrameCounter);

		// TODO: add camera rotation to debug text
		std::string viewCoordinates = "Camera: X: " + std::to_string(static_cast<int>(mainView2->getCenter().x)) + " Y: " + std::to_string(static_cast<int>(mainView2->getCenter().y));
		util::text::draw(*app->window, debugText, viewCoordinates, sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 12));
		util::text::draw(*app->window, debugText, "selectedEntities: " + std::to_string(simulation->entMan->selectedEnts.size()), sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 24));
		util::text::draw(*app->window, debugText, "totalEntities: " + std::to_string(simulation->entMan->entities.size()), sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 36));
		util::text::draw(*app->window, debugText, "maxEntities: " + std::to_string(simulation->entMan->maxEnts), sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 48));
		util::text::draw(*app->window, debugText, "physicalMaxEntities: " + std::to_string(simulation->entMan->physicalMaxEnts), sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 60));
		util::text::draw(*app->window, debugText, "maxEntitiesPerTeam: " + std::to_string(simulation->entMan->maxEntsPerTeam), sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 72));
		util::text::draw(*app->window, debugText, "delta: " + std::to_string(app->delta), sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 84));

		if (!simulation->entMan->selectedEnts.empty() && simulation->entMan->selectedEnts.size() == 1)
		{
			util::text::draw(*app->window, debugText, "entityID: " + std::to_string(simulation->entMan->selectedEnts[0]->entityID), sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 108));
//			util::text::draw(*app->window, debugText, "team: " + std::to_string(simulation->entMan->selectedEnts[0]->team), sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 120));
			util::text::draw(*app->window, debugText, "type: " + simulation->entMan->selectedEnts[0]->type, sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 132));
			util::text::draw(*app->window, debugText, "health: " + std::to_string(simulation->entMan->selectedEnts[0]->health), sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 144));
			util::text::draw(*app->window, debugText, "hitpoints: " + std::to_string(simulation->entMan->selectedEnts[0]->hitpoints), sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 156));
			util::text::draw(*app->window, debugText, "armor: " + std::to_string(simulation->entMan->selectedEnts[0]->armor), sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 168));

/*
			if (simulation->entMan->selectedEnts[0]->isComponentEntity)
			{
				static_cast<ComponentEntity*>(simulation->entMan->selectedEnts[0])->componentList();
			}
*/
		}

		std::string entries;

		for	(int i = logger::latestEntries.size() - 1; i >= 0; --i)
			entries += logger::latestEntries[i] + "\n";

		util::text::draw(*app->window, debugText, entries, sf::Vector2f(debugFrameCounter.getPosition().x, debugFrameCounter.getPosition().y + 216));

		app->window->draw(test);
	}

	app->window->display();
}

void GamePlayState::updateGameCamera()
{
	int moveX = 0, moveY = 0;
//	int moveAmount = baseViewSpeed * app->delta;
	int moveAmount = baseViewSpeed * .01;

	// keyboard based camera movement
	if (sf::Keyboard::isKeyPressed(app->keys.moveCameraUp))
		moveY -= moveAmount;
	if (sf::Keyboard::isKeyPressed(app->keys.moveCameraDown))
		moveY += moveAmount;
	if (sf::Keyboard::isKeyPressed(app->keys.moveCameraLeft))
		moveX -= moveAmount;
	if (sf::Keyboard::isKeyPressed(app->keys.moveCameraRight))
		moveX += moveAmount;

	if (app->settings.mouseMoveEnabled)
	{
		// mouse based camera movement
		float width = app->settings.mouseMoveBorderWidth;
		sf::FloatRect left(0, 0, width, app->window->getSize().y);
		sf::FloatRect right(app->window->getSize().x - width, 0, width, app->window->getSize().y);
		sf::FloatRect up(0, 0, app->window->getSize().x, width);
		sf::FloatRect down(0, app->window->getSize().y - width, app->window->getSize().x, width);

		sf::Vector2f mousePos = app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *ui->getViewAnchor());

		if (up.contains(mousePos))
			moveY -= moveAmount;
		if (down.contains(mousePos))
			moveY += moveAmount;
		if (left.contains(mousePos))
			moveX -= moveAmount;
		if (right.contains(mousePos))
			moveX += moveAmount;
	}

	sf::Vector2f newPosition = mainView2->getCenter() + sf::Vector2f(moveX, moveY);

	newPosition.x = std::clamp(newPosition.x, -400.f, 400.f);
	newPosition.y = std::clamp(newPosition.y, -300.f, 300.f);

	mainView2->setPosition(newPosition);
}

void GamePlayState::showObjectCoords(sf::Sprite &object)
{
	// TODO: this can be put into draw call, yes?

	std::string coords =
		"X: " +
		std::to_string(static_cast<int>(object.getPosition().x)) +
		" Y: " +
		std::to_string(static_cast<int>(object.getPosition().y));

	float x = object.getPosition().x - object.getLocalBounds().width;
	float y = object.getPosition().y - object.getLocalBounds().height / 1.5;
	sf::Vector2f position(x, y);

	util::text::draw(*app->window, debugText, coords, position, sf::Vector2f(.2f, .2f));
}

void GamePlayState::saveGame()
{
	logger::INFO("Saving game.");
	simulation->saveToFile("");
}
