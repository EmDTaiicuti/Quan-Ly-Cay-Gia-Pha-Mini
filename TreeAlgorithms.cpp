#include "TreeAlgorithms.h"
#include "TreeCore.h"
#include <ostream>
#include <vector>

namespace {
struct QueueNode {
    Node* treeNode;  // NÃºt gia pháº£ Ä‘ang chá» xá»­ lÃ½.
    int depth;       // Tháº¿ há»‡ cá»§a nÃºt trong lÆ°á»£t BFS.
    QueueNode* next; // Pháº§n tá»­ káº¿ tiáº¿p trong hÃ ng Ä‘á»£i.
    QueueNode(Node* value, int level) : treeNode(value), depth(level), next(0) {}
};
struct Queue {
    QueueNode* front; // Äáº§u hÃ ng Ä‘á»£i, nÆ¡i láº¥y pháº§n tá»­ ra.
    QueueNode* back;  // Cuá»‘i hÃ ng Ä‘á»£i, nÆ¡i thÃªm pháº§n tá»­ vÃ o.
    Queue() : front(0), back(0) {}
    void Push(Node* node, int depth) {
        QueueNode* item = new QueueNode(node, depth);
        if (!back) front = back = item; else { back->next = item; back = item; }
    }
    QueueNode* Pop() {
        if (!front) return 0;
        QueueNode* item = front; front = front->next;
        if (!front) back = 0;
        item->next = 0; return item;
    }
    ~Queue() { while (front) { QueueNode* item = front; front = front->next; delete item; } }
};
void CountRecursive(Node* node, int depth, FamilyStats& stats) {
    if (!node) return;
    ++stats.totalMembers;
    if (node->member.gender == 'M') ++stats.maleCount;
    else if (node->member.gender == 'F') ++stats.femaleCount;
    else ++stats.otherCount;
    if (depth > stats.generationCount) stats.generationCount = depth;
    CountRecursive(node->firstChild, depth + 1, stats);
    CountRecursive(node->nextSibling, depth, stats);
}

void PrintTree(Node* node, Node* treeRoot, std::ostream& output, int depth) {
    if (!node) return;
    if (!node->member.isInLaw) {
        for (int i = 0; i < depth; ++i) output << "  ";
        output << "|- " << node->member.name << " [ID: " << node->member.id
               << "] " << node->member.birthDate;
        Node* spouse = node->member.spouseId ? FindMember(treeRoot, node->member.spouseId) : 0;
        if (spouse) output << " | Vo/chong: " << spouse->member.name << " [ID: " << spouse->member.id << "]";
        output << '\n';
        PrintTree(node->firstChild, treeRoot, output, depth + 1);
        if (spouse) PrintTree(spouse->firstChild, treeRoot, output, depth + 1);
    }
    PrintTree(node->nextSibling, treeRoot, output, depth);
}
}

Node* FindByDfs(Node* node, int id) {
    // O(1) tá»‘t nháº¥t; O(n) trung bÃ¬nh vÃ  xáº¥u nháº¥t.
    if (!node) return 0;
    if (node->member.id == id) return node;
    Node* found = FindByDfs(node->firstChild, id);
    return found ? found : FindByDfs(node->nextSibling, id);
}

void PrintDfs(Node* node, std::ostream& output, int depth) {
    // DFS in cÃ¢y: O(n) thá»i gian, O(h) ngÄƒn xáº¿p Ä‘á»‡ quy.
    PrintTree(node, node, output, depth);
}

