#include "Auth.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace {
std::string MakeSalt(const std::string& username) {
    const long long stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::ostringstream output;
    output << std::hex << stamp << username.size() << (stamp ^ (stamp >> 17));
    return output.str();
}

std::string HashPassword(const std::string& salt, const std::string& password) {
    // Hash salted dùng cho bài demo; hệ thống thực tế nên thay bằng Argon2/bcrypt/PBKDF2.
    const std::string input = salt + ":" + password;
    unsigned long long hash = 14695981039346656037ULL;
    for (std::size_t i = 0; i < input.size(); ++i) {
        hash ^= static_cast<unsigned char>(input[i]);
        hash *= 1099511628211ULL;
    }
    std::ostringstream output;
    output << std::hex << std::setw(16) << std::setfill('0') << hash;
    return output.str();
}

bool ContainsComma(const std::string& value) { return value.find(',') != std::string::npos; }
}

void DestroyAccounts(AccountList& accounts) {
    while (accounts.head) {
        Account* item = accounts.head;
        accounts.head = item->next;
        delete item;
    }
    accounts.tail = 0;
}

Account* FindAccount(const AccountList& accounts, const std::string& username) {
    for (Account* item = accounts.head; item; item = item->next)
        if (item->username == username) return item;
    return 0;
}

bool HasAdminAccount(const AccountList& accounts) {
    for (Account* item = accounts.head; item; item = item->next)
        if (item->active && item->role == 'A') return true;
    return false;
}

Account* Authenticate(AccountList& accounts, const std::string& username, const std::string& password) {
    Account* account = FindAccount(accounts, username);
    if (!account || !account->active || HashPassword(account->salt, password) != account->passwordHash) return 0;
    return account;
}

bool CreateAccount(AccountList& accounts, const std::string& username, const std::string& email,
                   const std::string& password, int memberId, char role) {
    if (username.empty() || password.size() < 6 || memberId < 0 ||
        (role != 'A' && role != 'U') || ContainsComma(username) || ContainsComma(email) ||
        FindAccount(accounts, username)) return false;
    for (Account* item = accounts.head; item; item = item->next)
        if (memberId != 0 && item->active && item->memberId == memberId) return false;

    Account* account = new Account();
    account->username = username;
    account->email = email;
    account->salt = MakeSalt(username);
    account->passwordHash = HashPassword(account->salt, password);
    account->memberId = memberId;
    account->role = role;
    if (!accounts.head) accounts.head = account;
    else accounts.tail->next = account;
    accounts.tail = account;
    return true;
}

bool DeactivateAccount(AccountList& accounts, const std::string& username) {
    Account* account = FindAccount(accounts, username);
    if (!account || account->role == 'A') return false;
    account->active = false;
    return true;
}

bool LoadAccounts(const std::string& path, AccountList& accounts) {
    std::ifstream input(path.c_str());
    if (!input.is_open()) {
        std::ofstream createFile(path.c_str(), std::ios::binary);
        if (!createFile.is_open()) return false;
        createFile.write("\xEF\xBB\xBF", 3);
        createFile << "username,email,salt,passwordHash,memberId,role,active\n";
        return true;
    }
    std::string line;
    std::getline(input, line); // header
    while (std::getline(input, line)) {
        if (line.empty()) continue;
        std::istringstream row(line);
        std::string fields[7];
        bool valid = true;
        for (int i = 0; i < 7; ++i) {
            if (!std::getline(row, fields[i], ',')) { valid = false; break; }
        }
        if (!valid || fields[0].empty() || fields[5].size() != 1) continue;
        std::istringstream idInput(fields[4]); int memberId = 0;
        if (!(idInput >> memberId) || !idInput.eof()) continue;
        Account* account = new Account();
        account->username = fields[0]; account->email = fields[1]; account->salt = fields[2];
        account->passwordHash = fields[3]; account->memberId = memberId;
        account->role = fields[5][0]; account->active = fields[6] == "1";
        if (!accounts.head) accounts.head = account; else accounts.tail->next = account;
        accounts.tail = account;
    }
    return true;
}

bool SaveAccounts(const std::string& path, const AccountList& accounts) {
    std::ofstream output(path.c_str(), std::ios::binary);
    if (!output.is_open()) return false;
    output.write("\xEF\xBB\xBF", 3);
    output << "username,email,salt,passwordHash,memberId,role,active\n";
    for (Account* item = accounts.head; item; item = item->next)
        output << item->username << ',' << item->email << ',' << item->salt << ','
               << item->passwordHash << ',' << item->memberId << ',' << item->role << ','
               << (item->active ? 1 : 0) << '\n';
    return output.good();
}




