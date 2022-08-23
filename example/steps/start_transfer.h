#pragma once

#include "../parameters/transaction_data.h"

#include <optional>

class StartTransferCtxI {
public:
	struct TransactionInfo {
		int amount{0};
		std::string beneficiaryAccount;
		std::string beneficiaryName;
		std::string remoteId;
	};

	virtual TransactionInfo transactionInfo(int transactionId) = 0;
	virtual std::string consumerName(int consumerId) = 0;
	virtual std::optional<std::string> initiateTransfer(
		const std::string& senderName, const TransactionInfo& info) = 0;
	virtual void scheduleRetry(const std::string& requestId, int delay) = 0;
	virtual void updateTransaction(int transactionId, const std::string& remoteId) = 0;
};

std::optional<TransactionData> startTransfer(TransactionData data, StartTransferCtxI& ctx);
