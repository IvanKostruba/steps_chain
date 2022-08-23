#pragma once

#include "../parameters/compliance_data.h"
#include "../parameters/transaction_data.h"

#include <optional>

class PossibleRevertCtxI {
public:
	virtual bool loadAccount(int consumerId, int amount) = 0;
	virtual int transactionAmount(int transactionId) = 0;
};

std::optional<TransactionData> possibleRevert(ComplianceData data, PossibleRevertCtxI& ctx);
