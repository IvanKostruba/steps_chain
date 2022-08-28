#pragma once

#include "../parameters/transaction_data.h"
#include "../parameters/compliance_data.h"

// Each step defines an interface that business logic needs to interact with other modules.
// This implements the Dependency Inversion principle - instead of depending on other modules,
// business logic defines an interface, on which these modules will depend.
class SanctionsScreeningCtxI {
public:
	virtual std::string beneficiaryName(int transactionId) = 0;
	virtual bool passScreening(const std::string& screeningData) = 0;
};

ComplianceData sanctionsScreening(TransactionData data, SanctionsScreeningCtxI& ctx);
