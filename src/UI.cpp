#include "UI.h"
#include "TreeCore.h"
#include "TreeAlgorithms.h"
#include "FileManager.h"
#include <cstdlib>
#include <iostream>
#include <limits>
#include <iomanip>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#endif

namespace {
std::string ReadLine(const std::string& prompt) {
    std::string value; std::cout << prompt;
    if (!std::getline(std::cin, value)) throw std::runtime_error("Da het du lieu nhap.");
    return value;
}
int ReadInt(const std::string& prompt) {
    while (true) {
        std::string value = ReadLine(prompt); char* end = 0; long number = std::strtol(value.c_str(), &end, 10);
        if (end != value.c_str() && *end == '\0' && number >= -2147483647L && number <= 2147483647L) return static_cast<int>(number);
        std::cout << "Gia tri khong hop le, vui long nhap so nguyen.\n";
    }
}
Member ReadMember(bool readId) {
    Member member;
    if (readId) member.id = ReadInt("ID (so nguyen duong): ");
    do {
        member.name = ReadLine("Ho ten (bat buoc): ");
        if (member.name.find_first_not_of(" \t\r\n") == std::string::npos)
            std::cout << "Ho ten khong duoc de trong.\n";
    } while (member.name.find_first_not_of(" \t\r\n") == std::string::npos);
    while (true) {
        std::string gender = ReadLine("Gioi tinh (M/F/O): ");
        if (gender.size() == 1 && (gender[0] == 'M' || gender[0] == 'F' || gender[0] == 'O')) { member.gender = gender[0]; break; }
        std::cout << "Chi chap nhan M, F hoac O.\n";
    }
    do { member.birthDate = ReadLine("Ngay sinh (DD/MM/YYYY): "); if (!IsValidDate(member.birthDate)) std::cout << "Ngay sinh khong hop le.\n"; } while (!IsValidDate(member.birthDate));
    member.deathDate = ReadLine("Ngay mat (DD/MM/YYYY, bo trong neu chua ro/con song): ");
    while (!member.deathDate.empty() && (!IsValidDate(member.deathDate) || CompareDates(member.deathDate, member.birthDate) < 0)) {
        std::cout << "Ngay mat phai hop le va khong som hon ngay sinh.\n";
        member.deathDate = ReadLine("Ngay mat (DD/MM/YYYY): ");
    }
    member.hometown = ReadLine("Que quan: "); member.biography = ReadLine("Tieu su: ");
    member.photoPath = ReadLine("Duong dan anh (tuy chon): "); member.notes = ReadLine("Ghi chu: ");
    return member;
}
void ShowMember(const Member& m) {
    std::cout << "\n+--------------------- HO SO THANH VIEN ---------------------+\n"
              << "ID: " << m.id << " | Ho ten: " << m.name << " | Gioi tinh: " << m.gender
              << " | Sinh: " << m.birthDate << " | Mat: " << (m.deathDate.empty() ? "--" : m.deathDate)
              << "\nQue quan: " << (m.hometown.empty() ? "--" : m.hometown)
              << "\nVo/chong ID: " << m.spouseId << " | Ket hon vao dong ho: " << (m.isInLaw ? "Co" : "Khong")
              << "\nTieu su: " << (m.biography.empty() ? "--" : m.biography)
              << "\nAnh: " << (m.photoPath.empty() ? "--" : m.photoPath)
              << "\nGhi chu: " << (m.notes.empty() ? "--" : m.notes) << '\n';
}
bool Login(AccountList& accounts, Account*& current) {
    for (;;) {
        std::cout << "\n\x1B[38;2;132;73;95m+==============================================================+\n"
                  << "|                 CHAO MUNG DEN VOI GIA PHA                   |\n"
                  << "+==============================================================+\x1B[0m\n"
                  << "  1. Dang nhap\n  2. Dang ky tai khoan\n  0. Thoat\n";
        int action = ReadInt("Lua chon: ");
        if (action == 0) return true;
        if (action == 2) {
            std::string username = ReadLine("Ten dang nhap: ");
            std::string email = ReadLine("Email: ");
            std::string password = ReadLine("Mat khau (toi thieu 6 ky tu): ");
            std::string confirm = ReadLine("Nhap lai mat khau: ");
            const char role = HasAdminAccount(accounts) ? 'U' : 'A';
            if (password != confirm) {
                std::cout << "Mat khau khong khop. Vui long dang ky lai.\n";
            } else if (!CreateAccount(accounts, username, email, password, 0, role)) {
                std::cout << "Dang ky that bai: ten tai khoan co the da duoc su dung hoac du lieu khong hop le.\n";
            } else if (!SaveAccounts("data/accounts.csv", accounts)) {
                std::cout << "Da tao tai khoan trong phien nay nhung khong luu duoc accounts.csv.\n";
            } else {
                std::cout << (role == 'A' ? "Da tao Admin dau tien. " : "Da tao tai khoan User. ")
                          << "Hay chon Dang nhap de tiep tuc.\n";
            }
            continue;
        }
        if (action != 1) {
            std::cout << "Lua chon khong hop le.\n";
            continue;
        }
        for (int attempt = 0; attempt < 5; ++attempt) {
            std::cout << "\n=== DANG NHAP GIA PHA ===\n";
            std::string username = ReadLine("Tai khoan: "); std::string password = ReadLine("Mat khau: ");
            current = Authenticate(accounts, username, password);
            if (current) { std::cout << "Xin chao " << current->username << " (" << (current->role == 'A' ? "ADMIN" : "USER") << ").\n"; return true; }
            std::cout << "Sai thong tin hoac tai khoan da bi khoa. Con " << (4 - attempt) << " lan thu.\n";
        }
        std::cout << "\nQuay lai man hinh dang nhap.\n";
    }
}
void AddMenu(FamilyTree& tree) {
    std::cout << "1. Them goc dong ho\n2. Them con\n3. Them vo/chong\n";
    int choice = ReadInt("Lua chon: "); Member member = ReadMember(true); bool success = false;
    if (choice == 1) success = AddRoot(tree, member);
    else if (choice == 2) success = AddChild(tree, ReadInt("ID cha/me: "), member);
    else if (choice == 3) {
        int id = ReadInt("ID thanh vien: "); member.isInLaw = true; success = AddSpouse(tree, id, member);
    } else { std::cout << "Lua chon khong hop le.\n"; return; }
    std::cout << (success ? "Da them thanh vien.\n" : "That bai: ID trung/khong hop le hoac ngay sinh con som hon cha/me.\n");
}
void EditMenu(FamilyTree& tree) {
    int id = ReadInt("ID can sua: "); if (!FindMember(tree.root, id)) { std::cout << "Khong tim thay.\n"; return; }
    Member updated = ReadMember(false);
    std::cout << (UpdateMember(tree, id, updated) ? "Da cap nhat.\n" : "Cap nhat that bai; kiem tra ngay sinh voi cha/me va cac con.\n");
}
void SearchMenu(FamilyTree& tree) {
    int id = ReadInt("ID can tim: "); Node* node = FindByDfs(tree.root, id);
    if (!node) { std::cout << "Khong tim thay.\n"; return; } ShowMember(node->member);
}
void SearchNameMenu(FamilyTree& tree) {
    std::string query = ReadLine("Nhap chuoi ten can tim: "); int count = 0;
    SearchByNameContains(tree.root, query, std::cout, count);
    if (!count) std::cout << "Khong co ket qua phu hop.\n";
    else std::cout << "Tim thay " << count << " thanh vien.\n";
}
void RelationshipMenu(FamilyTree& tree) {
    int first = ReadInt("ID nguoi thu nhat: "), second = ReadInt("ID nguoi thu hai: ");
    Node* ancestor = FindLowestCommonAncestor(tree.root, first, second); int distance = GetRelationshipDistance(tree.root, first, second);
    if (!ancestor || distance < 0) std::cout << "Khong tim thay du ca hai thanh vien trong cung nhanh.\n";
    else std::cout << "To tien chung gan nhat: " << ancestor->member.name << " (ID " << ancestor->member.id
                   << ")\nKhoang cach: " << distance << " canh cha-con.\n";
}
void StatisticsMenu(FamilyTree& tree, AchievementList& achievements) {
    FamilyStats stats; CollectStatistics(tree.root, stats);
    std::cout << "Tong thanh vien: " << stats.totalMembers << "\nNam: " << stats.maleCount
              << "\nNu: " << stats.femaleCount << "\nKhac: " << stats.otherCount
              << "\nSo the he: " << stats.generationCount << '\n';
    int year = ReadInt("Nam thong ke (0 bo qua): ");
    if (year > 0) {
        int births = 0, deaths = 0, awards = 0; CountYear(tree.root, year, births, deaths);
        for (Achievement* item = achievements.head; item; item = item->next) if (item->year == year) ++awards;
        std::cout << "Nam " << year << ": sinh " << births << ", mat " << deaths << ", thanh tich " << awards << ".\n";
    }
}
void AccountMenu(AccountList& accounts, FamilyTree& tree) {
    std::cout << "1. Tao tai khoan nguoi xem\n2. Khoa tai khoan\n3. Liet ke tai khoan\n";
    int choice = ReadInt("Lua chon: ");
    if (choice == 1) {
        std::string username = ReadLine("Ten dang nhap: "), email = ReadLine("Email: ");
        std::string password = ReadLine("Mat khau (toi thieu 6 ky tu): "); int memberId = ReadInt("ID thanh vien lien ket (0 neu khong): ");
        if (memberId && !FindMember(tree.root, memberId)) { std::cout << "ID thanh vien khong ton tai.\n"; return; }
        std::cout << (CreateAccount(accounts, username, email, password, memberId, 'U') ? "Da tao tai khoan.\n" : "Tao that bai; ten/ID co the da duoc dung.\n");
    } else if (choice == 2) {
        std::string username = ReadLine("Tai khoan can khoa: ");
        std::cout << (DeactivateAccount(accounts, username) ? "Da khoa tai khoan.\n" : "Khong the khoa (khong co tai khoan hoac la admin).\n");
    } else if (choice == 3) {
        for (Account* a = accounts.head; a; a = a->next)
            std::cout << std::left << std::setw(20) << a->username << " | " << (a->role == 'A' ? "ADMIN" : "USER")
                      << " | member=" << a->memberId << " | " << (a->active ? "active" : "locked") << '\n';
    }
}
void AchievementMenu(AchievementList& list, FamilyTree& tree) {
    std::cout << "1. Them thanh tich\n2. Xem thanh tich theo thanh vien\n3. Xoa thanh tich\n";
    int choice = ReadInt("Lua chon: ");
    if (choice == 1) {
        int memberId = ReadInt("ID thanh vien: "); if (!FindMember(tree.root, memberId)) { std::cout << "ID khong ton tai.\n"; return; }
        int year = ReadInt("Nam: "); std::string title = ReadLine("Thanh tich: "), org = ReadLine("To chuc: "), desc = ReadLine("Mo ta: ");
        std::cout << (AddAchievement(list, memberId, year, title, org, desc) ? "Da them.\n" : "Du lieu khong hop le.\n");
    } else if (choice == 2) PrintAchievements(list, ReadInt("ID thanh vien: "), std::cout);
    else if (choice == 3) std::cout << (RemoveAchievement(list, ReadInt("ID thanh tich: ")) ? "Da xoa.\n" : "Khong tim thay.\n");
}
void RuleMenu(RuleList& rules, bool admin) {
    if (!admin) { PrintRules(rules, std::cout, false); return; }
    int choice = ReadInt("1. Xem quy uoc  2. Them quy uoc  3. Ngung ap dung\nLua chon: ");
    if (choice == 2) {
        std::string title = ReadLine("Tieu de: "), content = ReadLine("Noi dung: "), date = ReadLine("Ngay hieu luc DD/MM/YYYY: ");
        std::cout << (AddRule(rules, title, content, date) ? "Da them quy uoc.\n" : "Du lieu khong hop le.\n");
    } else if (choice == 3) {
        PrintRules(rules, std::cout, true);
        std::cout << (DeactivateRule(rules, ReadInt("ID quy uoc: ")) ? "Da ngung ap dung.\n" : "Khong tim thay quy uoc dang ap dung.\n");
    } else PrintRules(rules, std::cout, true);
}
void SaveAll(const std::string& familyPath, FamilyTree& tree, AccountList& accounts,
             AchievementList& achievements, RuleList& rules) {
    bool ok = SaveToCsv(familyPath, tree) && SaveAccounts("data/accounts.csv", accounts) &&
              SaveAchievements("data/achievements.csv", achievements) && SaveRules("data/family_rules.csv", rules);
    std::cout << (ok ? "Da luu toan bo du lieu.\n" : "Luu that bai; kiem tra quyen ghi file.\n");
}
}

