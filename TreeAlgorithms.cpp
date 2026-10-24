#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <string>

#include "TreeAlgorithms.h"

// ===================== HÀNG ĐỢI TỰ CÀI ĐẶT =====================
// (Không dùng std::queue)

namespace {

struct QueueNode {
    Node* treeNode;
    QueueNode* next;
};

struct NodeQueue {
    QueueNode* head;   // Đầu hàng: nơi lấy ra
    QueueNode* tail;   // Cuối hàng: nơi thêm vào
    int size;
};

}  // namespace

static void QueueInit(NodeQueue& queue) {
    queue.head = nullptr;
    queue.tail = nullptr;
    queue.size = 0;
}

static void Enqueue(NodeQueue& queue, Node* treeNode) {
    QueueNode* item = new QueueNode{treeNode, nullptr};

    if (queue.tail == nullptr) {
        // Hàng rỗng: head và tail cùng trỏ vào phần tử đầu tiên
        queue.head = item;
        queue.tail = item;
    } else {
        // Nối vào sau tail rồi dời tail
        queue.tail->next = item;
        queue.tail = item;
    }

    ++queue.size;
}

static Node* Dequeue(NodeQueue& queue) {
    QueueNode* item = queue.head;
    Node* treeNode = item->treeNode;

    queue.head = item->next;

    if (queue.head == nullptr) {
        // Vừa lấy phần tử cuối -> hàng rỗng
        queue.tail = nullptr;
    }

    delete item;       // Giải phóng ô hàng đợi
    --queue.size;

    return treeNode;
}

// ===================== CHUẨN HÓA TÊN ĐỂ TÌM KIẾM (sửa lỗi C) =====================

// Kiểm tra byte ở vị trí pos có phải byte tiếp nối của UTF-8 (dạng 10xxxxxx) không.
static bool IsContinuationByte(const std::string& text, size_t pos) {
    return pos < text.size()
        && (static_cast<unsigned char>(text[pos]) & 0xC0) == 0x80;
}

// Trả về chữ cái gốc (không dấu, chữ thường) của một mã Unicode tiếng Việt.
// Trả về 0 nếu mã đó không phải chữ có dấu tiếng Việt.
static char VietnameseBaseLetter(unsigned int cp) {
    switch (cp) {
        // Latin-1 Supplement và Latin Extended: À Á Â Ã Ă ... (hoa và thường)
        case 0xC0: case 0xC1: case 0xC2: case 0xC3:
        case 0xE0: case 0xE1: case 0xE2: case 0xE3:
        case 0x102: case 0x103:                         // Ă ă
            return 'a';

        case 0xC8: case 0xC9: case 0xCA:
        case 0xE8: case 0xE9: case 0xEA:
            return 'e';

        case 0xCC: case 0xCD: case 0xEC: case 0xED:
        case 0x128: case 0x129:                         // Ĩ ĩ
            return 'i';

        case 0xD2: case 0xD3: case 0xD4: case 0xD5:
        case 0xF2: case 0xF3: case 0xF4: case 0xF5:
        case 0x1A0: case 0x1A1:                         // Ơ ơ
            return 'o';

        case 0xD9: case 0xDA: case 0xF9: case 0xFA:
        case 0x168: case 0x169:                         // Ũ ũ
        case 0x1AF: case 0x1B0:                         // Ư ư
            return 'u';

        case 0xDD: case 0xFD:                           // Ý ý
            return 'y';

        case 0x110: case 0x111:                         // Đ đ
            return 'd';

        default:
            break;
    }

    // Latin Extended Additional: các chữ có dấu thanh (ạ ả ấ ầ ẩ ẫ ậ ...)
    if (cp >= 0x1EA0 && cp <= 0x1EB7) return 'a';
    if (cp >= 0x1EB8 && cp <= 0x1EC7) return 'e';
    if (cp >= 0x1EC8 && cp <= 0x1ECB) return 'i';
    if (cp >= 0x1ECC && cp <= 0x1EE3) return 'o';
    if (cp >= 0x1EE4 && cp <= 0x1EF1) return 'u';
    if (cp >= 0x1EF2 && cp <= 0x1EF9) return 'y';

    return 0;
}

