#include "TreeCore.h"
#include <cctype>

namespace {
bool IsLeapYear(int year) { return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0); }

bool IsUniqueId(Node* node, int id) {
    if (!node) return true;
    if (node->member.id == id) return false;
    return IsUniqueId(node->firstChild, id) && IsUniqueId(node->nextSibling, id);
}

int CountBranch(Node* node) {
    if (!node) return 0;
    int count = 1;
    for (Node* child = node->firstChild; child; child = child->nextSibling)
        count += CountBranch(child);
    return count;
}
void ClearBranchSpouseLinks(Node* branch, Node* wholeTree) {
    if (!branch) return;
    if (branch->member.spouseId != 0) {
        Node* spouse = FindMember(wholeTree, branch->member.spouseId);
        if (spouse) spouse->member.spouseId = 0;
    }
    // Duyệt đúng các con trong nhánh; tuyệt đối không đi theo nextSibling của gốc nhánh,
    // vì đó có thể là người thân ở ngoài vùng đang xóa.
    for (Node* child = branch->firstChild; child; child = child->nextSibling)
        ClearBranchSpouseLinks(child, wholeTree);
}

void AppendNode(Node*& head, Node*& tail, Node* added, Node* parent) {
    added->parent = parent;
    if (!head) head = added;
    else tail->nextSibling = added;
    tail = added;
}
}

void InitializeTree(FamilyTree& familyTree) {
    familyTree.root = 0;
    familyTree.lastRoot = 0;
    familyTree.memberCount = 0;
}

void DestroyTree(Node*& node) {
    if (!node) return;
    DestroyTree(node->firstChild);
    DestroyTree(node->nextSibling);
    delete node;
    node = 0;
}

Node* FindMember(Node* node, int id) {
    if (!node) return 0;
    if (node->member.id == id) return node;
    Node* found = FindMember(node->firstChild, id);
    return found ? found : FindMember(node->nextSibling, id);
}

Node* FindParent(FamilyTree& familyTree, int childId) {
    Node* member = FindMember(familyTree.root, childId);
    return member ? member->parent : 0;
}

bool IsValidDate(const std::string& date) {
    // Định dạng chuẩn: DD/MM/YYYY.
    if (date.size() != 10 || date[2] != '/' || date[5] != '/') return false;
    for (std::size_t i = 0; i < date.size(); ++i)
        if (i != 2 && i != 5 && !std::isdigit(static_cast<unsigned char>(date[i]))) return false;

    int day = (date[0]-'0')*10 + date[1]-'0';
    int month = (date[3]-'0')*10 + date[4]-'0';
    int year = (date[6]-'0')*1000 + (date[7]-'0')*100 + (date[8]-'0')*10 + date[9]-'0';
    if (year < 1 || month < 1 || month > 12) return false;
    const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int maxDay = days[month - 1] + (month == 2 && IsLeapYear(year) ? 1 : 0);
    return day >= 1 && day <= maxDay;
}

int CompareDates(const std::string& firstDate, const std::string& secondDate) {
    // Trả về -1, 0, 1 theo thứ tự thời gian; ngày phải hợp lệ DD/MM/YYYY.
    int firstYear = (firstDate[6]-'0')*1000 + (firstDate[7]-'0')*100 + (firstDate[8]-'0')*10 + firstDate[9]-'0';
    int secondYear = (secondDate[6]-'0')*1000 + (secondDate[7]-'0')*100 + (secondDate[8]-'0')*10 + secondDate[9]-'0';
    if (firstYear != secondYear) return firstYear < secondYear ? -1 : 1;
    int firstMonth = (firstDate[3]-'0')*10 + firstDate[4]-'0';
    int secondMonth = (secondDate[3]-'0')*10 + secondDate[4]-'0';
    if (firstMonth != secondMonth) return firstMonth < secondMonth ? -1 : 1;
    int firstDay = (firstDate[0]-'0')*10 + firstDate[1]-'0';
    int secondDay = (secondDate[0]-'0')*10 + secondDate[1]-'0';
    if (firstDay == secondDay) return 0;
    return firstDay < secondDay ? -1 : 1;
}

std::string NormalizeDate(const std::string& date) {
    // Chuẩn hóa dữ liệu ISO cũ YYYY-MM-DD khi nạp CSV sang DD/MM/YYYY.
    if (IsValidDate(date)) return date;
    if (date.size() == 10 && date[4] == '-' && date[7] == '-') {
        std::string converted = date.substr(8, 2) + "/" + date.substr(5, 2) + "/" + date.substr(0, 4);
        if (IsValidDate(converted)) return converted;
    }
    return date;
}

