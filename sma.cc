/*
sma - simple moving average
Written in 2015 by <Ahmet Inan> <xdsopl@googlemail.com>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <chrono>
#include <iostream>
#include <random>
#include <array>
#include <numeric>
#include "kahan.hh"
#include "pairwise.hh"

std::random_device rd;
std::default_random_engine generator(rd());
std::normal_distribution<float> distribution(0.0f, 1.0f);
auto myrand = std::bind(distribution, generator);

template <typename T, int N>
class SMA1
{
	std::vector<T> history;
	int position;
public:
	SMA1() : history(N), position(0) {}
	T operator ()(T v)
	{
		history[position] = v;
		position = (position + 1) % N;
#if 0
		return std::accumulate(history.begin(), history.end(), T(0)) / T(N);
#else
		return kahan_sum(history.begin(), history.end(), T(0)) / T(N);
#endif
	}
};

template <typename T, int N>
class SMA2
{
	std::vector<T> history;
	int position;
	T sum;
public:
	SMA2() : history(N), position(0), sum(0) {}
	T operator ()(T v)
	{
		sum += v - history[position];
		history[position] = v;
		position = (position + 1) % N;
		return sum / T(N);
	}
};

template <typename T, int N>
class SMA3
{
	std::vector<T> history;
	int position;
	Kahan<T> sum;
public:
	SMA3() : history(N), position(0), sum(0) {}
	T operator ()(T v)
	{
		sum(-history[position]);
		history[position] = v;
		position = (position + 1) % N;
		return sum(v) / T(N);
	}
};

template <typename T, int N>
class SMA4
{
	struct add {
		T operator () (T l, T r) { return l + r; }
	};
	Pairwise<T, N, add> history;
	int position;
public:
	SMA4() : position(0) {}
	T operator ()(T v)
	{
		history[position] = v;
		position = (position + 1) % N;
		return history.reduce() / T(N);
	}
};

template <typename SMA, typename T>
int benchmark(T &output, T &input)
{
	SMA sma;
	auto start = std::chrono::system_clock::now();
	for (size_t i = 0; i < input.size(); ++i)
		output[i] = sma(input[i]);
	auto end = std::chrono::system_clock::now();
	auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	return msec.count();
}

template <typename T>
typename T::value_type compare(T &a, T &b)
{
	typename T::value_type max_error(0);
	for (size_t i = 0; i < a.size(); ++i)
		max_error = std::max(max_error, std::abs(a[i] - b[i]));
	return max_error;
}

int main()
{
	const int num_samples = 10000000;
	std::vector<float> input(num_samples), output1(num_samples), output2(num_samples), output3(num_samples), output4(num_samples);
	const int window_length = 500;
	for (size_t i = 0; i < input.size(); ++i)
		input[i] = sin(i * 4.0f * 2.0f * M_PI / num_samples) + myrand();
	{
		int msec = benchmark<SMA1<float, window_length>>(output1, input);
		std::cerr << "sma1: " << msec << " milliseconds." << std::endl;
	}{
		int msec = benchmark<SMA2<float, window_length>>(output2, input);
		float max_error = compare(output1, output2);
		std::cerr << "sma2: " << msec << " milliseconds. max absolute error: " << max_error << std::endl;
	}{
		int msec = benchmark<SMA3<float, window_length>>(output3, input);
		float max_error = compare(output1, output3);
		std::cerr << "sma3: " << msec << " milliseconds. max absolute error: " << max_error << std::endl;
	}{
		int msec = benchmark<SMA4<float, window_length>>(output4, input);
		float max_error = compare(output1, output4);
		std::cerr << "sma4: " << msec << " milliseconds. max absolute error: " << max_error << std::endl;
	}
#if 1
	for (size_t i = 0; i < input.size(); ++i)
		std::cout << input[i] << " " << output1[i] << " " << output2[i] << " " << output3[i] << " " << output4[i] << std::endl;
#endif
}