// Chuẩn hóa chuỗi UTF-8 về dạng "chữ thường, không dấu":
//   "Nguyễn Văn Hùng" -> "nguyen van hung".
// - Bỏ dấu tiếng Việt, đổi đ/Đ thành d.
// - Bỏ luôn dấu rời (U+0300..U+036F) nếu chuỗi ở dạng tổ hợp (NFD).
// - Byte không phải tiếng Việt được giữ nguyên.
// Big-O: Thời gian O(L); Bộ nhớ O(L).
static std::string NormalizeForSearch(const std::string& text) {
    std::string result;
    result.reserve(text.size());

    size_t i = 0;

    while (i < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[i]);

        if (c < 0x80) {
            // ASCII: chỉ cần đổi sang chữ thường
            result += static_cast<char>(std::tolower(c));
            ++i;
            continue;
        }

        unsigned int cp = 0;
        size_t len = 0;

        if ((c & 0xE0) == 0xC0 && IsContinuationByte(text, i + 1)) {
            // Ký tự 2 byte
            cp = (static_cast<unsigned int>(c & 0x1F) << 6)
               | (static_cast<unsigned char>(text[i + 1]) & 0x3F);
            len = 2;
        } else if ((c & 0xF0) == 0xE0
                   && IsContinuationByte(text, i + 1)
                   && IsContinuationByte(text, i + 2)) {
            // Ký tự 3 byte (các chữ có dấu thanh như ệ, ổ, ữ ...)
            cp = (static_cast<unsigned int>(c & 0x0F) << 12)
               | ((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 6)
               | (static_cast<unsigned char>(text[i + 2]) & 0x3F);
            len = 3;
        } else {
            // Byte lạ / ký tự 4 byte: giữ nguyên, không xử lý
            result += text[i];
            ++i;
            continue;
        }

        if (cp >= 0x0300 && cp <= 0x036F) {
            // Dấu rời (NFD): bỏ đi
        } else {
            char base = VietnameseBaseLetter(cp);

            if (base != 0) {
                result += base;
            } else {
                result.append(text, i, len);   // ký tự khác: giữ nguyên
            }
        }

        i += len;
    }

    return result;
}

// true nếu tên của node chứa từ khóa (từ khóa đã chuẩn hóa).
static bool NameContains(const Node* node, const std::string& normalizedKey) {
    return NormalizeForSearch(node->member.name).find(normalizedKey)
           != std::string::npos;
}

// ===================== HÀM PHỤ DÙNG CHUNG =====================

// Dòng mô tả một thành viên: "Tên [ID n]".
// (Nếu project đã có hàm mô tả riêng, chỉ cần đổi nội dung hàm này.)
static std::string FormatMember(const Member& member) {
    return member.name + " [ID " + std::to_string(member.id) + "]";
}

// Ngày sinh "DD/MM/YYYY" -> số YYYYMMDD để so sánh.
// Trả về 0 nếu rỗng hoặc sai định dạng (xếp như người sinh sớm nhất).
static long BirthKey(const std::string& birthDate) {
    int day = 0;
    int month = 0;
    int year = 0;

    if (std::sscanf(birthDate.c_str(), "%d/%d/%d", &day, &month, &year) != 3) {
        return 0;
    }

    return static_cast<long>(year) * 10000L + month * 100L + day;
}

// ===================== VỢ/CHỒNG NHẬP GIA (sửa lỗi A) =====================
//
// - Người huyết thống (con ruột): member.isInLaw = false. Chỉ họ mới là "con"
//   khi sắp xếp, tính số thế hệ và đi xuống nhánh con.
// - Vợ/chồng nhập gia: member.isInLaw = true. Trong family.csv họ nằm cùng
//   chuỗi anh em với người phối ngẫu, nên các thuật toán ghép cặp trong chuỗi
//   bằng spouseId thay vì coi họ là một người con nữa.

static bool IsInLaw(const Node* node) {
    return node != nullptr && node->member.isInLaw;
}

// Hai người là vợ/chồng nếu spouseId của người này trỏ tới id người kia
// (chấp nhận khai báo một chiều hoặc hai chiều). spouseId = 0 nghĩa là không có.
static bool AreSpouses(const Node* a, const Node* b) {
    return (a->member.spouseId != 0 && a->member.spouseId == b->member.id)
        || (b->member.spouseId != 0 && b->member.spouseId == a->member.id);
}

