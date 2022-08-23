#include "context.h"

PayoutContext::PayoutContext(
	std::shared_ptr<ApiMock> api,
	std::shared_ptr<DbMock> db,
	std::shared_ptr<TimerMock> timer
) : _api{ api }, _db{ db }, _timer{ timer }
{}

bool PayoutContext::loadAccount(int consumerId, int amount) {
	return _api->loadConsumerAccount(consumerId, amount);
}

int PayoutContext::transactionAmount(int transactionId) {
	return _db->fetchTransactionAmount(transactionId);
}

std::string PayoutContext::beneficiaryName(int transactionId) {
	return _db->fetchBeneficiaryName(transactionId);
}

bool PayoutContext::passScreening(const std::string& screeningData) {
	return _api->passScreening(screeningData);
}

StartTransferCtxI::TransactionInfo PayoutContext::transactionInfo(int transactionId) {
	const auto record = _db->fetchTransactionRecord(transactionId);
	StartTransferCtxI::TransactionInfo result;
	result.amount = record.amount;
	result.beneficiaryAccount = record.beneficiaryAccount;
	result.beneficiaryName = record.beneficiaryName;
	result.remoteId = record.remoteId;
	return result;
}

std::string PayoutContext::consumerName(int consumerId) {
	return _db->fetchConsumerName(consumerId);
}

std::optional<std::string> PayoutContext::initiateTransfer(
	const std::string& senderName, const StartTransferCtxI::TransactionInfo& info)
{
	return _api->initiateTransfer(senderName, info.amount, info.beneficiaryAccount, info.beneficiaryName);
}

void PayoutContext::scheduleRetry(
	const std::string& requestId, int delay)
{
	_timer->setTimer(requestId, delay);
}

void PayoutContext::updateTransaction(
	int transactionId, const std::string& remoteId)
{
	_db->updateTransactionRecord(transactionId, remoteId);
}

int PayoutContext::createTransaction(
	const std::string& requestId,
	int amount,
	const std::string& beneficiaryAccount,
	const std::string& beneficiaryName
) {
	return _db->createTransaction(requestId, amount, beneficiaryAccount, beneficiaryName);
}

bool PayoutContext::unloadAccount(int consumerId, int amount) {
	return _api->unloadConsumerAccount(consumerId, amount);
}