bool IsBirthDateValidForParent(const std::string& childDate, const std::string& parentDate) {
    return IsValidDate(childDate) && IsValidDate(parentDate) && CompareDates(childDate, parentDate) >= 0;
}
bool AddRoot(FamilyTree& familyTree, const Member& member) {
    if (member.id <= 0 || !IsUniqueId(familyTree.root, member.id) || !IsValidDate(member.birthDate) || (!member.deathDate.empty() && (!IsValidDate(member.deathDate) || CompareDates(member.deathDate, member.birthDate) < 0))) return false;
    AppendNode(familyTree.root, familyTree.lastRoot, new Node(member), 0);
    ++familyTree.memberCount;
    return true;
}

bool AddChild(FamilyTree& familyTree, int parentId, const Member& member) {
    Node* parent = FindMember(familyTree.root, parentId);
    if (!parent || member.id <= 0 || !IsUniqueId(familyTree.root, member.id) ||
        !IsBirthDateValidForParent(member.birthDate, parent->member.birthDate) || (!member.deathDate.empty() && (!IsValidDate(member.deathDate) || CompareDates(member.deathDate, member.birthDate) < 0))) return false;
    AppendNode(parent->firstChild, parent->lastChild, new Node(member, parent), parent);
    ++familyTree.memberCount;
    return true;
}

bool AddSpouse(FamilyTree& familyTree, int memberId, Member spouse) {
    Node* member = FindMember(familyTree.root, memberId);
    if (!member || member->member.spouseId != 0 || spouse.spouseId != 0 || spouse.id <= 0 || !IsUniqueId(familyTree.root, spouse.id) || !IsValidDate(spouse.birthDate) || (!spouse.deathDate.empty() && (!IsValidDate(spouse.deathDate) || CompareDates(spouse.deathDate, spouse.birthDate) < 0))) return false;
    spouse.spouseId = memberId;
    member->member.spouseId = spouse.id;
    Node* parent = member->parent;
    if (parent) AppendNode(parent->firstChild, parent->lastChild, new Node(spouse, parent), parent);
    else AppendNode(familyTree.root, familyTree.lastRoot, new Node(spouse), 0);
    ++familyTree.memberCount;
    return true;
}

bool UpdateMember(FamilyTree& familyTree, int id, const Member& updatedMember) {
    Node* node = FindMember(familyTree.root, id);
    if (!node || !IsValidDate(updatedMember.birthDate) || (!updatedMember.deathDate.empty() && (!IsValidDate(updatedMember.deathDate) || CompareDates(updatedMember.deathDate, updatedMember.birthDate) < 0))) return false;
    if (node->parent && !IsBirthDateValidForParent(updatedMember.birthDate, node->parent->member.birthDate)) return false;
    for (Node* child = node->firstChild; child; child = child->nextSibling)
        if (!IsBirthDateValidForParent(child->member.birthDate, updatedMember.birthDate)) return false;
    const int originalId = node->member.id;
    const int originalSpouseId = node->member.spouseId;
    const bool originalInLaw = node->member.isInLaw;
    node->member = updatedMember;
    node->member.id = originalId;
    node->member.spouseId = originalSpouseId;
    node->member.isInLaw = originalInLaw;
    return true;
}

bool DeleteBranch(FamilyTree& familyTree, int id) {
    Node* target = FindMember(familyTree.root, id);
    if (!target) return false;

    Node* spouse = target->member.spouseId ? FindMember(familyTree.root, target->member.spouseId) : 0;
    if (spouse && spouse != target && spouse->parent == target->parent) {
        ClearBranchSpouseLinks(spouse, familyTree.root);
        Node* spouseParent = spouse->parent;
        Node*& spouseHead = spouseParent ? spouseParent->firstChild : familyTree.root;
        Node*& spouseTail = spouseParent ? spouseParent->lastChild : familyTree.lastRoot;
        Node* beforeSpouse = 0;
        Node* currentSpouse = spouseHead;
        while (currentSpouse && currentSpouse != spouse) {
            beforeSpouse = currentSpouse;
            currentSpouse = currentSpouse->nextSibling;
        }
        if (currentSpouse) {
            if (beforeSpouse) beforeSpouse->nextSibling = spouse->nextSibling;
            else spouseHead = spouse->nextSibling;
            if (spouseTail == spouse) spouseTail = beforeSpouse;
            const int spouseCount = CountBranch(spouse);
            spouse->nextSibling = 0;
            familyTree.memberCount -= spouseCount;
            DestroyTree(spouse);
        }
    }

    ClearBranchSpouseLinks(target, familyTree.root);

    Node* parent = target->parent;
    Node*& head = parent ? parent->firstChild : familyTree.root;
    Node*& tail = parent ? parent->lastChild : familyTree.lastRoot;
    Node* previous = 0;
    Node* current = head;
    while (current && current != target) { previous = current; current = current->nextSibling; }
    if (!current) return false;

    if (previous) previous->nextSibling = target->nextSibling;
    else head = target->nextSibling;
    if (tail == target) tail = previous;

    const int removedCount = CountBranch(target);
    target->nextSibling = 0;
    familyTree.memberCount -= removedCount;
    DestroyTree(target);
    return true;
}