void PrintBfsByGeneration(Node* root, std::ostream& output) {
    // BFS: O(n) thá»i gian, O(w) bá»™ nhá»› vá»›i w lÃ  bá» rá»™ng lá»›n nháº¥t cá»§a cÃ¢y.
    Queue queue;
    for (Node* current = root; current; current = current->nextSibling) queue.Push(current, 1);
    int generation = 0;
    while (queue.front) {
        QueueNode* item = queue.Pop();
        if (item->depth != generation) { generation = item->depth; output << "\nThe he " << generation << ":\n"; }
        Node* member = item->treeNode;
        if (!member->member.isInLaw) {
            output << "  " << member->member.name << " (ID: " << member->member.id
                   << ", " << member->member.birthDate << ")";
            Node* spouse = member->member.spouseId ? FindMember(root, member->member.spouseId) : 0;
            if (spouse) output << " | Vo/chong: " << spouse->member.name << " (ID: " << spouse->member.id << ")";
            output << '\n';
            for (Node* child = member->firstChild; child; child = child->nextSibling)
                if (!child->member.isInLaw) queue.Push(child, item->depth + 1);
            if (spouse)
                for (Node* child = spouse->firstChild; child; child = child->nextSibling)
                    if (!child->member.isInLaw) queue.Push(child, item->depth + 1);
        }
        delete item;
    }
}

namespace {
int GetDepth(Node* node) {
    int depth = 0;
    while (node) { ++depth; node = node->parent; }
    return depth;
}
}

namespace {
Node* FindLcaFromNodes(Node* first, Node* second) {
    int firstDepth = GetDepth(first);
    int secondDepth = GetDepth(second);
    while (firstDepth > secondDepth) { first = first->parent; --firstDepth; }
    while (secondDepth > firstDepth) { second = second->parent; --secondDepth; }
    while (first && second && first != second) {
        first = first->parent;
        second = second->parent;
    }
    return first == second ? first : 0;
}
}

Node* FindLowestCommonAncestor(Node* root, int firstId, int secondId) {
    // Hai láº§n tÃ¬m O(n), sau Ä‘Ã³ leo theo parent O(h); bá»™ nhá»› phá»¥ O(1).
    return FindLcaFromNodes(FindByDfs(root, firstId), FindByDfs(root, secondId));
}

int GetRelationshipDistance(Node* root, int firstId, int secondId) {
    // Hai láº§n tÃ¬m O(n), leo cÃ¢y O(h), bá»™ nhá»› phá»¥ O(1); trÃ¡nh tÃ¬m cÃ¢y thÃªm láº§n ná»¯a.
    Node* first = FindByDfs(root, firstId);
    Node* second = FindByDfs(root, secondId);
    if (!first || !second) return -1;
    Node* ancestor = FindLcaFromNodes(first, second);
    if (!ancestor) return -1;
    return GetDepth(first) + GetDepth(second) - 2 * GetDepth(ancestor);
}
namespace {
void AppendDetached(Node*& head, Node*& tail, Node* node) {
    node->nextSibling = 0;
    if (!head) head = node;
    else tail->nextSibling = node;
    tail = node;
}

void JoinChains(Node*& head, Node*& tail, Node* chainHead, Node* chainTail) {
    if (!chainHead) return;
    if (!head) head = chainHead;
    else tail->nextSibling = chainHead;
    tail = chainTail;
}

Node* QuickSortSiblingChain(Node* head, Node*& tail) {
    if (!head || !head->nextSibling) {
        tail = head;
        return head;
    }

    // Chá»n ngÃ y sinh nÃºt Ä‘áº§u lÃ m pivot; phÃ¢n vÃ¹ng báº±ng cÃ¡ch thÃ¡o/gáº¯n láº¡i liÃªn káº¿t.
    Node* slow = head;
    Node* fast = head;
    while (fast->nextSibling && fast->nextSibling->nextSibling) {
        slow = slow->nextSibling;
        fast = fast->nextSibling->nextSibling;
    }
    const std::string pivotDate = slow->member.birthDate;
    Node* newerHead = 0; Node* newerTail = 0;
    Node* equalHead = 0; Node* equalTail = 0;
    Node* olderHead = 0; Node* olderTail = 0;

    while (head) {
        Node* current = head;
        head = head->nextSibling;
        if (CompareDates(current->member.birthDate, pivotDate) > 0)
            AppendDetached(newerHead, newerTail, current);
        else if (CompareDates(current->member.birthDate, pivotDate) < 0)
            AppendDetached(olderHead, olderTail, current);
        else
            AppendDetached(equalHead, equalTail, current);
    }

    if (newerHead) newerHead = QuickSortSiblingChain(newerHead, newerTail);
    if (olderHead) olderHead = QuickSortSiblingChain(olderHead, olderTail);

    Node* sortedHead = 0; Node* sortedTail = 0;
    JoinChains(sortedHead, sortedTail, newerHead, newerTail);
    JoinChains(sortedHead, sortedTail, equalHead, equalTail);
    JoinChains(sortedHead, sortedTail, olderHead, olderTail);
    tail = sortedTail;
    return sortedHead;
}
}

