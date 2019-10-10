```cpp
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <ctime>

class TransBlock {
public:
    int src_id;
    int dest_id;
    int amount;
    std::time_t timestamp;
    bool failure;

    TransBlock(int sid, int did, unsigned int amount) {
        this->src_id = sid;
        this->dest_id = did;
        this->amount = amount;
        this->timestamp = std::time(nullptr);
        failure = false;
    }
};

class Account {
public:
    int id;
    unsigned int balance;
    std::vector<TransBlock*> transactions;
    std::mutex m;

    Account(int id, unsigned int balance) {
        this->id = id;
        this->balance = balance;
    }

    void send(unsigned int amount, int other_account_id) {
        transactions.push_back(new TransBlock(id, other_account_id, -amount));
    }

    void received(unsigned int amount, int other_account_id) {
        transactions.push_back(new TransBlock(other_account_id, id, amount));
    }

    void drop() {
        this->transactions.back()->failure = true;
    }

    TransBlock* last_transaction() {
        return this->transactions.back();
    }
};

bool transfer(Account* src, Account* dest, unsigned int amount, int identifier) {
//    std::cout << "Transfer on thread id: " << identifier << ", between: " << src->id << " and " << dest->id << " of: " << amount << std::endl;
    int success, threshold = 10;
    std::chrono::milliseconds timespan(rand() % 10000 + 1);
    do {
        threshold--;
        src->m.lock();
        if(src->balance >= amount) {
            src->balance -= amount;
            src->send(amount, dest->id);
        }
        success = dest->m.try_lock();
        if(success) {
            dest->balance += amount;
            dest->received(amount, src->id);
            // it would be nice to see here the history track
            dest->m.unlock();
            src->m.unlock();
            return true;
        } else {
            src->balance += amount;
            src->drop();
            src->m.unlock();
            std::this_thread::sleep_for(timespan);
        }
    } while (success and threshold);

    return false;
}

int main()
{
    /* initialize random seed: */
    srand (time(nullptr));

    // generate the list of accounts and threads
    std::vector<Account*> accounts;
    std::vector<std::thread*> threads;

    for(int iAcc=0; iAcc<10; iAcc++) {
        auto a = new Account(iAcc, rand() % 100 + 1);
        accounts.push_back(a);
        std::cout << "Account " << accounts.back()->id << ": " << accounts.back()->balance << "$" << std::endl;
    }
    std::cout << "\nAfter one run:\n";

    for(int iThrd=0; iThrd<=100; iThrd++) {
        int i = 0, j = 0;
        while(i == j) {
            i = rand() % 10;
            j = rand() % 10;
        }
        if(i < 0) i = -i;
        if(j < 0) j = -j;
        auto* t = new std::thread(transfer, accounts[i], accounts[j], 10, iThrd);
        threads.push_back(t);
    }

    // TO DO:
    // - insert some random timestamps at which you check the balance and the correctness of the balance on both sides
    // - wait a little bit to start some threads then query the balance at random moments by also validating the history

    for(int iThrd=0; iThrd<=100; iThrd++) {
        threads[iThrd]->join();
    }

    for(int iAcc=0; iAcc<10; iAcc++) {
        std::cout << "Account " << accounts[iAcc]->id << ": " << accounts[iAcc]->balance << "$" << std::endl;
        for (auto &transaction : accounts[iAcc]->transactions) {
            std::cout << "\t";
            if (transaction->failure) {
                std::cout << "failed to ";
            }
            if (transaction->amount < 0) {
                std::cout << "sent " << -transaction->amount << " to " << transaction->dest_id << std::endl;
            } else {
                std::cout << "received " << transaction->amount << " from " << transaction->dest_id
                          << std::endl;
            }
        }
        std::cout << std::endl;
    }

    return 0;
}
```
