#pragma once

#include "../parameters/initial_data.h"
#include "../parameters/transaction_data.h"

// Each step defines an interface that business logic needs to interact with other modules.
// This implements the Dependency Inversion principle - instead of depending on other modules,
// business logic defines an interface, on which these modules will depend.
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
