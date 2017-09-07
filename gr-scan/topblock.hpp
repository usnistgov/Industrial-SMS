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


#include <cmath>
#include <stdint.h>

#include <gnuradio/top_block.h>
#include <osmosdr/source.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/fft/fft_vcc.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/filter/single_pole_iir_filter_ff.h>
#include <gnuradio/blocks/nlog10_ff.h>
#include "scanner_sink.hpp"

class TopBlock : public gr::top_block
{
public:
	TopBlock(double start_freq, double end_freq, double sample_rate,
		 double fft_width, double step, unsigned int avg_size, 
		double gain_a, float gain_m, float gain_if, float total_gain, int use_AGC) :
		gr::top_block("Top Block"),
		vector_length(fft_width),
		window(GetWindow(vector_length)),
		source(osmosdr::source::make()), /* OsmoSDR Source */
		stv(gr::blocks::stream_to_vector::make(sizeof(float) * 2, vector_length)), /* Stream to vector */
		/* Based on the logpwrfft (a block implemented in python) */
		fft(gr::fft::fft_vcc::make(vector_length, true, window, false, 1)),
		ctf(gr::blocks::complex_to_mag_squared::make(vector_length)),
		iir(gr::filter::single_pole_iir_filter_ff::make(1.0, vector_length)),
		lg(gr::blocks::nlog10_ff::make(10, vector_length, -20 * std::log10(float(vector_length)) -10 * std::log10(float(GetWindowPower() / vector_length))))
		/* Sink - this does most of the interesting work */
	{
		/* Set up the OsmoSDR Source */
		source->set_sample_rate(sample_rate);
		source->set_center_freq(start_freq);
		source->set_freq_corr(0.0);

		const std::vector<std::string>gains = source->get_gain_names();
		for(std::vector<std::string>::const_iterator ii = gains.begin(); ii != gains.end(); ++ii)
		{
			printf("gain: %s\n", ii->c_str());
		}

		source->set_gain_mode(false);
		if(total_gain > 0)
			source->set_gain(total_gain);
		else
		{
			source->set_gain(gain_m, "BB");
			if(!use_AGC)
			{
				source->set_gain(gain_a, "RF");
				source->set_gain(gain_if, "IF");
			}
			else
			{
				source->set_gain(0, "RF");
				source->set_gain(0, "IF");
			}
		}

		float resulting_gain = source->get_gain("RF") + source->get_gain("BB") + source->get_gain("IF");

		sink = make_scanner_sink(source, vector_length, start_freq, end_freq, sample_rate, step, avg_size, resulting_gain, use_AGC);
		/* Set up the connections */
		connect(source, 0, stv, 0);
		connect(stv, 0, fft, 0);
		connect(fft, 0, ctf, 0);
//		connect(ctf, 0, iir, 0);
//		connect(iir, 0, lg, 0);
//		connect(lg, 0, sink, 0);
		connect(ctf, 0, sink, 0);
	}

private:
	/* http://en.wikipedia.org/w/index.php?title=Window_function&oldid=508445914 */
	std::vector<float> GetWindow(size_t n)
	{
		std::vector<float> w;
		w.resize(n);

		double a = 0.16;
		double a0 = (1.0 - a)/2.0;
		double a1 = 0.5;
		double a2 = a/2.0;

		for (unsigned int i = 0; i < n; ++i)
			w[i] = a0 - a1 * ::cos((2.0 * 3.14159 * static_cast<double>(i))/static_cast<double>(n - 1)) + a2 * ::cos((4.0 * 3.14159 * static_cast<double>(i))/static_cast<double>(n - 1));
		return w;
	}

	double GetWindowPower()
	{
		double total = 0.0;
		BOOST_FOREACH (double d, window)
			total += d * d;
		return total;
	}

	size_t vector_length;
	std::vector<float> window;
	osmosdr::source::sptr source;
	gr::blocks::stream_to_vector::sptr stv;
	gr::fft::fft_vcc::sptr fft;
	gr::blocks::complex_to_mag_squared::sptr ctf;
	gr::filter::single_pole_iir_filter_ff::sptr iir;
	gr::blocks::nlog10_ff::sptr lg;
	scanner_sink_sptr sink;
};
