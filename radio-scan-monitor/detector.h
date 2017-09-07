

/* Detector analyses given window of signal power vs frequency in terms of
 * peak and average power levels calculated in three parts ("left", "central" and "right")
 * with adjustable window size for averaging. Detection is based on correspondance of 
 * relation of calculated levels to target value - so for different types of signals, 
 * different detectors can be defined, each with its own parameters.
 * */
typedef struct sSignalDetector
{
	char name[32]; //filled by user, optional
	float standard_min_frequency_MHz; //min frequency where this signal type should apper, lower frequencies will have decreased probability
	float standard_max_frequency_MHz; //max frequency where this signal type should apper, higher frequencies will have decreased probability

	float center_width_kHz; //width of central band in kHz
	float side_width_kHz; //width of "side" background level in kHz
	
	float get_min_freq()
	{
		return center_width_kHz/2 + side_width_kHz;
	};
	float get_max_freq()
	{
		return center_width_kHz/2 + side_width_kHz;
	};
	int get_window_width_points(float frequency_step_hz)
	{
		return (2*side_width_kHz + center_width_kHz)*1000.0 / frequency_step_hz;
	};
/*process_data returns value from 0 to 1 indicating how well given signal fits detector's profile AT THE CENTER of provided frequency range.
 * this function doesn't scan given interval, scanning must be implemented outside of detector
 * If detector can't be applied (provided frequency range is smaller than detector parameters)
 * then it will return -1
 * 
 * Central band power is sent into res_power parameter
 * Also this function tries to detect channel BW, resulting width is stored into res_bw parameter
 * */
	float apply_detector(float *power_array, float frequency_start_hz, float frequency_step_hz, float current_frequency_hz, float *res_power, float *res_bw, float *res_centroid)
	{
		int center_pos = (current_frequency_hz - frequency_start_hz) / frequency_step_hz;
		int center_width = center_width_kHz * 1000.0 / frequency_step_hz;
		int side_width = side_width_kHz * 1000.0 / frequency_step_hz;
		int left_begin = center_pos - center_width/2 - side_width;
		int right_begin = center_pos + center_width/2 + side_width;
		
		float left_level = 0;
		float right_level = 0;
		for(int x = 0; x < side_width; x++)
		{
			left_level += power_array[left_begin + x];
			right_level += power_array[right_begin + x];
		}
		left_level /= (float)side_width;
		right_level /= (float)side_width;

		float center_level = 0;
		for(int x = 0; x < center_width; x++)
		{
			center_level += power_array[center_pos - center_width/2 + x];
		}
		center_level /= (float)center_width;
		
		if(center_level < left_level || center_level < right_level)
		{
			*res_power = 0;
			*res_bw = 0.0001;
			return 0;
		}

		float left_signal = 0;
		float right_signal = 0;
		float left_z = 0.000001;
		float right_z = 0.000001;
		for(int x = 0; x < 1 + center_width/2; x++)
		{
			float lv = power_array[center_pos - x];
			float rv = power_array[center_pos + x];
			if(lv > center_level)
			{
				left_signal += lv;
				left_z++;
			}
			if(rv > center_level)
			{
				right_signal += rv;
				right_z++;
			}
		}
		
		left_signal /= left_z;
		right_signal /= right_z;
		
		if(left_signal < left_level*0.98) return 0;
		if(right_signal < right_level*0.98) return 0;
		
		int left_signal_start = center_pos;
		int right_signal_start = center_pos;
//		printf("detecting: center %g left %g (%g) right %g (%g)\n", current_frequency_hz, left_signal, left_level, right_signal, right_level);
		for(int x = 0; x < 1 + center_width/2; x++)
		{
			float lv = power_array[center_pos - x];
			float rv = power_array[center_pos + x];
//			printf("x %d lv %g rv %g\n", x, lv, rv);
			if(lv < 0.7*left_signal + 0.3*left_level && left_signal_start == center_pos)
				left_signal_start = center_pos - x;
			if(rv < 0.7*right_signal + 0.3*right_level && right_signal_start == center_pos)
				right_signal_start = center_pos + x;
		}
		if(left_signal_start == center_pos) left_signal_start = center_pos - center_width/2;
		if(right_signal_start == center_pos) right_signal_start = center_pos + center_width/2;

		double pow_integr = 0.00000001;
		double freq_power_integr = 0.0;
		double pa_power_int = 0;
		for(int px = left_signal_start; px < right_signal_start; px++)
		{
			pa_power_int += power_array[px];
			double cur_pw = pow(10.0, power_array[px] * 0.1);
			pow_integr += cur_pw;
			double cur_fr = frequency_start_hz + px * frequency_step_hz;
			freq_power_integr += cur_fr * power_array[px];
		}

		*res_power = 10.0 * log10(pow_integr);
		*res_bw = (right_signal_start - left_signal_start) * frequency_step_hz;
		*res_centroid = freq_power_integr / pa_power_int;
		
		float score = left_level / left_signal - 1.0 + right_level / right_signal - 1.0;
		return score;
	}
}sSignalDetector;

