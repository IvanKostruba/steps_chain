#pragma once

#include <string>

struct TransactionData {
	std::string requestId;
	int consumerId;
	int transactionId;

	TransactionData() = default;
	explicit TransactionData(const std::string& data);
	std::string serialize() const;
};
