#pragma once
#include <thread>
#include <future>
#include <queue>
#include <memory>
#include <condition_variable>

template<typename T>
class threadsafe_queue
{
private:
	mutable std::mutex mut; //Mutex musi byæ modyfikowalny
	std::queue<T> data_queue;
	std::condition_variable data_cond;
	std::atomic<unsigned short> is_break = 0;

public:
	threadsafe_queue()
	{
	}
	threadsafe_queue(threadsafe_queue const& other)
	{
		std::lock_guard<std::mutex> lk(other.mut);
		data_queue = other.data_queue;
	}

	void finish() {
		std::cout << "Break notify: count in list " << data_queue.size() << std::endl;
		is_break++;
		data_cond.notify_one();
	}

	void push(T new_value)
	{
		std::lock_guard<std::mutex> lk(mut);
		data_queue.push(new_value);
		data_cond.notify_one();
	}
	bool wait_and_pop(T& value) //, std::atomic<bool>& control_wait)
	{
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk, [&, this]
		{
			return (!data_queue.empty() || is_break.load() > 0);
		});
		if (is_break.load() > 0)
		{
			std::cout << "Przerwano watek kolejki" << std::endl;
			return false;
		}

		value = data_queue.front();
		data_queue.pop();
		return true;
	}
	std::shared_ptr<T> wait_and_pop()
	{
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk, [&, this]
		{
			return (!data_queue.empty() || is_break.load() > 0);
		});
		if (is_break.load() > 0)
		{
			std::cout << "Przerwano w¹tek kolejki" << std::endl;
			return std::shared_ptr<T>();
		}
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
		data_queue.pop();
		return res;
	}
	bool try_pop(T& value)
	{
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty())
			return false;

		value = data_queue.front();
		data_queue.pop();
		return true;
	}
	std::shared_ptr<T> try_pop()
	{
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty())
			return std::shared_ptr<T>();
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
		data_queue.pop();
		return res;
	}
	bool empty() const
	{
		std::lock_guard<std::mutex> lk(mut);
		return data_queue.empty();
	}
};

template <typename...> struct Typelist;
class JoiningThread
{
	std::thread thd_;

public:
	// perfect forwarding constructor
	template <typename... Args, typename = std::enable_if_t<!std::is_same<Typelist<JoiningThread>,
		Typelist<std::decay_t<Args>...>>::value>>
		JoiningThread(Args&&... args) : thd_{ std::forward<Args>(args)... }
	{
	}

	JoiningThread(const JoiningThread&) = delete;
	JoiningThread& operator=(const JoiningThread&) = delete;

	JoiningThread(JoiningThread&&) = default;
	JoiningThread& operator=(JoiningThread&&) = default;

	~JoiningThread()
	{
		if (thd_.joinable())
			thd_.join();
	}

	std::thread& get()
	{
		return thd_;
	}
};
