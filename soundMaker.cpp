#pragma once


#include <Windows.h>
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <cmath>

#include "soundMaker.h"



namespace soundMaker {

	Damping damping;

	std::vector<Note> notes_vector;

	double base_frequency = 261.0;
	double global_volume = 100.0;

	std::mutex note_mutex;

	int current_instrument = 0;
	int instruments_count;
	std::vector<std::unique_ptr<instrumentBase>> instruments;

	double Radians(double frequency)
	{
		return 2.0 * PI * frequency;
	}

	double GetFrequency(int id)
	{
		return base_frequency * pow(SEMITON, id);
	}

	double Oscillator(double frequency, double time, WaveShape shape)
	{
		double output = 0.0;
		switch (shape)
		{
		case WaveShape::Sin:
			output = sin(Radians(frequency) * time);
			break;
		case WaveShape::Triangle:
			output = asin(sin(Radians(frequency) * time)) * (2.0 / PI);
			break;
		case WaveShape::Square:
			output = (sin(Radians(frequency) * time) > 0.0) ? 1.0 : -1.0;
			break;
		case WaveShape::Saw:
			output = (2.0 / PI) * (frequency * PI * fmod(time, 1.0 / frequency) - (PI / 2.0));
			break;
		case WaveShape::Unit:
			output = (sin(Radians(frequency) * time) > 0.0) ? 1.0 : 0.0;
		default:
			output = 0.0;
			break;
		}
		return output;
	}

	Note::Note()
	{
		id = 0;
		time_on = 0.0;
		time_off = 0.0;
		is_active = false;
		frequency = 0.0;
	}

	Damping::Damping()
	{
		attack_time = 0.1;
		decay_time = 0.01;
		release_time = 0.02;
		sustain_amplitude = 0.8;
		start_amplitude = 1.0;
		trigger_off_time = 0.0;
		trigger_on_time = 0.0;
		note_on = false;
	}

	double Damping::GetAmplitude(double time, Note& n)
	{
		double amplitude = 0.0;
		trigger_on_time = n.time_on;
		trigger_off_time = n.time_off;
		note_on = (trigger_off_time < trigger_on_time);
		double life_time = time - trigger_on_time;

		if (note_on)
		{
			if (life_time <= attack_time)
				amplitude = (life_time / attack_time) * start_amplitude;
			else if ((life_time > attack_time) && (life_time <= (attack_time + decay_time)))
				amplitude = ((life_time - attack_time) / decay_time) * (sustain_amplitude - start_amplitude) + start_amplitude;
			else if (life_time > (attack_time + decay_time))
			{
				amplitude = sustain_amplitude;
			}
		}
		else
		{
			amplitude = ((time - trigger_off_time) / release_time) * (0.0 - sustain_amplitude) + sustain_amplitude;
		}
		if (amplitude <= 0.0001)
		{
			amplitude = 0.0;
			n.is_active = false;
		}
		return amplitude;
	}



	void DeleteNotes(std::vector<Note>& notes)
	{
		auto itr = notes.begin();
		while (itr != notes.end())
		{
			if (!itr->is_active)
			{
				itr = notes.erase(itr);
			}
			else
				++itr;
		}
	}

	instrumentSinWave::instrumentSinWave()
	{
		damp = Damping();
		name = "Instrument - Sin wave: ";
	}

	double instrumentSinWave::Sound(double time, Note& n)
	{
		double amplitude = damp.GetAmplitude(time, n);
		double output = 0.0;
		if (global_volume < 0.0 || global_volume > 100)
			return output;
		output = amplitude *
			(
				Oscillator(GetFrequency(n.id), time, WaveShape::Sin)
				);


		return output;
	}

	instrumentTriangleWave::instrumentTriangleWave()
	{
		damp = Damping();
		name = "Instrument - Triangle wave: ";
	}

	double instrumentTriangleWave::Sound(double time, Note& n)
	{
		double amplitude = damp.GetAmplitude(time, n);
		double output = 0.0;
		if (global_volume < 0.0 || global_volume > 100)
			return output;
		output = amplitude *
			(
				Oscillator(GetFrequency(n.id), time, WaveShape::Triangle)
				);


		return output;
	}

