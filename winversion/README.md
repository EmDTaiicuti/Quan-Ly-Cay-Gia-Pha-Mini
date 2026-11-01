# Quản lý Cây Gia Phả — phiên bản Windows (Win32 API thuần)

Ứng dụng desktop C++17, giao diện viết hoàn toàn bằng **Win32 API có sẵn trong Windows**, không dùng thư viện bên thứ ba
(không Raylib, Qt, wxWidgets...). Phần lõi (cây gia phả, CSV, tài khoản) giữ cấu trúc dữ liệu của bản Raylib.

## Thành phần Windows được dùng

| Thành phần | Header / thư viện link | Dùng cho |
|---|---|---|
| User32 (Win32 API gốc) | `<windows.h>`, `-luser32` | Cửa sổ, menu, phím tắt, nút (`BUTTON`), ô nhập (`EDIT`), nhãn (`STATIC`), hộp chọn (`COMBOBOX`), MessageBox, vòng lặp message |
| GDI | `-lgdi32` | Tự vẽ phả đồ (thẻ bo góc, đường nối), vẽ ảnh chân dung, double-buffer |
| GDI+ | `<gdiplus.h>`, `-lgdiplus` | Màn hình đăng nhập: nền gradient, bo góc và chữ khử răng cưa |
| Common Controls v6 | `<commctrl.h>`, `-lcomctl32` | Bảng `ListView`, ô chọn ngày `DateTimePicker`, thanh trạng thái `StatusBar` |
| Common Dialogs | `<commdlg.h>`, `-lcomdlg32` | Hộp thoại chọn file (nhập/xuất CSV, chọn ảnh chân dung) |
| OLE | `<olectl.h>`, `-lole32 -loleaut32 -luuid` | Đọc ảnh JPG/BMP/GIF (`OleLoadPicture`) |
| Shell | `<shellapi.h>`, `-lshell32` | Mở ảnh bằng trình xem ảnh của Windows |

`res/app.manifest` bật **Common Controls phiên bản 6** (theme Windows hiện đại). Bỏ manifest thì control quay về kiểu Windows 98.

Không dùng file `.rc` để vẽ dialog: lớp `FormDialog` trong `src/gui/Controls.cpp` tự tạo control bằng `CreateWindowExW`,
mỗi form chỉ khai báo danh sách trường. `ListDialog` dùng cho các bảng (tài khoản, thành tích, quy ước, người liên quan).

## Cấu trúc thư mục

```
winversion/
├─ build.bat              Build bằng MinGW-w64 (g++ + windres)
├─ res/                   app.manifest, app.rc, app.ico
├─ data/                  CSV (UTF-8 BOM, ngày DD/MM/YYYY), profiles/, photos/
├─ src/main.cpp           wWinMain: InitCommonControlsEx, COM, tìm thư mục data
├─ src/core/              Lõi không phụ thuộc giao diện
│   FamilyNode.h          Node First Child – Next Sibling
│   TreeCore              Thêm gốc/con/vợ-chồng, sửa, xóa nhánh, kiểm tra ngày
│   TreeAlgorithms        DFS, tìm tên không dấu, LCA, khoảng cách đời, Quick Sort anh chị em, thống kê
│   FileManager, CsvUtil  Đọc/ghi CSV (trường có dấu phẩy, ngoặc kép, xuống dòng)
│   Auth, Registry, Security   Tài khoản (băm + muối), thành tích, quy ước, mã bảo mật
├─ src/gui/
│   LoginWindow           Màn hình đăng nhập / đăng ký / mã bảo mật (GDI+)
│   Controls              Tiện ích control, ListView, FormDialog, ListDialog, hộp thoại file, đọc ảnh
│   TreeCanvas            Control tự vẽ phả đồ: thu phóng, kéo, chọn, thu gọn nhánh
│   MainWindow            Cửa sổ chính, menu, danh sách, hồ sơ, thanh trạng thái
│   AppDialogs            Đăng nhập/đăng ký, thêm/sửa/xóa thành viên, thống kê, quan hệ, thành tích, quy ước, tài khoản
└─ tests/                 test_core.cpp + run_tests.bat (kiểm thử phần lõi)
```

## Build và chạy

Cần MinGW-w64 có `g++` và `windres` trong PATH (ví dụ MSYS2: `C:\msys64\ucrt64\bin`).

```bat
cd /d "C:\Users\LENOVO\Downloads\FamilyTreeManagement (2)\winversion"
build.bat
FamilyTree.exe
```

Kiểm thử phần lõi: `tests\run_tests.bat`.

