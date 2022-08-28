#pragma once

#include "../parameters/compliance_data.h"
#include "../parameters/transaction_data.h"

#include <optional>

// Each step defines an interface that business logic needs to interact with other modules.
// This implements the Dependency Inversion principle - instead of depending on other modules,
// business logic defines an interface, on which these modules will depend.
class PossibleRevertCtxI {
public:
	virtual bool loadAccount(int consumerId, int amount) = 0;
	virtual int transactionAmount(int transactionId) = 0;
};

std::optional<TransactionData> possibleRevert(ComplianceData data, PossibleRevertCtxI& ctx);
