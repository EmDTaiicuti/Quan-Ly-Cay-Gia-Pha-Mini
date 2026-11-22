
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