void SortSiblingsByBirthDate(Node*& firstSibling) {
    // Quick Sort relink node: O(n log n) trung bÃ¬nh, O(n^2) xáº¥u nháº¥t,
    // O(log n) ngÄƒn xáº¿p trung bÃ¬nh; khÃ´ng sao chÃ©p/Ä‘á»•i dá»¯ liá»‡u Member.
    std::vector<Node*> spouses;
    Node* bloodline = 0;
    Node* tail = 0;
    while (firstSibling) {
        Node* current = firstSibling;
        firstSibling = firstSibling->nextSibling;
        current->nextSibling = 0;
        if (current->member.isInLaw) spouses.push_back(current);
        else AppendDetached(bloodline, tail, current);
    }
    bloodline = QuickSortSiblingChain(bloodline, tail);
    Node* sorted = 0;
    tail = 0;
    while (bloodline) {
        Node* current = bloodline;
        bloodline = bloodline->nextSibling;
        current->nextSibling = 0;
        AppendDetached(sorted, tail, current);
        for (std::size_t i = 0; i < spouses.size(); ++i) {
            if (spouses[i] && spouses[i]->member.spouseId == current->member.id) {
                AppendDetached(sorted, tail, spouses[i]);
                spouses[i] = 0;
                break;
            }
        }
    }
    for (std::size_t i = 0; i < spouses.size(); ++i)
        if (spouses[i]) AppendDetached(sorted, tail, spouses[i]);
    firstSibling = sorted;
}
void CollectStatistics(Node* root, FamilyStats& stats) {
    // ThÄƒm má»—i nÃºt má»™t láº§n: O(n) thá»i gian, O(h) ngÄƒn xáº¿p.
    stats = FamilyStats(); CountRecursive(root, 1, stats);
}