// Tìm vợ/chồng của `person` trong chuỗi anh em bắt đầu từ `head`.
// Người huyết thống chỉ ghép với vợ/chồng nhập gia và ngược lại.
// Big-O: O(k) với k là số node trong chuỗi.
static const Node* FindSpouseInChain(const Node* head, const Node* person) {
    for (const Node* sibling = head;
         sibling != nullptr;
         sibling = sibling->nextSibling) {

        if (sibling != person
            && IsInLaw(sibling) != IsInLaw(person)
            && AreSpouses(person, sibling)) {
            return sibling;
        }
    }

    return nullptr;
}

// Đầu chuỗi anh em chứa node. Với node gốc (không có parent) thì dùng
// rootHead do người gọi cung cấp.
static const Node* ChainHead(const Node* node, const Node* rootHead) {
    return (node->parent != nullptr) ? node->parent->firstChild : rootHead;
}

// Vợ/chồng nhập gia đã có người phối ngẫu trong chuỗi -> sẽ được in/đếm cạnh
// người đó, nên bỏ qua khi duyệt chuỗi như một "người con".
static bool IsPairedInLaw(const Node* head, const Node* node) {
    return IsInLaw(node) && FindSpouseInChain(head, node) != nullptr;
}

// ===================== DFS =====================

// DFS tìm đường "xuất xứ": in từ người cần tìm ngược lên thủy tổ.
// Cách hoạt động: duyệt sâu xuống con (firstChild),
// duyệt ngang qua anh em (nextSibling).
// Khi tìm thấy mục tiêu thì trả true; khi đệ quy QUAY LUI,
// mỗi tổ tiên trên đường về sẽ tự in tên mình,
// nhờ đó đường đi được in ra mà không cần mảng/ngăn xếp phụ.
// Nếu mục tiêu là vợ/chồng nhập gia thì ghi chú là vợ/chồng của ai.
// Big-O: Thời gian O(n) (xấu nhất duyệt hết cây);
// Bộ nhớ O(h) cho ngăn xếp đệ quy (h = chiều cao).

bool DfsPrintLineage(Node* node, int targetId) {
    for (Node* current = node;
         current != nullptr;
         current = current->nextSibling) {

        if (current->member.id == targetId) {
            std::cout << FormatMember(current->member);

            if (IsInLaw(current)) {
                const Node* partner = FindSpouseInChain(node, current);

                if (partner != nullptr) {
                    std::cout << " [vợ/chồng của "
                              << partner->member.name << "]";
                } else {
                    std::cout << " [vợ/chồng nhập gia]";
                }
            }

            return true;
        }

        if (DfsPrintLineage(current->firstChild, targetId)) {
            // Tìm sâu xuống nhánh con
            std::cout << "  <-  "
                      << FormatMember(current->member);

            // In khi quay lui
            return true;
        }
    }

    return false;
}


// Hàm đệ quy thực sự của DfsSearchByName.
// Nhận sẵn từ khóa ĐÃ chuẩn hóa để không phải chuẩn hóa lại ở mỗi lần gọi.
static int SearchByNameRec(Node* node, const std::string& normalizedKey) {
    int found = 0;

    for (Node* current = node;
         current != nullptr;
         current = current->nextSibling) {

        if (NameContains(current, normalizedKey)) {
            std::cout << "  - " << FormatMember(current->member) << "  (";

            if (IsInLaw(current)) {
                const Node* partner = FindSpouseInChain(node, current);

                if (partner != nullptr) {
                    std::cout << "vợ/chồng của "
                              << partner->member.name << ", ";
                } else {
                    std::cout << "vợ/chồng nhập gia, ";
                }
            }

            std::cout << "Đời " << GetDepth(current) + 1 << ")\n";

            ++found;
        }

        found += SearchByNameRec(current->firstChild, normalizedKey);
    }

    return found;
}

// DFS (tiền tự) tìm mọi thành viên có tên chứa từ khóa.
// Không phân biệt hoa/thường và KHÔNG phân biệt dấu tiếng Việt:
// "nguyen van", "NGUYEN VAN", "Nguyễn Văn" đều ra cùng kết quả.
// Big-O: Thời gian O(n * L) với L là độ dài tên;
// Bộ nhớ O(h).

int DfsSearchByName(
    Node* node,
    const std::string& keyword
) {
    return SearchByNameRec(node, NormalizeForSearch(keyword));
}


// ===================== BFS =====================

