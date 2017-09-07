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

#include <ctime>
#include <set>
#include <utility>

#include <boost/shared_ptr.hpp>

#include <gnuradio/block.h>
#include <gnuradio/io_signature.h>
#include <osmosdr/source.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1000000

class scanner_sink : public gr::block
{
public:
	scanner_sink(osmosdr::source::sptr source, unsigned int vector_length, double start_freq,
		     double end_freq, double samples_per_second, double step, 
		unsigned int avg_size, double def_gain, int use_AGC) :
		gr::block("scanner_sink",
			  gr::io_signature::make(1, 1, sizeof (float) * vector_length),
			  gr::io_signature::make(0, 0, 0)),
		m_source(source), //We need the source in order to be able to control it
		m_buffer(new float[vector_length]), //buffer into which we accumulate the total for averaging
		m_vector_length(vector_length), //size of the FFT
		m_count(0), //number of FFTs totalled in the buffer
		m_wait_count(0), //number of times we've listenned on this frequency
		m_avg_size(avg_size), //the number of FFTs we should average over
		m_step(step), //the amount by which the frequency shold be incremented
		m_start_freq(start_freq), //start frequency
		m_end_freq(end_freq), //end frequency
		m_sps(samples_per_second), //samples per second
		m_start_time(time(0)), //the start time of the scan (useful for logging/reporting/monitoring)
		m_default_gain(def_gain)
	{
		current_gain_RF = 0;
		current_gain_IF = 0;
		rf_gain_mod = 0; //compensation for RF gain not equal to 14dB in hardware
		agc_threshold_low = 0.01;
		agc_threshold_high = 2.0;
		agc_power_level = 0.5*(agc_threshold_high + agc_threshold_low); //init at value that won't force gain change at the beginning

		gain_change_timeout = 0;
		m_current_freq = start_freq;
		m_use_AGC = use_AGC;

		last_log_out = 0;
		ZeroBuffer();
		key_t key = 47192032; //some random number that must be the same in monitor shared mem module
		int shmid;

		if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
		printf("shmget error!\n");
		}

		if ((shared_memory = (uint8_t*)shmat(shmid, NULL, 0)) == (uint8_t *) -1) {
		printf("shmat error!\n");
		}
	}

	virtual ~scanner_sink()
	{
		delete []m_buffer; //delete the buffer
	}

