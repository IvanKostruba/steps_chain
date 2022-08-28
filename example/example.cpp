#include "payout_process.h"
#include "api/api_mock.h"
#include "db/db_mock.h"
#include "timer/timer_mock.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

void runProcess(
    steps_chain::ChainWrapper p,
    const std::string& requestId,
    std::shared_ptr<DbMock> db
) {
    const auto data = db->fetchProcessData(requestId);
    p.initialize(data.parameters, data.stepIdx);
    try {
        while (!p.is_finished()) {
            if (p.advance()) {
                const auto [idx, params] = p.get_current_state();
                db->setProcessData(data.requestId, idx, params);
            }
            else {
                std::cout << "Processing of request [" << data.requestId << "] interrupted.\n";
                return;
            }
        }
    }
    catch (const std::exception& ex) {
        std::cout << "Exception in processing of request [" << data.requestId << "]: "
            << ex.what() << "\n";
        return;
    }
    std::cout << "Processing of request [" << data.requestId
        << "] completed successfully.\n";
    const auto processData = db->fetchProcessData(requestId);
    std::cout << "Finished processing data: output |"
        << processData.parameters << "|\n";
}

int main(int argc, char **argv) {
    auto api = std::make_shared<ApiMock>();
    auto db = std::make_shared<DbMock>();
    auto timer = std::make_shared<TimerMock>();
    
    std::cout << "Positive case -- successful payout without problems.\n";
    db->_consumers[1001] = "Franz Ferdinand";
    api->_balance[1001] = 5000;
    //                                   req-ID   user  amt dest account  dest name
    const std::string incomingRequest_1{"ABCD-101 1001 3241 DE0243983278 Thereza Mustermann"};
    db->setProcessData("ABCD-101", 0, incomingRequest_1);
    runProcess(payoutProcess(api, db, timer), "ABCD-101", db);
    // Check that the balance is deduced correctly.
    assert(api->_balance[1001] == 5000 - 3241);

    std::cout << "\nNegative case -- balance is too low.\n";
    db->_consumers[1002] = "Ozzy Osborne";
    api->_balance[1002] = 2000;
    const std::string incomingRequest_2{ "NJDS-102 1002 2001 DE0734574568 Jeremy Soul" };
    db->setProcessData("NJDS-102", 0, incomingRequest_2);
    runProcess(payoutProcess(api, db, timer), "NJDS-102", db);
    // Make sure that process did not progress after an exception on the first step
    assert(db->_processes["NJDS-102"].stepIdx == 0);

    std::cout << "\nNegative case -- request is rejected.\n";
    db->_consumers[1003] = "Anonymous";
    api->_balance[1003] = 8000;
    api->_sanctions.insert("Pablo Escobar");
    const std::string incomingRequest_3{ "KLEN-103 1003 4000 ES0543987821 Pablo Escobar" };
    db->setProcessData("KLEN-103", 0, incomingRequest_3);
    runProcess(payoutProcess(api, db, timer), "KLEN-103", db);
    assert(db->_processes["KLEN-103"].stepIdx == 2);
    std::cout << "Update request data with rejection and continue processing.\n";
    const std::string updatedRequest_3{ "KLEN-103 1003 1002 4" };
    db->updateProcessData("KLEN-103", updatedRequest_3);
    runProcess(payoutProcess(api, db, timer), "KLEN-103", db);
    assert(db->_processes["KLEN-103"].stepIdx == 2);
    assert(db->_transactions.size() == 3);
    // Make sure the money is returned back as we reject the transaction.
    assert(api->_balance[1003] == 8000);

    std::cout << "\nPositive case -- retry on transient error.\n";
    db->_consumers[1004] = "Unlucky";
    api->_balance[1004] = 4331;
    api->_errors.insert("DE0203492344");
    const std::string incomingRequest_4{ "IJSA-104 1004 0331 DE0203492344 Elusive Joe" };
    db->setProcessData("IJSA-104", 0, incomingRequest_4);
    runProcess(payoutProcess(api, db, timer), "IJSA-104", db);
    assert(db->_processes["IJSA-104"].stepIdx == 3);
    // Make sure that retry timer is set
    assert(timer->_timers.find("IJSA-104") != timer->_timers.end());
    api->_errors.erase("DE0203492344");
    runProcess(payoutProcess(api, db, timer), "IJSA-104", db);
    return 0;
}
