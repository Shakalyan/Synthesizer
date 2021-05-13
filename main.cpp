#define NOMINMAX

#include "soundDrawing.h"


int main()
{
	

	soundDrawing::StartPreparations();
	
	
	std::vector<std::wstring> devices = soundLoader_t<short>::GetAudioDevices();
	soundLoader_t<short> sound(devices[0], 44100, 1, 8, 256);
	sound.SetUserFuncion(soundMaker::MakeSound);

	std::thread draw_thread(&soundDrawing::DrawWindows, std::ref(sound));

	int current_key = -1;

	while (soundDrawing::window_is_open)
	{
		soundMaker::Note n;
		if (soundDrawing::window_type == soundDrawing::Windows::Start)
			continue;
		for (int key = 0; key < soundMaker::NUMBER_OF_KEYS; key++)
		{
			short key_state = GetAsyncKeyState(soundMaker::KEYBOARD[key]);
			double time_now = sound.GetTime();
			soundMaker::note_mutex.lock();
			auto note_found = std::find_if(soundMaker::notes_vector.begin(), soundMaker::notes_vector.end(), [&key](soundMaker::Note const& note) { return note.id == key; });
			
			if (note_found == soundMaker::notes_vector.end())
			{
				if (key_state && 0x8000)
				{
					n.id = key;
					n.time_on = time_now;
					n.is_active = true;
					soundDrawing::keys_arr[key].is_active = true;
					soundMaker::notes_vector.push_back(n);
					soundDrawing::piano_update = true;
				}
			}
			else
			{
				if (key_state && 0x8000)
				{
					;
				}
				else
				{
					if (note_found->time_off < note_found->time_on)
					{
						note_found->time_off = time_now;
						soundDrawing::keys_arr[key].is_active = false;
						soundDrawing::piano_update = true;
					}
				}
			}
			soundMaker::note_mutex.unlock();
		}
	}

	draw_thread.join();

	sound.Stop();
		
	return 0;
}
