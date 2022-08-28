#include "compliance_data.h"

ComplianceData::ComplianceData(const std::string& data) {
	requestId = data.substr(0, 8);
	consumerId = std::stoi(data.substr(9, 4));
	transactionId = std::stoi(data.substr(14, 4)) - 1000;
	complianceDecision = std::stoi(data.substr(19));
}

std::string ComplianceData::serialize() const {
	return requestId + " "
		+ std::to_string(consumerId) + " "
		+ std::to_string(transactionId + 1000) + " "
		+ std::to_string(complianceDecision);
}
