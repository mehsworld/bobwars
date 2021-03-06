#include "GooglePlus.hpp"

#include "GroundMoveComponent.hpp"
#include "Util/Logger.hpp"

GooglePlus::GooglePlus(const int entityID) : ComponentEntity(entityID)
{
	logger::INFO("GooglePlus entity created.");

	texture.loadFromFile("bobwars/resource/textures/googleplus.png"); // no point in logging failure, as SFML does this for us (I wish it didn't!)
	sprite.setTexture(texture, true);
	sprite.setOrigin(sf::Vector2f(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2));

	team = Team::BadGuys;
	maxHealth = 100;
	health = maxHealth;
	hitpoints = 1;
	type = "GooglePlus";

	addComponent(new GroundMoveComponent);
}

GooglePlus::~GooglePlus()
{
}