void PrintFamilyTree(Node* root) {
    if (!root) { std::cout << "(Cay gia pha dang rong)\n"; return; }
    PrintDfs(root, std::cout, 0);
}

void RunMenu(FamilyTree& tree, const std::string& familyPath, AccountList& accounts,
             AchievementList& achievements, RuleList& rules) {
#ifdef _WIN32
    // File CSV dung UTF-8; dat code page console UTF-8 de tranh loi mojibake tieng Viet.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE); DWORD mode = 0;
    if (handle != INVALID_HANDLE_VALUE && GetConsoleMode(handle, &mode)) SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
    Account* current = 0;
    if (!Login(accounts, current)) { std::cout << "Dang nhap that bai.\n"; return; }
    if (!current) return;
    int choice;
    do {
        FamilyStats stats; CollectStatistics(tree.root, stats); const bool admin = current->role == 'A';
        std::cout << "\n\x1B[38;2;132;73;95m  +================================================================+\n"
                  << "  |                    GIA PHA GIA DINH                             |\n"
                  << "  |            Giu gin ket noi qua cac the he                        |\n"
                  << "  +================================================================+\x1B[0m\n"
                  << "  \x1B[38;2;254;168;55mTHONG TIN\x1B[0m  Thanh vien: " << stats.totalMembers
                  << "  |  The he: " << stats.generationCount
                  << "  |  Tai khoan: \x1B[38;2;184;91;86m" << current->username
                  << (admin ? " [ADMIN]\x1B[0m\n" : " [USER]\x1B[0m\n")
                  << "  \x1B[38;2;222;116;28mTRA CUU\x1B[0m\n"
                  << "  01  In so do gia pha             02  Liet ke theo the he (BFS)\n"
                  << "  03  Tim theo ID                  04  Tim ten gan dung\n"
                  << "  05  Xem thanh tich               06  Xem quy uoc gia toc\n"
                  << "  07  Tim to tien / khoang cach    08  Thong ke theo nam\n";
        if (admin) std::cout << "  \x1B[38;2;222;116;28mQUAN TRI\x1B[0m\n"
                             << "  09  Them thanh vien              10  Sua ho so\n"
                             << "  11  Xoa nhanh                     12  Sap xep con theo nam sinh\n"
                             << "  13  Quan ly tai khoan             14  Quan ly thanh tich\n"
                             << "  15  Them quy uoc                  16  Luu du lieu\n";
        else std::cout << "  16  Luu du lieu\n";
        std::cout << "  \x1B[38;2;132;73;95m00  Luu va dang xuat\n  +================================================================+\x1B[0m\n";
        choice = ReadInt("  \x1B[38;2;184;91;86mLua chon > \x1B[0m");
        switch (choice) {
            case 1: PrintFamilyTree(tree.root); break;
            case 2: PrintBfsByGeneration(tree.root, std::cout); break;
            case 3: SearchMenu(tree); break;
            case 4: SearchNameMenu(tree); break;
            case 5: PrintAchievements(achievements, ReadInt("ID thanh vien: "), std::cout); break;
            case 6: RuleMenu(rules, false); break;
            case 7: RelationshipMenu(tree); break;
            case 8: StatisticsMenu(tree, achievements); break;
            case 9: if (admin) AddMenu(tree); else std::cout << "Khong du quyen.\n"; break;
            case 10: if (admin) EditMenu(tree); else std::cout << "Khong du quyen.\n"; break;
            case 11:
                if (admin) {
                    int id = ReadInt("ID can xoa: ");
                    std::string answer = ReadLine("Xoa ca nhanh con va vo/chong? (y/n): ");
                    if (answer == "y" || answer == "Y")
                        std::cout << (DeleteBranch(tree, id) ? "Da xoa nhanh va vo/chong.\n" : "Khong tim thay ID.\n");
                    else std::cout << "Da huy.\n";
                } else std::cout << "Khong du quyen.\n";
                break;
            case 12:
                if (admin) {
                    int id = ReadInt("ID cha/me: ");
                    Node* parent = FindMember(tree.root, id);
                    if (parent) {
                        SortSiblingsByBirthDate(parent->firstChild);
                        parent->lastChild = parent->firstChild;
                        while (parent->lastChild && parent->lastChild->nextSibling)
                            parent->lastChild = parent->lastChild->nextSibling;
                        std::cout << "Da sap xep con ruot (vo/chong di cung cap doi).\n";
                    } else std::cout << "Khong tim thay.\n";
                } else std::cout << "Khong du quyen.\n";
                break;
            case 13: if (admin) AccountMenu(accounts, tree); else std::cout << "Khong du quyen.\n"; break;
            case 14: if (admin) AchievementMenu(achievements, tree); else std::cout << "Khong du quyen.\n"; break;
            case 15: if (admin) RuleMenu(rules, true); else std::cout << "Khong du quyen.\n"; break;
            case 16: SaveAll(familyPath, tree, accounts, achievements, rules); break;
            case 0: break;
            default: std::cout << "Lua chon khong hop le.\n";
        }
    } while (choice != 0);
    SaveAll(familyPath, tree, accounts, achievements, rules);
}



