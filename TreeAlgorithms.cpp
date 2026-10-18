#include <algorithm>
#include <cctype>

// ===================== HÀNG ĐỢI TỰ CÀI ĐẶT =====================
// (Không dùng std::queue)

struct QueueNode {
    FamilyNode* treeNode;
    QueueNode* next;
};

struct NodeQueue {
    QueueNode* head;   // Đầu hàng: nơi lấy ra
    QueueNode* tail;   // Cuối hàng: nơi thêm vào
    int size;
};

static void QueueInit(NodeQueue& queue) {
    queue.head = nullptr;
    queue.tail = nullptr;
    queue.size = 0;
}

static void Enqueue(NodeQueue& queue, FamilyNode* treeNode) {
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

static FamilyNode* Dequeue(NodeQueue& queue) {
    QueueNode* item = queue.head;
    FamilyNode* treeNode = item->treeNode;

    queue.head = item->next;

    if (queue.head == nullptr) {
        // Vừa lấy phần tử cuối -> hàng rỗng
        queue.tail = nullptr;
    }

    delete item;       // Giải phóng ô hàng đợi
    --queue.size;

    return treeNode;
}

static std::string ToLower(const std::string& text) {
    std::string result = text;

    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<char>(
            std::tolower(static_cast<unsigned char>(result[i]))
        );
    }

    return result;
}