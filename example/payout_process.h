#pragma once

#include <chain_wrapper.h>

#include "api/api_mock.h"
#include "db/db_mock.h"
#include "timer/timer_mock.h"

#include <memory>

steps_chain::ChainWrapper payoutProcess(
	std::shared_ptr<ApiMock> api,
	std::shared_ptr<DbMock> db,
	std::shared_ptr<TimerMock> timer
);