Dữ liệu nằm trong `data\` cạnh file `.exe`. Dữ liệu trong thư mục này được chép từ bản Raylib nên đăng nhập bằng tài khoản
Admin cũ sẽ thấy cùng phả đồ (`data\profiles\u_...`). Nếu `data\accounts.csv` chưa có Admin, chương trình mở màn hình
**Khởi tạo trưởng tộc**.

## Giao diện

- **Màn hình đăng nhập** (`LoginWindow`): nửa trái giới thiệu "gia phả nhà mình" trên nền xanh rêu, nửa phải thẻ đăng nhập.
  Cùng một cửa sổ chuyển giữa Đăng nhập, Tạo tài khoản mới, Khởi tạo trưởng tộc (lần đầu) và Nhập mã bảo mật;
  lỗi hiện ngay dưới ô nhập. Bố cục tự co khi cửa sổ nhỏ.
- **Bảng màu chung** (`Controls.h`, `THEME_*`): xanh rêu `#1F4D34`, nền kem `#F4F3EE`, thẻ `#FDFDFA`, vàng đồng `#C9A048`,
  đất nung `#B06252`. Cửa sổ chính có dải xanh chứa thanh tìm kiếm, dòng đang chọn trong bảng tô xanh rêu,
  thẻ phả đồ: nam viền xanh rêu, nữ viền đất nung, người đang chọn viền vàng đồng.

## Chức năng

- Đăng nhập / đăng ký / đổi mật khẩu; Admin quản lý tài khoản (tạo, khóa/mở khóa, đặt lại mật khẩu, liên kết thành viên).
- Mã bảo mật phả đồ (chỉ lưu băm), tối đa 5 lần nhập sai.
- Phả đồ tự vẽ: cuộn chuột thu phóng, kéo nền di chuyển, nút tròn thu gọn nhánh, giới hạn số đời hiển thị,
  xem riêng đời sau của một người, tô tổ tiên / hậu duệ, menu chuột phải.
- Danh sách thành viên: lọc theo tên không dấu hoặc ID, theo giới tính, sắp xếp theo cột.
- Hồ sơ: tuổi / hưởng thọ, cha mẹ, vợ/chồng, con, anh chị em, tiểu sử nhiều dòng, thành tích, ảnh chân dung.
- Thêm gốc / con / vợ-chồng, sửa, xóa nhánh với thông báo lỗi cụ thể (ngày sai, con sinh trước cha mẹ, ID trùng...).
- Thống kê (theo giới, còn sống/đã mất, tuổi thọ trung bình, theo năm), tổ tiên chung & mô tả quan hệ.
- Thành tích và quy ước gia tộc; nhập / xuất phả đồ CSV; hỏi lưu khi đóng nếu còn thay đổi.

Phím tắt: `Ctrl+S` lưu · `Ctrl+F` tìm · `Ctrl+N` thêm con · `Ctrl+M` thêm vợ/chồng · `F2` sửa · `F5` vừa khung ·
`Ctrl +/−` thu phóng · `F1` hướng dẫn.

## Các lỗi đã sửa so với bản Raylib

1. **Xóa dâu/rể làm mất cả nhánh**: `DeleteBranch` trên con dâu xóa luôn người chồng huyết thống và toàn bộ con cháu.
   Giờ chỉ xóa người huyết thống mới kéo theo vợ/chồng.
2. **Dữ liệu cũ đánh dấu sai dâu/rể**: file 7 cột gán `isInLaw = spouseId > 0` nên cả hai vợ chồng đều thành dâu/rể
   (Nguyễn Văn Minh hiện "Thành viên kết hôn vào dòng họ"). Khi nạp, `RepairFamilyLinks` sửa lại cờ và xóa `spouseId` treo.
3. **Dâu/rể lớn tuổi hơn cha mẹ chồng bị mất khi nạp file** và không sửa hồ sơ được (bị so ngày sinh với "cha").
4. **Sắp xếp anh chị em làm hỏng danh sách**: không cập nhật `lastChild`, lần thêm con sau bị nối sai; thứ tự cũ là út
   trước. Giờ anh/chị cả trước và `SortChildrenByBirthDate` cập nhật con trỏ cuối.
5. **CSV có xuống dòng** (tiểu sử, mô tả nhiều dòng) làm vỡ file khi đọc lại. Giờ đọc được trường nhiều dòng trong ngoặc kép.
6. **User đăng ký chỉ thấy phả đồ trống** (mỗi tài khoản một hồ sơ riêng nhưng User không được sửa). Giờ User xem phả đồ
   do Admin quản lý.
7. Thành tích của người đã bị xóa / không còn sau khi nhập CSV được dọn; tổ tiên chung tính dâu/rể theo vợ/chồng;
   đường dẫn file Unicode (thư mục có dấu tiếng Việt) mở được; các dòng CSV lỗi được báo số lượng thay vì im lặng bỏ qua.
