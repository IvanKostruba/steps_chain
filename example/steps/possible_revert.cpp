#include "possible_revert.h"

#include <stdexcept>

std::optional<TransactionData> possibleRevert(ComplianceData data, PossibleRevertCtxI& ctx) {
	if (data.complianceDecision == REQUIRED) {
		// We have to wait for external input, so stop process on this step.
		return std::nullopt;
	}
	if (data.complianceDecision == REJECT) {
		ctx.loadAccount(data.consumerId, ctx.transactionAmount(data.transactionId));
		throw std::runtime_error{ "Transaction rejected by Compliance." };
	}
	// decision is APPROVE or NOT_REQUIRED
	TransactionData result;
	result.consumerId = data.consumerId;
	result.requestId = data.requestId;
	result.transactionId = data.transactionId;
	return result;
}
