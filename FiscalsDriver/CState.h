#pragma once

#include <shared_mutex>
using lock_state = std::unique_lock<std::shared_mutex>;

template <typename T>
class CState
{
private:
	std::shared_mutex set_lock_state;
	T value;
	std::string description_value{""};
public:
	CState(T initValue) {
		lock_state lock(set_lock_state);
		value = initValue;
	};

	inline T set(T newSet) {
		lock_state lock(set_lock_state);
		const T last = value;
		value=newSet;
		return last;
	}	
	template <typename S>
	inline T set(T newSet, S _description) {
		lock_state lock(set_lock_state);
		const T last = value;
		value = newSet;
		description_value = _description;
		return last;
	}	
	inline void set(std::string& _description) {
		lock_state lock(set_lock_state);
		description_value = _description;
	}

	inline T get() {
		lock_state lock(set_lock_state);
		return value;
	}

	inline auto description() {
		lock_state lock(set_lock_state);
		return description_value;
	}

	const auto check(T compare) {
		lock_state lock(set_lock_state);
		return (value == compare);
	}
};

