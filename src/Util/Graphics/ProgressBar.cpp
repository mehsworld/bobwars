#include "ProgressBar.hpp"

#include <iostream>

ProgressBar::ProgressBar()
{
	for (int i = 0; i < 4; ++i)
		bar[i].color = barColor;
}

ProgressBar::ProgressBar(sf::Vector2f position, float width, float height)
{
	setPosition(position);

	for (int i = 0; i < 4; ++i)
		bar[i].color = barColor;
}

void ProgressBar::setWidth(float width)
{
	this->width = width;

	setPosition(bar[0].position);
}

float ProgressBar::getWidth() const
{
	return width;
}

void ProgressBar::setHeight(float height)
{
	this->height = height;

	setPosition(bar[0].position);
}

float ProgressBar::getHeight() const
{
	return height;
}

void ProgressBar::setPosition(const sf::Vector2f& position)
{
	box.setPosition(position);

	bar[0].position = sf::Vector2f(position.x + 0, position.y + 0);
	bar[1].position = sf::Vector2f(position.x + width, position.y + 0);
	bar[3].position = sf::Vector2f(position.x + 0, position.y + height);
	bar[2].position = sf::Vector2f(position.x + width, position.y + height);
}

sf::Vector2f ProgressBar::getPosition() const
{
	return bar[0].position;
}

void ProgressBar::addThingToDo()
{
	thingsToDo += 1;
	setValue(thingsDone, thingsToDo);
}

void ProgressBar::addThingsToDo(signed int things)
{
	thingsToDo += things;
	setValue(thingsDone, thingsToDo);
}

void ProgressBar::oneThingDone()
{
	setValue(thingsDone + 1, thingsToDo);
}

void ProgressBar::setColor(const sf::Color& color)
{
	for (int i = 0; i < 4; ++i)
		bar[i].color = color;
}

void ProgressBar::setColor(const sf::Color& topLeft, const sf::Color& topRight, const sf::Color& bottomLeft, const sf::Color& bottomRight)
{
	bar[0].color = topLeft;
	bar[1].color = topRight;
	bar[3].color = bottomLeft;
	bar[2].color = bottomRight;
}

void ProgressBar::setValue(int thingsDone_)
{
	thingsDone = thingsDone_;
	percentDone = calculatePercent(thingsDone, thingsToDo);

	float x = (percentDone / 100) * width;

	bar[1].position.x = bar[2].position.x = x;
}

void ProgressBar::setValue(int thingsDone_, int thingsToDo_)
{
	thingsDone = thingsDone_;
	thingsToDo = thingsToDo_;
	percentDone = calculatePercent(thingsDone, thingsToDo);

	float x = (percentDone / 100) * width;

	bar[1].position.x = bar[2].position.x = box.getPosition().x + x;

//	std::cout << percentDone << "/" << 100 << "*" << width << "=" << (percentDone / 100) * width << "(" << x << ")" << std::endl;
}

float ProgressBar::getValue() const
{
	return percentDone;
}

void ProgressBar::reset()
{
	setValue(0, 0);
}

void ProgressBar::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(box, states);
	target.draw(bar, 4, sf::Quads, states);
}

float ProgressBar::calculatePercent(float x, float y)
{
	return x / y * 100;
}