// Xếp hàng cả chuỗi anh em bắt đầu từ head cho đời kế tiếp.
// Vợ/chồng đã có người phối ngẫu thì bỏ qua (được in cạnh người đó).
static void EnqueueChain(NodeQueue& queue, Node* head) {
    for (Node* child = head;
         child != nullptr;
         child = child->nextSibling) {

        if (IsPairedInLaw(head, child)) {
            continue;
        }

        Enqueue(queue, child);
    }
}

// BFS theo từng thế hệ.
// Mỗi vòng while xử lý đúng "levelSize" phần tử
// = toàn bộ một đời.
// Con của họ (đi theo firstChild rồi nextSibling)
// được xếp hàng cho đời kế tiếp.
// Vợ/chồng nhập gia được in cạnh người phối ngẫu ("A  &  B"),
// không xếp hàng như một người con.
// root là đầu chuỗi gốc (có thể có nhiều gốc nối bằng nextSibling).
// Big-O: Thời gian O(n * k) với k là số anh em đông nhất (ghép cặp trong chuỗi);
// Bộ nhớ O(w) với w là số người đông nhất trong một thế hệ.

void BfsPrintByGeneration(Node* root) {

    if (root == nullptr) {
        std::cout << "Cây gia phả đang rỗng.\n";
        return;
    }

    NodeQueue queue;

    QueueInit(queue);

    EnqueueChain(queue, root);

    int generation = 1;

    while (queue.size > 0) {

        int levelSize = queue.size;
        // Chốt số lượng của đời hiện tại
        // TRƯỚC khi thêm con vào hàng.

        std::cout << "Đời " << generation << ":\n";

        for (int i = 0; i < levelSize; ++i) {

            Node* current = Dequeue(queue);

            std::cout << "    "
                      << FormatMember(current->member);

            if (IsInLaw(current)) {
                // Vợ/chồng không tìm thấy người phối ngẫu trong chuỗi
                std::cout << "  [vợ/chồng nhập gia]";
            } else {
                const Node* head = ChainHead(current, root);

                for (const Node* sibling = head;
                     sibling != nullptr;
                     sibling = sibling->nextSibling) {

                    if (IsInLaw(sibling) && AreSpouses(current, sibling)) {
                        std::cout << "  &  "
                                  << FormatMember(sibling->member);
                    }
                }
            }

            std::cout << "\n";

            EnqueueChain(queue, current->firstChild);
        }

        ++generation;
    }
}


// ===================== ĐỘ SÂU, LCA, KHOẢNG CÁCH =====================

// Độ sâu = số cạnh đi ngược lên gốc nhờ con trỏ parent.
// Thủy tổ có độ sâu 0.
// Big-O: Thời gian O(h); Bộ nhớ O(1).

int GetDepth(const Node* node) {
    int depth = 0;

    while (node != nullptr && node->parent != nullptr) {
        node = node->parent;
        ++depth;
    }

    return depth;
}


// Tổ tiên chung gần nhất (LCA) bằng con trỏ parent:
// 1. Đưa người sâu hơn nhích lên cho cùng độ sâu với người kia.
// 2. Cả hai cùng nhích lên từng bước cho tới khi trùng nhau
//    -> đó là LCA.
// Yêu cầu a, b là người huyết thống
// (UI đã quy vợ/chồng về người huyết thống).
// Big-O: Thời gian O(h); Bộ nhớ O(1).

Node* FindLca(Node* a, Node* b) {
    if (a == nullptr || b == nullptr) {
        return nullptr;
    }

    int depthA = GetDepth(a);
    int depthB = GetDepth(b);

    while (depthA > depthB) {
        a = a->parent;
        --depthA;
    }

    while (depthB > depthA) {
        b = b->parent;
        --depthB;
    }

    while (a != b) {
        a = a->parent;
        b = b->parent;

        if (a == nullptr || b == nullptr) {
            return nullptr;
            // Khác cây (không xảy ra trong 1 cây hợp lệ).
        }
    }

    return a;
}

// Tính khoảng cách quan hệ giữa hai người.
//
// Công thức:
// distance = depthA + depthB - 2 * depthLca
//
// Trả về -1 nếu không có tổ tiên chung.
//
// Big-O:
// Thời gian O(h).
// Bộ nhớ O(1).

int FamilyDistance(
    Node* a,
    Node* b
) {
    Node* lca = FindLca(a, b);

    if (lca == nullptr) {
        return -1;
    }

    return GetDepth(a)
         + GetDepth(b)
         - 2 * GetDepth(lca);
}


// ===================== SẮP XẾP ANH EM =====================

