#include "api_mock.h"

bool ApiMock::loadConsumerAccount(int consumerId, int amount) {
	_balance[consumerId] += amount;
	return true;
}

bool ApiMock::passScreening(const std::string& screeningData) {
	if (_sanctions.find(screeningData) != _sanctions.end()) {
		return false;
	}
	return true;
}

bool ApiMock::unloadConsumerAccount(int consumerId, int amount) {
	_balance[consumerId] -= amount;
	return true;
}

std::optional<std::string> ApiMock::initiateTransfer(
	const std::string& senderName,
	int amount,
	const std::string& beneficiaryAccount,
	const std::string& beneficiaryName
) {
	if (_errors.find(beneficiaryAccount) != _errors.end()) {
		return std::nullopt;
	}
	_transfers.push_back(
		senderName + " -> " + beneficiaryAccount + " (" + beneficiaryName + ") " + std::to_string(amount));
	return std::to_string(_transfers.size() + 1000);
}
