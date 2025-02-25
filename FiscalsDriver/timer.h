#pragma once
#include <cstdint>
#include <chrono>
#include <sstream>
#include <iostream>

class Timer
{
public:
	Timer() : beg_(clock_::now()) {}
	void reset() {
		beg_ = clock_::now();
	}
	double elapsed() const
	{
		return std::chrono::duration_cast<second_>(clock_::now() - beg_).count();
	}
	bool timeout(size_t ms) const
	{
		return std::chrono::duration_cast<second_>(clock_::now() - beg_).count() >= ms;
	}
private:
	typedef std::chrono::high_resolution_clock clock_;
	typedef std::chrono::duration<double, std::ratio<1> > second_;
	std::chrono::time_point<clock_> beg_;
};

class TimerWait
{
public:
	TimerWait(uint64_t ms) {
		reset(ms);
	}
	void reset(uint64_t ms) {
		ms_timeout = ms;
		beg_ = clock_::now();
	}
	bool timeout() const
	{
		uint64_t diff = (uint64_t)std::chrono::duration_cast<ms_>(clock_::now() - beg_).count();
		return (diff >= ms_timeout);
	}

	uint64_t getDefault() {
		return ms_timeout;
	}
private:
	typedef std::chrono::high_resolution_clock clock_;
	typedef std::chrono::duration<double, std::ratio<1, 1000> >ms_;
	std::chrono::time_point<clock_> beg_;
	uint64_t ms_timeout;

};


template <typename T, const int64_t D>
class Timeout
{
public:
	Timeout() { ; }
	Timeout(T counter) {
		reset(counter);
	}
	void reset(T counter) {
		timeout_counter = counter;
		beg_ = clock_::now();
	}
	bool timeout() const {
		T diff = (T)std::chrono::duration_cast<duration_>(clock_::now() - beg_).count();
		return (diff >= timeout_counter);
	}
	T elapsed() const {
		return (T)(timeout_counter - std::chrono::duration_cast<duration_>(clock_::now() - beg_).count());
	}
	T getDefault() {
		return timeout_counter;
	}
private:
	typedef std::chrono::high_resolution_clock clock_;
	typedef std::chrono::duration<double, std::ratio<1, D> > duration_;
	std::chrono::time_point<clock_> beg_;
	T timeout_counter;
};

//template <typename TLog>
class TimerDebugProtocol : public Timer
{
public:
	TimerDebugProtocol(const char* _message, const char* _file, const char* _function, int _line) : Timer()
	{
		DebugLogger(_message,_file, _function, _line);
		setTab(1);
	}

	void DebugLogger(const char* _message, const char* _file, const char* _function, int _line) {
		//std::stringstream msg;msg << _function << ": " << _message;
		std::cerr << std::string(tab, '\t') << _function << ": " << _message << std::endl;
		message = _message;
		file = _file;
		function = _function;
		line = _line;
	}

	~TimerDebugProtocol()
	{
		std::stringstream _t_stop;
		_t_stop << function << ": STOP - " << std::fixed << elapsed() << " sek";
		setTab(-1);
		std::cerr << std::string(tab, '\t') << _t_stop.str() << std::endl;
	}
private:
	size_t line;
	const char* file;
	const char* function;
	const char* message;
	unsigned short tab = 0;
	void setTab(unsigned short _tab) {
		tab += _tab;
	}
};