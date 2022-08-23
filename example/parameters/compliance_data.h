#pragma once

#include <string>

const int NOT_REQUIRED = 1;
const int REQUIRED = 2;
const int APPROVE = 3;
const int REJECT = 4;

struct ComplianceData {
	std::string requestId;
	int consumerId;
	int transactionId;
	int complianceDecision;

	ComplianceData() = default;
	explicit ComplianceData(const std::string& data);
	std::string serialize() const;
};
