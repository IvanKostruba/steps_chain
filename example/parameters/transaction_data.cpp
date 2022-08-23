#include "transaction_data.h"

TransactionData::TransactionData(const std::string& data) {
	requestId = data.substr(0, 8);
	consumerId = std::stoi(data.substr(9, 4));
	transactionId = std::stoi(data.substr(14, 4));
}

std::string TransactionData::serialize() const {
	return requestId + " "
		+ std::to_string(consumerId) + " "
		+ std::to_string(transactionId);
}