namespace {
std::string LowerAscii(const std::string& value) {
    std::string result = value;
    struct Accent { const char* source; const char* target; };
    static const Accent accents[] = {
        {"Ã¡","a"},{"Ã ","a"},{"áº£","a"},{"Ã£","a"},{"áº¡","a"},{"Äƒ","a"},{"áº¯","a"},{"áº±","a"},{"áº³","a"},{"áºµ","a"},{"áº·","a"},{"Ã¢","a"},{"áº¥","a"},{"áº§","a"},{"áº©","a"},{"áº«","a"},{"áº­","a"},
        {"Ã©","e"},{"Ã¨","e"},{"áº»","e"},{"áº½","e"},{"áº¹","e"},{"Ãª","e"},{"áº¿","e"},{"á»","e"},{"á»ƒ","e"},{"á»…","e"},{"á»‡","e"},
        {"Ã­","i"},{"Ã¬","i"},{"á»‰","i"},{"Ä©","i"},{"á»‹","i"},
        {"Ã³","o"},{"Ã²","o"},{"á»","o"},{"Ãµ","o"},{"á»","o"},{"Ã´","o"},{"á»‘","o"},{"á»“","o"},{"á»•","o"},{"á»—","o"},{"á»™","o"},{"Æ¡","o"},{"á»›","o"},{"á»","o"},{"á»Ÿ","o"},{"á»¡","o"},{"á»£","o"},
        {"Ãº","u"},{"Ã¹","u"},{"á»§","u"},{"Å©","u"},{"á»¥","u"},{"Æ°","u"},{"á»©","u"},{"á»«","u"},{"á»­","u"},{"á»¯","u"},{"á»±","u"},
        {"Ã½","y"},{"á»³","y"},{"á»·","y"},{"á»¹","y"},{"á»µ","y"},{"Ä‘","d"},
        {"Ã","a"},{"Ã€","a"},{"áº¢","a"},{"Ãƒ","a"},{"áº ","a"},{"Ä‚","a"},{"áº®","a"},{"áº°","a"},{"áº²","a"},{"áº´","a"},{"áº¶","a"},{"Ã‚","a"},{"áº¤","a"},{"áº¦","a"},{"áº¨","a"},{"áºª","a"},{"áº¬","a"},
        {"Ã‰","e"},{"Ãˆ","e"},{"áºº","e"},{"áº¼","e"},{"áº¸","e"},{"ÃŠ","e"},{"áº¾","e"},{"á»€","e"},{"á»‚","e"},{"á»„","e"},{"á»†","e"},
        {"Ã","i"},{"ÃŒ","i"},{"á»ˆ","i"},{"Ä¨","i"},{"á»Š","i"},
        {"Ã“","o"},{"Ã’","o"},{"á»Ž","o"},{"Ã•","o"},{"á»Œ","o"},{"Ã”","o"},{"á»","o"},{"á»’","o"},{"á»”","o"},{"á»–","o"},{"á»˜","o"},{"Æ ","o"},{"á»š","o"},{"á»œ","o"},{"á»ž","o"},{"á» ","o"},{"á»¢","o"},
        {"Ãš","u"},{"Ã™","u"},{"á»¦","u"},{"Å¨","u"},{"á»¤","u"},{"Æ¯","u"},{"á»¨","u"},{"á»ª","u"},{"á»¬","u"},{"á»®","u"},{"á»°","u"},
        {"Ã","y"},{"á»²","y"},{"á»¶","y"},{"á»¸","y"},{"á»´","y"},{"Ä","d"}
    };
    std::string folded;
    for (std::size_t i = 0; i < result.size();) {
        const unsigned char c = static_cast<unsigned char>(result[i]);
        if (c >= 'A' && c <= 'Z') { folded += static_cast<char>(c - 'A' + 'a'); ++i; continue; }
        bool matched = false;
        for (std::size_t j = 0; j < sizeof(accents) / sizeof(accents[0]); ++j) {
            const std::string source(accents[j].source);
            if (result.compare(i, source.size(), source) == 0) {
                folded += accents[j].target;
                i += source.size();
                matched = true;
                break;
            }
        }
        if (!matched) { folded += result[i]; ++i; }
    }
    return folded;
}
void SearchNameRecursive(Node* node, const std::string& query, std::ostream& output, int& count) {
    if (!node) return;
    if (LowerAscii(node->member.name).find(query) != std::string::npos) {
        output << "  ID " << node->member.id << " | " << node->member.name << " | " << node->member.birthDate << '\n';
        ++count;
    }
    SearchNameRecursive(node->firstChild, query, output, count);
    SearchNameRecursive(node->nextSibling, query, output, count);
}
int ExtractYear(const std::string& date) {
    if (!IsValidDate(date)) return -1;
    return (date[6]-'0')*1000 + (date[7]-'0')*100 + (date[8]-'0')*10 + date[9]-'0';
}
void CountYearRecursive(Node* node, int year, int& births, int& deaths) {
    if (!node) return;
    if (ExtractYear(node->member.birthDate) == year) ++births;
    if (ExtractYear(node->member.deathDate) == year) ++deaths;
    CountYearRecursive(node->firstChild, year, births, deaths);
    CountYearRecursive(node->nextSibling, year, births, deaths);
}
}

void SearchByNameContains(Node* root, const std::string& query, std::ostream& output, int& count) {
    // Duyá»‡t tuyáº¿n tÃ­nh toÃ n cÃ¢y: best O(1) náº¿u khá»›p á»Ÿ nÃºt Ä‘áº§u; average/worst O(n), stack O(h).
    count = 0;
    if (query.empty()) return;
    SearchNameRecursive(root, LowerAscii(query), output, count);
}

void CountYear(Node* root, int year, int& births, int& deaths) {
    // Thá»‘ng kÃª pháº£i xÃ©t má»i thÃ nh viÃªn: O(n) thá»i gian, O(h) ngÄƒn xáº¿p Ä‘á»‡ quy.
    births = 0; deaths = 0; CountYearRecursive(root, year, births, deaths);
}