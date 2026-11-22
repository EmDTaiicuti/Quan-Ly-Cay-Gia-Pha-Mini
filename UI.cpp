
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
