#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class DbMock {
public:
	struct RequestProcessRecord {
		std::string requestId;
		int8_t stepIdx{-1};
		std::string parameters;
	};
	void setProcessData(const std::string& requestId, int8_t stepIdx, const std::string& parameters);
	RequestProcessRecord fetchProcessData(const std::string& requestId);

	int fetchTransactionAmount(int transactionId);
	std::string fetchBeneficiaryName(int transactionId);

	struct TransactionRecord {
		int transactionId{0};
		std::string requestId;
		int amount{0};
		std::string beneficiaryAccount;
		std::string beneficiaryName;
		std::string remoteId;
	};
	TransactionRecord fetchTransactionRecord(int transactionId);
	std::string fetchConsumerName(int consumerId);
	int createTransaction(
		const std::string& requestId,
		int amount,
		const std::string& beneficiaryAccount,
		const std::string& beneficiaryName
	);
	void updateTransactionRecord(int transactionId, const std::string& remoteId);

	std::unordered_map<std::string, RequestProcessRecord> _processes;
	std::vector<TransactionRecord> _transactions;
	std::unordered_map<int, std::string> _consumers;
};
