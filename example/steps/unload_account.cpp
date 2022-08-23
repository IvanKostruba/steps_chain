#include "unload_account.h"

#include <stdexcept>

TransactionData unloadAccount(const InitialData& data, UnloadAccountCtxI& ctx) {
	const int transactionId = ctx.createTransaction(
		data.requestId, data.amount, data.beneficiaryAccount, data.beneficiaryName);
	if (!ctx.unloadAccount(data.consumerId, data.amount)) {
		throw std::runtime_error{ "Balance is too low." };
	}
	TransactionData result;
	result.consumerId = data.consumerId;
	result.requestId = data.requestId;
	result.transactionId = transactionId;
	return result;
}
