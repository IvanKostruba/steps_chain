#pragma once

#include "../parameters/initial_data.h"
#include "../parameters/transaction_data.h"

class UnloadAccountCtxI {
public:
	virtual int createTransaction(
		const std::string& requestId,
		int amount,
		const std::string& beneficiaryAccount,
		const std::string& beneficiaryName
	) = 0;
	virtual bool unloadAccount(int consumerId, int amount) = 0;
};

TransactionData unloadAccount(const InitialData& data, UnloadAccountCtxI& ctx);
