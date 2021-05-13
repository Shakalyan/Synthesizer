#pragma once

#include <SFML/Graphics.hpp>
#include "soundLoader.h"
#include "soundMaker.h"


namespace soundDrawing
{

	const sf::Vector2f WHITE_KEY_SIZE(120, 700);
	const sf::Vector2f BLACK_KEY_SIZE(60, 380);

	extern sf::RectangleShape WHITE_KEY;
	extern sf::RectangleShape BLACK_KEY;

	const sf::Color WHITE_PRESSED(0, 160, 230), WHITE_DEFAULT(255, 255, 255);
	const sf::Color BLACK_PRESSED(60, 70, 200), BLACK_DEFAULT(0, 0, 0);

	const sf::Vector2f OPTION_PANEL_SIZE(400, WHITE_KEY_SIZE.y);
	const sf::Vector2f WINDOW_SIZE = sf::Vector2f(WHITE_KEY_SIZE.x * 10, WHITE_KEY_SIZE.y);

	const sf::Vector2f FW_DEFAULT_POS(0, WINDOW_SIZE.y / 2);
	const int FW_COEFF = 50;

	const int OPTIONS_COUNT = 2;
	const sf::Color OPTION_PANEL_COLOR = sf::Color::White;

	extern sf::Texture controls_texture;
	extern sf::Sprite controls_sprite;

	extern sf::RectangleShape OP_MAIN_RECT;
	extern sf::CircleShape OP_MARKER;
	extern sf::Text OP_TEXT;

	extern bool piano_update;
	extern bool OP_update;
	extern bool wave_update;
	extern bool window_is_open;

	extern int current_option;

	extern sf::Font font;

	enum class Windows
	{
		Start,
		Piano,
		Waves,
		Options
	};



	struct key_t
	{
		int id;
		bool type;
		bool is_active;
		sf::RectangleShape rectangle_shape;
		key_t();
	};

	extern key_t keys_arr[soundMaker::NUMBER_OF_KEYS];


	sf::Vector2f GetKeyPosition(key_t key);

	void CheckKeys();


	extern Windows window_type;



	void ClearWindow(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2f size, sf::Color color);


	void StartPreparations();


	struct option_t
	{
		int id;
		std::string str_name;
		std::string str_units;
		double* state;
		double min_state;
		double max_state;
		option_t(int id, std::string str_name, std::string str_units, double* state, double min_state, double max_state);
	};

	extern option_t options[OPTIONS_COUNT];

	void DrawOptionPanel(sf::RenderWindow& window);

	void DrawPiano(sf::RenderWindow& window);

	void DrawWaves(sf::RenderWindow& window, double time);

	void DrawWindows(soundLoader_t<short>& sl);

}