// Nối node vào cuối một danh sách liên kết (head, tail).
static void AppendSibling(Node*& head, Node*& tail, Node* node) {
    node->nextSibling = nullptr;

    if (head == nullptr) {
        head = node;
        tail = node;
    } else {
        tail->nextSibling = node;
        tail = node;
    }
}

// Selection Sort trên danh sách liên kết anh em.
//
// Chỉ sắp xếp CON RUỘT (isInLaw = false):
// - Vợ/chồng nhập gia được tách ra trước, sắp xếp xong thì mỗi người được
//   đặt NGAY SAU người phối ngẫu của mình (ghép bằng spouseId).
// - Vợ/chồng không tìm thấy người phối ngẫu được nối vào cuối,
//   nên không ai bị xen lẫn giữa các con ruột và không ai bị mất.
//
// Sắp xếp ngày sinh giảm dần (birthDate dạng DD/MM/YYYY):
// - Người sinh muộn nhất đứng đầu.
// - Nếu cùng năm thì so tiếp tháng và ngày.
// - Nếu ngày sinh bằng nhau thì giữ thứ tự cũ.
//
// Không hoán đổi dữ liệu của node.
// Chỉ thay đổi con trỏ nextSibling.
// Người gọi phải cập nhật firstChild/lastChild của cha (xem SortChildrenOf).
//
// Big-O:
// Thời gian O(k^2), với k là số anh em.
// Bộ nhớ O(1).

Node* SortSiblingList(Node* head) {
    // Bước 1: tách con ruột và vợ/chồng thành 2 danh sách (giữ thứ tự cũ)
    Node* bloodHead = nullptr;
    Node* bloodTail = nullptr;
    Node* inLawHead = nullptr;
    Node* inLawTail = nullptr;

    while (head != nullptr) {
        Node* next = head->nextSibling;

        if (IsInLaw(head)) {
            AppendSibling(inLawHead, inLawTail, head);
        } else {
            AppendSibling(bloodHead, bloodTail, head);
        }

        head = next;
    }

    // Bước 2: Selection Sort danh sách con ruột (ngày sinh giảm dần)
    Node* sortedHead = nullptr;
    Node* sortedTail = nullptr;

    while (bloodHead != nullptr) {
        Node* maxNode = bloodHead;
        Node* maxPrev = nullptr;
        Node* prev = bloodHead;

        // Tìm node có ngày sinh lớn nhất
        for (
            Node* current = bloodHead->nextSibling;
            current != nullptr;
            prev = current, current = current->nextSibling
        ) {
            if (
                BirthKey(current->member.birthDate)
                > BirthKey(maxNode->member.birthDate)
            ) {
                maxNode = current;
                maxPrev = prev;
            }
        }

        // Tháo maxNode khỏi danh sách còn lại
        if (maxPrev == nullptr) {
            // maxNode đang là đầu danh sách
            bloodHead = maxNode->nextSibling;
        } else {
            // Nối vòng qua maxNode
            maxPrev->nextSibling = maxNode->nextSibling;
        }

        // Nối maxNode vào đuôi danh sách đã sắp xếp
        AppendSibling(sortedHead, sortedTail, maxNode);
    }

    // Bước 3: ghép lại - mỗi con ruột kéo theo vợ/chồng của mình đứng ngay sau
    Node* resultHead = nullptr;
    Node* resultTail = nullptr;

    Node* person = sortedHead;

    while (person != nullptr) {
        Node* nextPerson = person->nextSibling;

        AppendSibling(resultHead, resultTail, person);

        Node* prevInLaw = nullptr;
        Node* inLaw = inLawHead;

        while (inLaw != nullptr) {
            Node* nextInLaw = inLaw->nextSibling;

            if (AreSpouses(person, inLaw)) {
                // Tháo inLaw khỏi danh sách vợ/chồng rồi đặt sau person
                if (prevInLaw == nullptr) {
                    inLawHead = nextInLaw;
                } else {
                    prevInLaw->nextSibling = nextInLaw;
                }

                AppendSibling(resultHead, resultTail, inLaw);
            } else {
                prevInLaw = inLaw;
            }

            inLaw = nextInLaw;
        }

        person = nextPerson;
    }

    // Bước 4: vợ/chồng không có người phối ngẫu trong chuỗi -> nối cuối
    while (inLawHead != nullptr) {
        Node* next = inLawHead->nextSibling;

        AppendSibling(resultHead, resultTail, inLawHead);

        inLawHead = next;
    }

    return resultHead;
}


