#include "start_transfer.h"

std::optional<TransactionData> startTransfer(TransactionData data, StartTransferCtxI& ctx) {
	auto txnInfo = ctx.transactionInfo(data.transactionId);
	if (!txnInfo.remoteId.empty()) {
		// transfer is already initiated
		return data;
	}
	const auto senderName = ctx.consumerName(data.consumerId);
	const auto remoteId = ctx.initiateTransfer(senderName, txnInfo);
	if (!remoteId.has_value()) {
		ctx.scheduleRetry(data.requestId, 60);
		return std::nullopt;
	}
	txnInfo.remoteId = *remoteId;
	ctx.updateTransaction(data.transactionId, *remoteId);
	return data;
}
