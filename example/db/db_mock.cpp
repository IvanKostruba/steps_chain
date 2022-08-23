#include "db_mock.h"

void DbMock::setProcessData(
	const std::string& requestId, int8_t stepIdx, const std::string& parameters) {
	_processes[requestId] = RequestProcessRecord{ requestId, stepIdx, parameters };
}

DbMock::RequestProcessRecord DbMock::fetchProcessData(const std::string& requestId) {
	return _processes[requestId];
}

int DbMock::fetchTransactionAmount(int transactionId) {
	return _transactions[transactionId].amount;
}

std::string DbMock::fetchBeneficiaryName(int transactionId) {
	return _transactions[transactionId].beneficiaryName;
}

DbMock::TransactionRecord DbMock::fetchTransactionRecord(int transactionId) {
	return _transactions[transactionId];
}

std::string DbMock::fetchConsumerName(int consumerId) {
	return _consumers[consumerId];
}

int DbMock::createTransaction(
	const std::string& requestId,
	int amount,
	const std::string& beneficiaryAccount,
	const std::string& beneficiaryName
) {
	_transactions.push_back(TransactionRecord{ 
		static_cast<int>(_transactions.size()),
		requestId,
		amount,
		beneficiaryAccount,
		beneficiaryName,
		""
	});
	return _transactions.back().transactionId;
}

void DbMock::updateTransactionRecord(int transactionId, const std::string& remoteId) {
	_transactions[transactionId].remoteId = remoteId;
}