	instrumentSquareWave::instrumentSquareWave()
	{
		damp = Damping();
		name = "Instrument - Square wave: ";
	}

	double instrumentSquareWave::Sound(double time, Note& n)
	{
		double amplitude = damp.GetAmplitude(time, n);
		double output = 0.0;
		if (global_volume < 0.0 || global_volume > 100)
			return output;
		output = amplitude *
			(
				Oscillator(GetFrequency(n.id), time, WaveShape::Square)
				);


		return output;
	}

	instrumentSawWave::instrumentSawWave()
	{
		damp = Damping();
		name = "Instrument - Saw Wave: ";
	}

	double instrumentSawWave::Sound(double time, Note& n)
	{
		double amplitude = damp.GetAmplitude(time, n);
		double output = 0.0;
		if (global_volume < 0.0 || global_volume > 100)
			return output;
		output = amplitude *
			(
				Oscillator(GetFrequency(n.id), time, WaveShape::Saw)
				);


		return output;
	}


	instrument1::instrument1()
	{
		damp.attack_time = 0.1;
		damp.decay_time = 0.01;
		damp.release_time = 0.1;
		damp.sustain_amplitude = 0.8;
		name = "Instrument - 1: ";
	}

	double instrument1::Sound(double time, Note& n)
	{
		double amplitude = damp.GetAmplitude(time, n);
		double output = 0.0;
		if (global_volume < 0.0 || global_volume > 100)
			return output;
		output = amplitude *
			(
				Oscillator(GetFrequency(n.id), time, WaveShape::Sin) +
				Oscillator(GetFrequency(n.id) * 2, time, WaveShape::Triangle)
				);


		return output;
	}

	instrument2::instrument2()
	{
		damp = Damping();
		

		name = "Instrument - 2: ";
	}

	double instrument2::Sound(double time, Note& n)
	{
		double amplitude = damp.GetAmplitude(time, n);
		double output = 0.0;
		if (global_volume < 0.0 || global_volume > 100)
			return output;
		output = amplitude *
				(
				Oscillator(GetFrequency(n.id) , time, WaveShape::Triangle) *
				Oscillator(GetFrequency(n.id) , time, WaveShape::Triangle) +
				Oscillator(GetFrequency(n.id) , time, WaveShape::Sin)
				);


		return output;
	}

	instrument3::instrument3()
	{
		damp = Damping();
		damp.start_amplitude = 1.0;
		damp.sustain_amplitude = 0.7;
		damp.attack_time = 0.1;
		damp.decay_time = 0.3;

		name = "Instrument - 3: ";
	}

	double instrument3::Sound(double time, Note& n)
	{
		double amplitude = damp.GetAmplitude(time, n);
		double output = 0.0;
		if (global_volume < 0.0 || global_volume > 100)
			return output;
		output = amplitude *
			(
				Oscillator(GetFrequency(n.id), time, WaveShape::Sin) *
				Oscillator(GetFrequency(n.id) * 1.5, time, WaveShape::Sin) +
				Oscillator(GetFrequency(n.id) * 2.0, time, WaveShape::Triangle) *
				Oscillator(GetFrequency(n.id) * 2.0, time, WaveShape::Sin)
				);


		return output;
	}
	

	void PrepareInstruments()
	{
		instruments.push_back(std::make_unique<instrumentSinWave>());
		instruments.push_back(std::make_unique<instrumentTriangleWave>());
		instruments.push_back(std::make_unique<instrumentSquareWave>());
		instruments.push_back(std::make_unique<instrumentSawWave>());
		instruments.push_back(std::make_unique<instrument1>());
		instruments.push_back(std::make_unique<instrument2>());
		instruments.push_back(std::make_unique<instrument3>());
		instruments_count = instruments.size();
	}

	double MakeSound(int number, double time)
	{
		std::unique_lock<std::mutex> lock(note_mutex);
		double mixed_output = 0.0;

		for (auto& n : notes_vector)
		{
			mixed_output += instruments[current_instrument]->Sound(time, n);
		}
		DeleteNotes(notes_vector);
		return mixed_output * 0.001 * global_volume;
	}
}