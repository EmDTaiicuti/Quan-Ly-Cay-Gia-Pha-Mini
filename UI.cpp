"), std::cout); break;
            case 6: RuleMenu(rules, false); break;
            case 7: RelationshipMenu(tree); break;
            case 8: StatisticsMenu(tree, achievements); break;
            case 9: if (admin) AddMenu(tree); else std::cout << "Khong du quyen.\n"; break;
            case 10: if (admin) EditMenu(tree); else std::cout << 
