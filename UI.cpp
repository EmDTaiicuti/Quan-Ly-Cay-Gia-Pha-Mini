
  
}
void StatisticsMenu(FamilyTree& tree, AchievementList& achievements) {
    FamilyStats stats = ComputeStats(tree.root);
    std::cout << "Tong thanh vien: " << stats.totalMembers << "\nNam: " << stats.maleCount
              << "\nNu: " << stats.femaleCount << "\nKhac: " << stats.otherCount
              << "\nSo the he: " << stats.generationCount << '\n';
    int year = ReadInt("Nam thong ke (0 bo qua): ");
    if (year > 0) {
        int births = 0, deaths = 0, awards = 0; // CountYear(tree.root, year, births, deaths);
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
    bool ok = SaveToCsv(familyPath, tree) && SaveAccounts("accounts.csv", accounts) &&
              SaveAchievements("achievements.csv", achievements) && SaveRules("family_rules.csv", rules);
    std::cout << (ok ? "Da luu toan bo du lieu.\n" : "Luu that bai; kiem tra quyen ghi file.\n");
}
}

void PrintFamilyTree(Node* root) {
    if (!root) { std::cout << "(Cay gia pha dang rong)\n"; return; }
   BfsPrintByGeneration(root);
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
    int choice;
    do {
        FamilyStats stats = ComputeStats(tree.root); const bool admin = current->role == 'A';
        std::cout << "\n\x1B[96m  +================================================================+\n"
                  << "  |                 GIA PHA  /  FAMILY TREE                         |\n"
                  << "  +================================================================+\n\x1B[0m"
                  << "  Thanh vien: " << stats.totalMembers << " | The he: " << stats.generationCount
                  << " | Tai khoan: " << current->username << (admin ? " [ADMIN]\n" : " [USER]\n")
                  << "  01  In so do gia pha             02  Liet ke theo the he (BFS)\n"
                  << "  03  Tim theo ID                  04  Tim ten gan dung\n"
                  << "  05  Xem thanh tich               06  Xem quy uoc gia toc\n"
                  << "  07  Tim to tien / khoang cach    08  Thong ke theo nam\n";
        if (admin) std::cout << "  09  Them thanh vien              10  Sua ho so\n"
                             << "  11  Xoa nhanh                     12  Sap xep con theo nam sinh\n"
                             << "  13  Quan ly tai khoan             14  Quan ly thanh tich\n"
                             << "  15  Them quy uoc                  16  Luu du lieu\n";
        else std::cout << "  16  Luu du lieu\n";
        std::cout << "  00  Luu va dang xuat\n  ==================================================================\n";
        choice = ReadInt("  Lua chon > ");
        switch (choice) {
            case 1: PrintFamilyTree(tree.root); break;
           case 2: BfsPrintByGeneration(tree.root); break;
            case 3: SearchMenu(tree); break;
            case 4: SearchNameMenu(tree); break;
            case 5: PrintAchievements(achievements, ReadInt("ID thanh vien: "), std::cout); break;
            case 6: RuleMenu(rules, false); break;
            case 7: RelationshipMenu(tree); break;
            case 8: StatisticsMenu(tree, achievements); break;
            case 9: if (admin) AddMenu(tree); else std::cout << "Khong du quyen.\n"; break;
            case 10: if (admin) EditMenu(tree); else std::cout << 
