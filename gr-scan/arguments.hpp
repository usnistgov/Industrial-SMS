/*
	gr-scan - A GNU Radio signal scanner
	Copyright (C) 2015 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
	Copyright (C) 2012  Nicholas Tomlinson
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <argp.h>
#include <string>

class Arguments
{
public:
	Arguments(int argc, char **argv) :
		avg_size(1000),
		start_freq(87000000.0),
		end_freq(108000000.0),
		sample_rate(2000000.0),
		fft_width(1000.0),
		step(-1.0),
		gain_a(0.0),
		gain_if(0.0),
		gain_m(0.0),
		gain_total(0.0),
		use_AGC(1)
	{
		argp_parse (&argp_i, argc, argv, 0, 0, this);
	}

	unsigned int get_avg_size()
	{
		return avg_size;
	}

	double get_start_freq()
	{
		return start_freq;
	}

	double get_end_freq()
	{
		return end_freq;
	}

	double get_sample_rate()
	{
		return sample_rate;
	}

	double get_fft_width()
	{
		return fft_width;
	}

	double get_step()
	{
		if (step < 0.0)
			return sample_rate / 4.0; // I've found this to be a good choice (slightly faster might be / 3.0)
		else
			return step;
	}

	double get_gain_a() { return gain_a; }
	double get_gain_m() { return gain_m; }
	double get_gain_if() { return gain_if; }
	double get_gain_total() { return gain_total; }
	int get_use_AGC() { return use_AGC; }

private:
	static error_t s_parse_opt(int key, char *arg, struct argp_state *state)
	{
		Arguments *arguments = static_cast<Arguments *>(state->input);
		return arguments->parse_opt(key, arg, state);
	}

	error_t parse_opt(int key, char *arg, struct argp_state *state)
	{
		switch (key)
		{
		case 'a':
			avg_size = atoi(arg);
			break;
		case 'x':
			start_freq = atof(arg) * 1000000.0; //MHz
			break;
		case 'y':
			end_freq = atof(arg) * 1000000.0; //MHz
			break;
		case 'r':
			sample_rate = atof(arg) * 1000000.0; //MSamples/s
			break;
		case 'w':
			fft_width = atoi(arg);
			break;
		case 'z':
			step = atof(arg) * 1000000.0; //MHz
			break;
		case 'g':
			gain_m = atof(arg);
			break;
		case 'i':
			gain_if = atof(arg);
			break;
		case 't':
			gain_a = atof(arg);
			break;
		case 'G':
			gain_total = atof(arg);
			break;
		case 'A':
			use_AGC = atoi(arg);
			break;
		case ARGP_KEY_ARG:
			if (state->arg_num > 0)
				argp_usage(state);
			break;
		case ARGP_KEY_END:
			break;
		default:
			return ARGP_ERR_UNKNOWN;
		}
		return 0;
	}

	static argp_option options[];
	static argp argp_i;

	unsigned int avg_size;
	double start_freq;
	double end_freq;
	double sample_rate;
	double fft_width;
	double step;
	double gain_a;
	double gain_if;
	double gain_m;
	double gain_total;
	int use_AGC;
};

argp_option Arguments::options[] = {
	{"average", 'a', "COUNT", 0, "Average over COUNT samples (default: 1000)"},
	{"start-frequency", 'x', "FREQ", 0, "Start frequency in MHz (default: 87)"},
	{"end-frequency", 'y', "FREQ", 0, "End frequency in MHz (default: 108)"},
	{"sample-rate", 'r', "RATE", 0, "Samplerate in Msamples/s (default: 2)"},
	{"fft-width", 'w', "COUNT", 0, "Width of FFT in samples (default: 1000)"},
	{"step", 'z', "FREQ", 0, "Increment step in MHz (default: sample_rate / 4)"},
	{"gain_main", 'g', "GAINM", 0, "main gain"},
	{"gain_if", 'i', "GAINIF", 0, "IF gain"},
	{"gain_ant", 't', "GAINANT", 0, "antenna gain"},
	{"gain_total", 'G', "GAINTOTAL", 0, "total gain (overrides individual gains)"},
	{"use_AGC", 'A', "USEAGC", 0, "use agc (0 - turn off, 1 - turn on, on by default)"},
	{0}
};

argp Arguments::argp_i = {options, s_parse_opt, 0, 0};

const char *argp_program_bug_address = "Jason@zx2c4.com";
const char *argp_program_version = 
		"gr-scan " VERSION " - A GNU Radio signal scanner\n"
				   "Copyright (C) 2015 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.\n"
				   "Copyright (C) 2012  Nicholas Tomlinson\n"
				   "\n"
				   "This program is free software: you can redistribute it and/or modify\n"
				   "it under the terms of the GNU General Public License as published by\n"
				   "the Free Software Foundation, either version 3 of the License, or\n"
				   "(at your option) any later version.\n"
				   "\n"
				   "This program is distributed in the hope that it will be useful,\n"
				   "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
				   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
				   "GNU General Public License for more details.\n"
				   "\n"
				   "You should have received a copy of the GNU General Public License\n"
				   "along with this program.  If not, see <http://www.gnu.org/licenses/>.";
