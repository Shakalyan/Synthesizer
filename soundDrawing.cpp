#include "soundDrawing.h"
#include "soundMaker.h"
#include <Windows.h>




namespace soundDrawing
{	

	sf::RectangleShape WHITE_KEY;
	sf::RectangleShape BLACK_KEY;
	sf::RectangleShape OP_MAIN_RECT;
	sf::CircleShape OP_MARKER;
	sf::Text OP_TEXT;
	bool piano_update = false;
	bool OP_update = false;
	bool wave_update = true;
	bool window_is_open = true;
	int current_option = 0;
	sf::Font font;
	sf::Texture controls_texture;
	sf::Sprite controls_sprite;

	key_t::key_t()
	{
		id = 0;
		type = 0;
		is_active = false;
	}

	key_t keys_arr[soundMaker::NUMBER_OF_KEYS];


	sf::Vector2f GetKeyPosition(key_t key)
	{
		int x = 0, y = 0;
		if (!key.type)
		{
			if (key.id < 5)
				x = key.id / 2 * WHITE_KEY.getSize().x;
			else if (key.id < 12)
				x = (3 + (key.id - 5) / 2) * WHITE_KEY.getSize().x;
			else
				x = (7 + (key.id - 12) / 2) * WHITE_KEY.getSize().x;
		}
		else
			x = GetKeyPosition(keys_arr[key.id - 1]).x + WHITE_KEY.getSize().x - (BLACK_KEY.getSize().x / 2);


		return sf::Vector2f(x, y);
	}

	void CheckKeys()
	{
		for (auto& key : keys_arr)
			if (key.is_active)
				key.rectangle_shape.setFillColor((key.type) ? BLACK_PRESSED : WHITE_PRESSED);
			else
				key.rectangle_shape.setFillColor((key.type) ? BLACK_DEFAULT : WHITE_DEFAULT);
	}

	

	Windows window_type = Windows::Start;


	void ClearWindow(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2f size, sf::Color color)
	{
		sf::RectangleShape rect;
		rect.setSize(size);
		rect.setFillColor(color);
		rect.setPosition(position);
		for (int i = 0; i < 3; i++)
		{
			window.draw(rect);
			window.display();
			Sleep(1);
		}
	}


	void StartPreparations()
	{
		soundMaker::PrepareInstruments();
		
		WHITE_KEY.setSize(WHITE_KEY_SIZE);
		WHITE_KEY.setFillColor(WHITE_DEFAULT);
		WHITE_KEY.setOutlineColor(sf::Color(0, 0, 0));
		WHITE_KEY.setOutlineThickness(3);

		BLACK_KEY.setSize(BLACK_KEY_SIZE);
		BLACK_KEY.setFillColor(BLACK_DEFAULT);
		BLACK_KEY.setOutlineColor(BLACK_DEFAULT);

		
		for (int i = 0; i < soundMaker::NUMBER_OF_KEYS; i++)
		{
			if ((i < 5) || (i > 11))
				keys_arr[i].type = (bool)(i % 2);
			else if (i < 12)
				keys_arr[i].type = (bool)((i + 1) % 2);

			keys_arr[i].id = i;
			keys_arr[i].rectangle_shape = (keys_arr[i].type) ? BLACK_KEY : WHITE_KEY;
			keys_arr[i].rectangle_shape.setPosition(GetKeyPosition(keys_arr[i]));
		}

		font.loadFromFile("font.ttf");

		OP_MAIN_RECT.setSize(WINDOW_SIZE);
		OP_MAIN_RECT.setFillColor(OPTION_PANEL_COLOR);
		OP_MAIN_RECT.setPosition(0, 0);

		OP_MARKER.setRadius(5);
		OP_MARKER.setFillColor(sf::Color::Green);

		OP_TEXT.setFillColor(sf::Color::Black);
		OP_TEXT.setFont(font);
		OP_TEXT.setStyle(sf::Text::Bold);
		OP_TEXT.setCharacterSize(50);

		controls_texture.loadFromFile("startMenu.png");
		controls_sprite = sf::Sprite(controls_texture);
	}


	option_t::option_t(int id, std::string str_name, std::string str_units, double* state, double min_state, double max_state)
	{
		this->id = id;
		this->str_name = str_name;
		this->str_units = str_units;
		this->state = state;
		this->min_state = min_state;
		this->max_state = max_state;
	}

	option_t options[OPTIONS_COUNT] = { option_t(0, "Volume: ", "%", &soundMaker::global_volume, 0, 100),
										option_t(1, "Frequency: ", "Hz", &soundMaker::base_frequency, 0, 20000),
	};

