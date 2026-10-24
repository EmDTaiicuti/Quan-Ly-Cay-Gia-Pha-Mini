#ifndef FAMILY_NODE_H
#define FAMILY_NODE_H

#include <string>

struct Member {
    int id;
    int spouseId;
    std::string name;
    char gender;
    std::string birthDate;       // DD/MM/YYYY
    std::string deathDate;       // DD/MM/YYYY; rỗng nếu còn sống/chưa rõ.
    std::string hometown;
    std::string biography;
    std::string photoPath;
    std::string notes;
    bool isInLaw;                // Thành viên kết hôn vào, không thuộc huyết thống gốc.
    Member() : id(0), spouseId(0), name(), gender('O'), birthDate(), deathDate(),
               hometown(), biography(), photoPath(), notes(), isInLaw(false) {}
};

struct Node {
    Member member;
    Node* parent;
    Node* firstChild;
    Node* lastChild;
    Node* nextSibling;
    explicit Node(const Member& value, Node* parentNode = 0)
        : member(value), parent(parentNode), firstChild(0), lastChild(0), nextSibling(0) {}
};

struct FamilyTree {
    Node* root;
    Node* lastRoot;
    int memberCount;
    FamilyTree() : root(0), lastRoot(0), memberCount(0) {}
};

struct FamilyStats {
    int totalMembers, maleCount, femaleCount, otherCount, generationCount;
    FamilyStats() : totalMembers(0), maleCount(0), femaleCount(0), otherCount(0), generationCount(0) {}
};

#endif
