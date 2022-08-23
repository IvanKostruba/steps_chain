#pragma once

#include "../parameters/transaction_data.h"
#include "../parameters/compliance_data.h"

class SanctionsScreeningCtxI {
public:
	virtual std::string beneficiaryName(int transactionId) = 0;
	virtual bool passScreening(const std::string& screeningData) = 0;
};

ComplianceData sanctionsScreening(TransactionData data, SanctionsScreeningCtxI& ctx);