private:
	virtual int general_work(int noutput_items, gr_vector_int &ninput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items)
	{
		for (int i = 0; i < ninput_items[0]; ++i)
			ProcessVector(static_cast<const float *>(input_items[0]) + i * m_vector_length);

		consume_each(ninput_items[0]);
		return 0;
	}

	void ProcessVector(const float *input)
	{
		float sample_max = -100;
		float sample_average = 0;
		float sample_top_average = 0;
		float avg_z = 0;
		float avg_top_z = 0;
		float signal_mult = 1.0;//pow(10, -(m_default_gain + current_gain) * 0.1);
		for (unsigned int i = 0; i < m_vector_length; ++i)
		{
			if(i > 10 && i < m_vector_length - 10)
			{
				sample_average += input[i];
				avg_z++;
				if(sample_max < input[i]) sample_max = input[i];
			}
			m_buffer[i] += input[i]*signal_mult; //if gain is turned on, adjust signal to fit
		}
		sample_average /= avg_z;
		++m_count; //increment the total

		if(current_gain_RF > 1) rf_gain_mod = -8;
		else rf_gain_mod = 0;
		if(m_use_AGC)
		{
			for (unsigned int i = 0; i < m_vector_length; ++i)
			{
				if(i > 10 && i < m_vector_length - 10)
					if(input[i] > sample_average)
					{
						sample_top_average += input[i];
						avg_top_z++;
					}
			}
			sample_top_average /= avg_top_z;

			agc_power_level *= 0.9;
			agc_power_level += 0.1 * sample_top_average;

			if(gain_change_timeout > 0) gain_change_timeout--;

			if(agc_power_level < agc_threshold_low && gain_change_timeout < 1) //increase gain
			{
				if(current_gain_RF < 1)
					current_gain_RF = 14;
				else
					current_gain_IF += 8;
				if(current_gain_IF > 40) current_gain_IF = 40;
				m_source->set_gain(current_gain_RF, "RF");
				m_source->set_gain(current_gain_IF, "IF");
				gain_change_timeout = 200;
			}

			if(agc_power_level > agc_threshold_high && gain_change_timeout < 1) //decrease gain
			{
				if(current_gain_IF > 0)
					current_gain_IF -= 8;
				else
					current_gain_RF = 0;
				if(current_gain_IF < 0) current_gain_IF = 0;
				m_source->set_gain(current_gain_RF, "RF");
				m_source->set_gain(current_gain_IF, "IF");
				gain_change_timeout = 200;
			}
		}

		if (m_count < m_avg_size) //we haven't yet averaged over the number we intended to
			return;


		double freqs[m_vector_length]; //for convenience
		float bands0[m_vector_length]; //bands in order of frequency

		Rearrange(bands0, freqs, m_current_freq, m_sps); //organise the buffer into a convenient order (saves to bands0)
		for(unsigned int n = 0; n < m_vector_length; n++)
		{
			bands0[n] = 10*log10(bands0[n]) - 38.0 - (m_default_gain + current_gain_IF + current_gain_RF + rf_gain_mod);
		}
		PrintSignals(freqs, bands0);

//		m_source->set_gain(m_default_gain); //by default, set gain to match 1dBm
//		m_gain_mode = 1;

		m_count = 0; //next time, we're starting from scratch - so note this
		ZeroBuffer(); //get ready to start again

		++m_wait_count; //we've just done another listen
		if(1){
			for (;;) { //keep moving to the next frequency until we get to one we can listen on (copes with holes in the tunable range)
				if (m_current_freq >= m_end_freq) { //we reached the end!
//					m_step = -m_step;
					//do something to end the scan
					fprintf(stderr, "[*] Finished range, starting again\n"); //say we're exiting
					m_current_freq = m_start_freq;
//					exit(0); //TODO: This probably isn't the right thing, but it'll do for now
				}

				m_current_freq += m_step; //calculate the frequency we should change to
				double actual = m_source->set_center_freq(m_current_freq); //change frequency
				if (fabs(m_current_freq - actual) < 100.0) //success
					break; //so stop changing frequency
			}
			m_wait_count = 0; //new frequency - we've listened 0 times on it
		}
	}

	void PrintSignals(double *freqs, float *bands0)
	{
		/* Calculate the current time after start */
		unsigned int t = time(NULL) - m_start_time;
		unsigned int hours = t / 3600;
		unsigned int minutes = (t % 3600) / 60;
		unsigned int seconds = t % 60;

		//Print that we finished scanning something
		fprintf(stderr, "%02u:%02u:%02u: Finished scanning %f MHz - %f MHz\n",
			hours, minutes, seconds, (m_current_freq - m_sps/2.0)/1000000.0, (m_current_freq + m_sps/2.0)/1000000.0);

		if(fabs(m_current_freq - last_log_out) >= 1000000.0)
		{
			last_log_out = m_current_freq;
			char logfn[512];
			sprintf(logfn, "logs/signal_%02u_%02u_%02u_%f_%f.txt", hours, minutes, seconds, (m_current_freq - m_sps/2.0)/1000000.0, (m_current_freq + m_sps/2.0)/1000000.0);
			int log_file = open(logfn, O_WRONLY | O_CREAT, 0b110110110);
			if(log_file > 0)
			{
				char line[1024];
				for (unsigned int i = 0; i < m_vector_length; i++){
					int lng = sprintf(line, "%g %g\n", freqs[i], bands0[i]);
					int sz = write(log_file, line, lng);
					if(sz < 0) printf("log file write failed\n");
				}
				close(log_file);
			}
		}
		float *f_shm = (float*)shared_memory;
		int *i_shm = (int*)shared_memory;
		i_shm[4] = m_vector_length;
		f_shm[1] = current_gain_IF + current_gain_RF + rf_gain_mod + m_default_gain;
	
		int rpos = 0;
		for(unsigned int r = 0; r < m_vector_length; r++)
		{
			f_shm[5 + rpos*2] = freqs[r];
			f_shm[6 + rpos*2] = bands0[r];
			rpos++;
		}

		i_shm[0]++;
	}

	void Rearrange(float *bands, double *freqs, double centre, double bandwidth)
	{
		double samplewidth = bandwidth/(double)m_vector_length;
		for (unsigned int i = 0; i < m_vector_length; ++i) {
			/* FFT is arranged starting at 0 Hz at the start, rather than in the middle */
			if (i < m_vector_length / 2) //lower half of the fft
				bands[i + m_vector_length / 2] = m_buffer[i] / static_cast<float>(m_avg_size);
			else //upper half of the fft
				bands[i - m_vector_length / 2] = m_buffer[i] / static_cast<float>(m_avg_size);

			freqs[i] = centre + i * samplewidth - bandwidth / 2.0; //calculate the frequency of this sample
		}
	}

	void ZeroBuffer()
	{
		/* writes zeros to m_buffer */
		for (unsigned int i = 0; i < m_vector_length; ++i)
			m_buffer[i] = 0.0;
	}

	std::set<double> m_signals;
	osmosdr::source::sptr m_source;
	float *m_buffer;
	unsigned int m_vector_length;
	unsigned int m_count;
	unsigned int m_wait_count;
	unsigned int m_avg_size;
	double m_step;
	double m_start_freq;
	double m_current_freq;
	double m_end_freq;
	double m_sps;
	time_t m_start_time;
	int m_gain_mode; //check whether gain was turned off already
	int m_use_AGC;
	double m_default_gain; //BB gain in dBm
	double agc_power_level;
	double agc_threshold_low;
	double agc_threshold_high;
	double current_gain_RF;
	double current_gain_IF;
	double rf_gain_mod;
	
	uint8_t *shared_memory; //memory shared with external monitor
	double last_log_out;
	int gain_change_timeout;
};

/* Shared pointer thing gnuradio is fond of */
typedef boost::shared_ptr<scanner_sink> scanner_sink_sptr;
scanner_sink_sptr make_scanner_sink(osmosdr::source::sptr source, unsigned int vector_length, double start_freq, double end_freq, double samples_per_second,double step, unsigned int avg_size, double def_gain, int use_AGC)
{
	return boost::shared_ptr<scanner_sink>(new scanner_sink(source, vector_length, start_freq, end_freq, samples_per_second, step, avg_size, def_gain, use_AGC));
}
