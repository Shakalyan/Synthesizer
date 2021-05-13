#pragma once

#pragma comment(lib, "winmm.lib")

#include <iostream>
#include <Windows.h>
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <cmath>


template<class SoundDepth_t>
class soundLoader_t
{
public:

	soundLoader_t(std::wstring output_device, unsigned int frequency = 44100, unsigned int channels = 1, unsigned int blocks = 8, unsigned int samples_per_block = 256)
	{
		Create(output_device, frequency, channels, blocks, samples_per_block);
	}

	bool Create(std::wstring output_device, unsigned int frequency, unsigned int channels, unsigned int blocks, unsigned int samples_per_block)
	{
		m_frequency = frequency;
		m_channels_count = channels;
		m_blocks_count = blocks;
		m_samples_per_block = samples_per_block;
		m_free_blocks_count = blocks;
		m_current_block = 0;
		m_block_memory_pointer = nullptr;
		m_headers_pointer = nullptr;
		m_UserFunction = nullptr;
		m_is_ready = false;


		std::vector<std::wstring> devices = GetAudioDevices();
		auto device = find(devices.begin(), devices.end(), output_device);
		if (device != devices.end())
		{
			int device_ID = distance(devices.begin(), device);
			WAVEFORMATEX wave_format_ex;
			wave_format_ex.wFormatTag = WAVE_FORMAT_PCM;
			wave_format_ex.nSamplesPerSec = m_frequency;
			wave_format_ex.wBitsPerSample = sizeof(SoundDepth_t) * 8;
			wave_format_ex.nChannels = m_channels_count;
			wave_format_ex.nBlockAlign = (wave_format_ex.wBitsPerSample / 8) * wave_format_ex.nChannels;
			wave_format_ex.nAvgBytesPerSec = wave_format_ex.nBlockAlign * wave_format_ex.nSamplesPerSec;
			wave_format_ex.cbSize = 0;

			if (waveOutOpen(&m_handle_wave_out, device_ID, &wave_format_ex, (DWORD_PTR)WaveOutProcWrap, (DWORD_PTR)this, CALLBACK_FUNCTION) != S_OK)
				return Destroy();
		}


		m_block_memory_pointer = new SoundDepth_t[m_blocks_count * m_samples_per_block];
		if (m_block_memory_pointer == nullptr)
			return Destroy();
		ZeroMemory(m_block_memory_pointer, sizeof(SoundDepth_t) * m_blocks_count * m_samples_per_block);

		m_headers_pointer = new WAVEHDR[m_blocks_count];
		if (m_headers_pointer == nullptr)
			return Destroy();
		ZeroMemory(m_headers_pointer, sizeof(WAVEHDR) * m_blocks_count);


		for (unsigned int i = 0; i < m_blocks_count; i++)
		{
			m_headers_pointer[i].dwBufferLength = m_samples_per_block * sizeof(SoundDepth_t);
			m_headers_pointer[i].lpData = (LPSTR)(m_block_memory_pointer + (i * m_samples_per_block));
		}
		m_is_ready = true;

		m_main_thread = std::thread(&soundLoader_t::MainThread, this);

		std::unique_lock<std::mutex> lock(m_mutex_fill_free_block);
		m_cv_fill_free_block.notify_one();
		
		
		return true;
	}

	bool Destroy()
	{
		return false;
	}

	void Stop()
	{
		m_is_ready = false;
		m_main_thread.join();
	}

	static std::vector<std::wstring> GetAudioDevices()
	{
		int devices_count = waveOutGetNumDevs();
		std::vector<std::wstring> devices;
		WAVEOUTCAPS wave_out_caps;
		for (int i = 0; i < devices_count; i++)
		{
			if (waveOutGetDevCaps(i, &wave_out_caps, sizeof(WAVEOUTCAPS)) == S_OK)
				devices.push_back(wave_out_caps.szPname);
		}
		return devices;
	}

	void SetUserFuncion(double(*function)(int, double))
	{
		m_UserFunction = function;
	}

	double Cut(double sample, double max_sample)
	{
		if (sample >= 0.0)
			return fmin(sample, max_sample);
		else
			return fmax(sample, -max_sample);
	}

	virtual double DefaultFunction(int channels, double time)
	{
		return 0.0;
	}

	double GetTime()
	{
		return m_global_time;
	}


private:
	double(*m_UserFunction)(int, double);

	unsigned int m_frequency;
	unsigned int m_channels_count;
	unsigned int m_blocks_count;
	unsigned int m_samples_per_block;
	unsigned int m_current_block;
	
	std::thread m_main_thread;
	std::atomic<bool> m_is_ready;
	std::atomic<unsigned int> m_free_blocks_count;
	std::atomic<double> m_global_time;
	std::mutex m_mutex_fill_free_block;
	std::condition_variable m_cv_fill_free_block;
	


	SoundDepth_t* m_block_memory_pointer;
	WAVEHDR* m_headers_pointer;
	HWAVEOUT m_handle_wave_out;



	void WaveOutProc(HWAVEOUT header_wave_out, UINT message, DWORD parameter1, DWORD parameter2)
	{
		if (message != WOM_DONE)
			return;
		m_free_blocks_count++;
		try
		{
			std::unique_lock<std::mutex> lock(m_mutex_fill_free_block);
			m_cv_fill_free_block.notify_one();
		}
		catch (...) {}
	}

	static void CALLBACK WaveOutProcWrap(HWAVEOUT h_wave_out, UINT message, DWORD instance, DWORD parameter1, DWORD parameter2)
	{
		((soundLoader_t*)instance)->WaveOutProc(h_wave_out, message, parameter1, parameter2);
	}

	void MainThread()
	{
		m_global_time = 0.0;
		double time_step = 1.0 / m_frequency;
		double max_sample_value = static_cast<double>(pow(2, sizeof(SoundDepth_t) * 8 - 1) - 1);

		while (m_is_ready)
		{

			if (m_free_blocks_count == 0)
			{
				std::unique_lock<std::mutex> lock(m_mutex_fill_free_block);
				while (m_free_blocks_count == 0)
					m_cv_fill_free_block.wait(lock);
			}

			m_free_blocks_count--;

			if (m_headers_pointer[m_current_block].dwFlags & WHDR_PREPARED)
				waveOutUnprepareHeader(m_handle_wave_out, &m_headers_pointer[m_current_block], sizeof(WAVEHDR));

			SoundDepth_t new_sample = 0;
			int current_block = m_current_block * m_samples_per_block;

			for (unsigned int i = 0; i < m_samples_per_block; i += m_channels_count)
			{
				for (int c = 0; c < m_channels_count; c++)
				{
					if (m_UserFunction == nullptr)
						new_sample = static_cast<SoundDepth_t>(Cut(DefaultFunction(m_channels_count, m_global_time), 1.0) * max_sample_value);
					else
						new_sample = static_cast<SoundDepth_t>(Cut(m_UserFunction(m_channels_count, m_global_time), 1.0) * max_sample_value);

					m_block_memory_pointer[current_block + i + c] = new_sample;						
				}
				m_global_time = m_global_time + time_step;
			}

			waveOutPrepareHeader(m_handle_wave_out, &m_headers_pointer[m_current_block], sizeof(WAVEHDR));
			waveOutWrite(m_handle_wave_out, &m_headers_pointer[m_current_block], sizeof(WAVEHDR));
			m_current_block++;
			m_current_block %= m_blocks_count;
		}

	}

};