// Sắp xếp các con của một người.
// Cập nhật firstChild VÀ lastChild vì thứ tự thay đổi
// (nếu không cập nhật lastChild, lần thêm con sau sẽ nối sai chỗ).
//
// Big-O: O(k^2).

void SortChildrenOf(Node* parent) {
    if (parent == nullptr) {
        return;
    }

    parent->firstChild = SortSiblingList(
        parent->firstChild
    );

    Node* last = parent->firstChild;

    while (last != nullptr && last->nextSibling != nullptr) {
        last = last->nextSibling;
    }

    parent->lastChild = last;
}


// Sắp xếp đồng loạt mọi gia đình trong nhánh gốc tại node bằng đệ quy.
//
// Big-O:
// Tổng O(sum k_i^2), tối đa O(n^2).
// Bộ nhớ O(h).

void SortAllSiblings(Node* node) {
    if (node == nullptr) {
        return;
    }

    // Sắp xếp danh sách con của node hiện tại
    SortChildrenOf(node);

    // Tiếp tục sắp xếp các nhánh con
    for (
        Node* child = node->firstChild;
        child != nullptr;
        child = child->nextSibling
    ) {
        SortAllSiblings(child);
    }
}

// ===================== THỐNG KÊ =====================

// Duyệt mọi node của chuỗi bắt đầu từ head và toàn bộ con cháu.
// Mỗi người (kể cả vợ/chồng nhập gia) được đếm đúng 1 lần.
static void CollectStats(const Node* head, FamilyStats& stats) {
    for (const Node* node = head;
         node != nullptr;
         node = node->nextSibling) {

        ++stats.totalMembers;

        char gender = static_cast<char>(
            std::toupper(static_cast<unsigned char>(node->member.gender))
        );

        if (gender == 'M')
            ++stats.maleCount;
        else if (gender == 'F')
            ++stats.femaleCount;
        else
            ++stats.otherCount;

        CollectStats(node->firstChild, stats);
    }
}

// Đếm số người trong nhánh gốc tại node
// (node + vợ/chồng nhập gia của node + toàn bộ con cháu),
// KHÔNG tính anh em của node.
// Big-O: O(m * k) với m là kích thước nhánh, k là số anh em đông nhất.
int CountSubtree(const Node* node) {
    if (node == nullptr)
        return 0;

    int count = 1;

    if (!IsInLaw(node)) {
        // Cộng vợ/chồng của node (nằm trong chuỗi anh em của node)
        const Node* head = ChainHead(node, node);

        for (const Node* sibling = head;
             sibling != nullptr;
             sibling = sibling->nextSibling) {

            if (IsInLaw(sibling) && AreSpouses(node, sibling)) {
                ++count;
            }
        }
    }

    for (const Node* child = node->firstChild;
         child != nullptr;
         child = child->nextSibling) {

        if (IsPairedInLaw(node->firstChild, child)) {
            continue;   // đã được tính cùng người phối ngẫu
        }

        count += CountSubtree(child);
    }

    return count;
}

// Chiều cao cây = số thế hệ (cây chỉ có thủy tổ -> 1).
// Vợ/chồng nhập gia cùng thế hệ với người phối ngẫu nên KHÔNG tạo thêm thế hệ.
// Big-O: Thời gian O(n); Bộ nhớ O(h).
int TreeHeight(const Node* node) {
    if (node == nullptr)
        return 0;

    int maxChildHeight = 0;

    for (const Node* child = node->firstChild;
         child != nullptr;
         child = child->nextSibling) {

        if (IsInLaw(child)) {
            continue;
        }

        maxChildHeight = std::max(
            maxChildHeight,
            TreeHeight(child)
        );
    }

    return 1 + maxChildHeight;
}

// root là đầu chuỗi gốc: nếu có nhiều gốc thì tính tất cả,
// số thế hệ là chiều cao lớn nhất trong các gốc.
// Big-O: O(n).
FamilyStats ComputeStats(const Node* root) {
    FamilyStats stats;

    if (root == nullptr)
        return stats;

    CollectStats(root, stats);

    for (const Node* top = root;
         top != nullptr;
         top = top->nextSibling) {

        if (IsInLaw(top)) {
            continue;
        }

        stats.generationCount = std::max(
            stats.generationCount,
            TreeHeight(top)
        );
    }

    return stats;
}