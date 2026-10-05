#ifndef FAMILY_NODE_H
#define FAMILY_NODE_H

#include <string>
// Thông tin của một người trong gia phả; các ID dùng để định danh và liên kết.
struct Member {
    int id;                    // ID duy nhất của thành viên
    int spouseId;              // ID vợ/chồng; bằng 0 nếu chưa khai báo
    std::string name;          // họ tên đầy đủ
    char gender;               // giới tính: M-ale, F-emale , 0-biết
    std::string birthDate;     // ngày sinh theo định dạng dd-mm-yyyy
    std::string notes;         // ghi chú tự do về thành viên.
