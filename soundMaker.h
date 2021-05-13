#pragma once



namespace soundMaker {

	struct instrumentBase;
	struct Note;
	struct Damping;

	const double PI = 3.14159;
	const int NUMBER_OF_KEYS = 17;
	const double SEMITON = pow(2, 1.0 / 12.0);
	const unsigned char KEYBOARD[NUMBER_OF_KEYS + 1] = "ZSXDCVGBHNJM\xbcL\xbe\xba\xbf";

	extern double base_frequency;
	extern std::mutex note_mutex;
	extern double global_volume;
	extern int instruments_count;
	extern int current_instrument;
	extern Damping damping;
	extern std::vector<Note> notes_vector;
	extern std::vector<std::unique_ptr<instrumentBase>> instruments;

	double Radians(double frequency);

	double GetFrequency(int id);

	enum class WaveShape
	{
		Sin,
		Triangle,
		Square,
		Saw,
		Unit
	};

	double Oscillator(double frequency, double time, WaveShape shape);

	struct Note
	{
		int id;
		double time_on;
		double time_off;
		bool is_active;
		double frequency;
		Note();
	};

	struct Damping
	{
		double attack_time;
		double decay_time;
		double sustain_amplitude;
		double release_time;
		double start_amplitude;
		double trigger_on_time;
		double trigger_off_time;
		bool note_on;

		Damping();

		double GetAmplitude(double time, Note& n);

	};


	void DeleteNotes(std::vector<Note>& notes);


	struct instrumentBase
	{
		Damping damp;
		std::string name;
		virtual double Sound(double, Note&) = 0;
	};

	struct instrumentSinWave : instrumentBase
	{
		instrumentSinWave();

		double Sound(double time, Note& n);
	};

	struct instrumentTriangleWave : instrumentBase
	{
		instrumentTriangleWave();

		double Sound(double time, Note& n);
	};

	struct instrumentSquareWave : instrumentBase
	{
		instrumentSquareWave();

		double Sound(double time, Note& n);
	};

	struct instrumentSawWave : instrumentBase
	{
		instrumentSawWave();

		double Sound(double time, Note& n);
	};

	struct instrument1 : instrumentBase
	{
		instrument1();

		double Sound(double time, Note& n);
	};

	struct instrument2 : instrumentBase
	{
		instrument2();

		double Sound(double time, Note& n);
	};

	struct instrument3 : instrumentBase
	{
		instrument3();

		double Sound(double time, Note& n);
	};


	void PrepareInstruments();


	double MakeSound(int number, double time);
}