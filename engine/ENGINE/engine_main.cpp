#include <string>
#include <time.h>
#include <iostream>
#include <fstream>

#include <SFML\Graphics.hpp>
#include <ENGINE\engine_main.hpp>

namespace engine
{
	std::string const version("1.0.7");
	CVar cl_debug(1);

	namespace text
	{
		void draw(sf::RenderWindow &window, sf::Text &text, const std::string string, const sf::Vector2f &pos, const int characterSize, const sf::Color &c)
		{
			text.setPosition(pos);
			text.setCharacterSize(characterSize);
			text.setFillColor(c);
			text.setString(string);

			window.draw(text);

			text.setFillColor(sf::Color::White);
		}

		void draw(sf::RenderWindow &window, sf::Text &text, const std::string string, const sf::Vector2f &pos, const int characterSize)
		{
			text.setPosition(pos);
			text.setCharacterSize(characterSize);
			text.setString(string);

			window.draw(text);
		}

		void draw(sf::RenderWindow &window, sf::Text &text, const std::string string, const sf::Vector2f &pos)
		{
			text.setPosition(pos);
			text.setCharacterSize(34);
			text.setString(string);

			window.draw(text);
		}
	}

	namespace graphics
	{
		void outline(sf::RenderWindow &window, sf::Shape &object, float thickness, sf::Color color)
		{
			sf::RectangleShape line;

			line.setPosition(object.getPosition());
			line.setSize(sf::Vector2f(object.getLocalBounds().width, object.getLocalBounds().height));
			line.setFillColor(sf::Color(0, 0, 0, 0));

			line.setOutlineColor(color);
			line.setOutlineThickness(thickness);

			window.draw(line);
		}

		void outline(sf::RenderWindow &window, sf::Sprite &object, float thickness, sf::Color color)
		{
			sf::RectangleShape line;

			line.setPosition(object.getPosition());
			line.setSize(sf::Vector2f(object.getLocalBounds().width, object.getLocalBounds().height));
			line.setFillColor(sf::Color(0, 0, 0, 0));

			line.setOutlineColor(color);
			line.setOutlineThickness(thickness);

			window.draw(line);
		}
	}

	std::string getTime()
	{
		time_t $time = time(0);

		int seconds = $time % 60;
		$time /= 60;

		int minutes = $time % 60;
		$time /= 60;

		int hours = $time % 24; // Oklahoma timezone.
		$time /= 24;

		std::string seconds_s;
		std::string minutes_s;
		std::string hours_s(std::to_string(hours));

		if (seconds < 10)
		{
			for (int i = 0; i < 10; i++)
			{
				seconds_s = "0" + std::to_string(seconds);
			}
		}
		else
		{
			seconds_s = std::to_string(seconds);
		}

		if (minutes < 10)
		{
			for (int i = 0; i < 10; i++)
			{
				minutes_s = "0" + std::to_string(minutes);
			}
		}
		else
		{
			minutes_s = std::to_string(minutes);
		}

		if (hours < 10)
		{
			for (int i = 0; i < 10; i++)
			{
				hours_s = "0" + std::to_string(hours);
			}
		}
		else
		{
			hours_s = std::to_string(hours);
		}

		std::string time = hours_s + ":" + minutes_s + ":" + seconds_s;

		return time;
	}
}
