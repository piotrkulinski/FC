#pragma once
#include "CState.h"
#include "FiscalStatus.h"

template <typename T, typename R>
class CStateManage {
	private:
		CState<T>* state;
	public:
		CStateManage(CState<T>& state_manage) {
			state = &state_manage;
			state->set(T::START,"Rozpoczêcie zadania");
		}		
		CStateManage(CState<T>& state_manage, CState<R>& result_manage) : CStateManage(state_manage) {
			result_manage.set(R::UNKNOW,"Nieokreœlony");
		}		
		~CStateManage() {
			state->set(T::FINISH);
		}
};