	void DrawOptionPanel(sf::RenderWindow& window)
	{
		int k = 3;
		int marker_offset = 15;

		window.draw(OP_MAIN_RECT);

		for (auto option : options)
		{
			std::string state_str = std::to_string((int)*option.state);
			OP_TEXT.setString(option.str_name + state_str + option.str_units);
			OP_TEXT.setPosition(WINDOW_SIZE.x / k, 5 * (option.id + 1) + 65 * option.id);
			window.draw(OP_TEXT);
		}

		for (int i = 0; i < soundMaker::instruments_count; i++)
		{
			std::string state_str = (soundMaker::current_instrument == i) ? "On" : "Off";
			OP_TEXT.setString((*soundMaker::instruments[i]).name + state_str);
			OP_TEXT.setPosition(WINDOW_SIZE.x / k, 140 + 5 * (i + 1) + 65 * i);
			window.draw(OP_TEXT);
		}
		OP_MARKER.setPosition(OP_TEXT.getPosition().x - OP_MARKER.getRadius() - marker_offset, 30 + 70 * current_option);
		window.draw(OP_MARKER);
		window.display();
		OP_update = false;
	}

	void DrawPiano(sf::RenderWindow& window)
	{
		CheckKeys();
		std::vector<sf::RectangleShape> black_keys;
		for (key_t key : keys_arr)
		{
			if (!key.type)
				window.draw(key.rectangle_shape);
			else
				black_keys.push_back(key.rectangle_shape);
		}
		for (auto rect : black_keys)
			window.draw(rect);
		window.display();
	}

	void DrawWaves(sf::RenderWindow& window, double time)
	{
		static sf::Vertex frequency_point1(FW_DEFAULT_POS);
		sf::Vertex frequency_point2;

		if (wave_update)
		{
			wave_update = false;
			ClearWindow(window, sf::Vector2f(0, 0), WINDOW_SIZE, sf::Color::Black);
			frequency_point1.position.x = FW_DEFAULT_POS.x;
		}

		frequency_point2.position.x = FW_DEFAULT_POS.x + time * FW_COEFF;
		frequency_point2.position.y = FW_DEFAULT_POS.y;

		for (auto& n : soundMaker::notes_vector)
		{
			frequency_point2.position.y += (*soundMaker::instruments[soundMaker::current_instrument]).Sound(time, n) * FW_COEFF * 10 * 0.001 * soundMaker::global_volume;
		}

		frequency_point2.position.x = (int)frequency_point2.position.x % (int)WINDOW_SIZE.x;
		if (frequency_point2.position.x <= 3)
		{
			ClearWindow(window, sf::Vector2f(0, 0), WINDOW_SIZE, sf::Color::Black);
		}

		sf::Vertex line[] = { frequency_point1, frequency_point2 };
		line->color = sf::Color(0, 255, 0);

		window.draw(line, 2, sf::Lines);
		window.display();
		frequency_point1.position = frequency_point2.position;
	}


	void DrawWindows(soundLoader_t<short>& sl)
	{
		sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE.x, WINDOW_SIZE.y), "Synthesizer");

		window.setVerticalSyncEnabled(true);

		Windows old_window_type = window_type;
		window.draw(controls_sprite);
		window.display();
		//DrawPiano(window);

		while (window.isOpen())
		{
			if (old_window_type != window_type)
			{
				old_window_type = window_type;
				if (window_type == Windows::Piano)
					piano_update = true;
				ClearWindow(window, sf::Vector2f(0, 0), WINDOW_SIZE, sf::Color::Black);
			}
			if (window_type == Windows::Waves)
				DrawWaves(window, sl.GetTime());
			else if ((window_type == Windows::Piano) && piano_update)
			{
				DrawPiano(window);
				piano_update = false;
			}
			else if (window_type == Windows::Options && OP_update)
			{
				DrawOptionPanel(window);
			}

			sf::Event event;
			while (window.pollEvent(event))
			{

				if (event.type == sf::Event::KeyPressed)
				{
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
					{
						window_type = Windows::Waves;
						wave_update = true;
					}
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
					{
						window_type = Windows::Piano;
						piano_update = true;
					}
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
					{
						window_type = Windows::Options;
						OP_update = true;
					}
					else if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) && (window_type == Windows::Start))
					{
						window_type = Windows::Piano;
						piano_update = true;
					}
					else if (window_type == Windows::Options)
					{
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && (current_option > 0))
						{
							current_option--;
							OP_update = true;
						}
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && (current_option < OPTIONS_COUNT - 1 + soundMaker::instruments_count))
						{
							current_option++;
							OP_update = true;
						}
						else if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Equal) || sf::Keyboard::isKeyPressed(sf::Keyboard::Dash)) && (current_option < OPTIONS_COUNT))
						{
							double& parameter = *options[current_option].state;
							double min = options[current_option].min_state;
							double max = options[current_option].max_state;

							int delta = (sf::Keyboard::isKeyPressed(sf::Keyboard::Equal)) ? 1 : -1;
							if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
							{
								delta *= 10;
								if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
									delta *= 10;
							}

							if (parameter + delta > max)
								parameter = max;
							else if (parameter + delta < min)
								parameter = min;
							else
								parameter += delta;
							OP_update = true;
						}
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
						{
							if ((window_type == Windows::Options) && (current_option > OPTIONS_COUNT - 1))
							{
								soundMaker::current_instrument = current_option - OPTIONS_COUNT;
								OP_update = true;
							}
						}
					}
				}


				if (event.type == sf::Event::Closed)
				{
					window.close();
					window_is_open = false;
				}
			}
		}
	}

}