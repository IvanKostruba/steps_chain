#include "sanctions_screening.h"

ComplianceData sanctionsScreening(TransactionData data, SanctionsScreeningCtxI& ctx) {
	const auto name = ctx.beneficiaryName(data.transactionId);
	const bool passed = ctx.passScreening(name);
	ComplianceData result;
	result.complianceDecision = passed ? NOT_REQUIRED : REQUIRED;
	result.consumerId = data.consumerId;
	result.requestId = data.requestId;
	result.transactionId = data.transactionId;
	return result;
}
