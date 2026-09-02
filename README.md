# 🚗 Hệ thống Quản lý Bãi đỗ xe thông minh (Smart Parking System)

> **Bài tập lớn môn:** Kết nối vạn vật và ứng dụng (IoT)  
> **Trường:** Đại học Thủy Lợi (TLU)  
> **Mô tả chung:** Đây là dự án kết hợp giữa thiết bị phần cứng (IOT) và hệ thống phần mềm quản lý để tạo thành một giải pháp bãi đỗ xe thông minh. *(Hiện tại, do điều kiện thực tế, phần cứng đã được thay thế bằng các trình mô phỏng - Simulator trên phần mềm).*

---

## 📌 Tổng quan kiến trúc hệ thống

Hệ thống được chia làm 3 phân hệ chính, mỗi phân hệ được đặt trong một thư mục riêng biệt:

1. **[Phân hệ Server (Backend)](./smart-parking-server)**: Đóng vai trò làm bộ não trung tâm xử lý logic nghiệp vụ, quản lý trạng thái bãi đỗ, tính toán chi phí và giao tiếp với cơ sở dữ liệu.
2. **[Phân hệ Client (Frontend)](./smart-parking-client)**: Giao diện tương tác người dùng, cung cấp Dashboard cho Quản trị viên (Admin) và Khách hàng (Customer), cùng với một giao diện giả lập (Simulator).
3. **[Phân hệ IoT (Hardware/Simulator)](#)**: Mô phỏng hoặc kết nối với các thiết bị phần cứng IoT (Cảm biến nhận diện xe, đóng mở cổng barie). *(Thư mục hiện tại)*

---

## ⚙️ Công nghệ sử dụng

- **Backend:** Python, FastAPI, MongoDB (Motor Async).
- **Frontend:** Next.js (React), TailwindCSS, Framer Motion.
- **Hardware/IoT:** (Mô phỏng) Giao tiếp qua API RESTful / HTTP Requests.

---

## 🌟 Chức năng nổi bật

- **Quản lý bãi đỗ xe theo thời gian thực:** Cập nhật ngay lập tức trạng thái `Available` (Trống) hoặc `Occupied` (Đã có xe).
- **Tính toán chi phí tự động (Billing):** Tự động tính tiền dựa trên thời gian thực gửi xe và tự động trừ vào số dư tài khoản khách hàng khi Check-out.
- **Phân quyền người dùng:** 
  - *Admin:* Quản lý toàn bộ danh sách bãi đỗ, xem doanh thu, thêm sửa xoá tài khoản người dùng.
  - *Customer:* Tự quản lý thông tin, đổi mật khẩu, nạp tiền (Top up) và xem lịch sử đỗ xe (Invoices).
- **Hệ thống Mô phỏng (Simulator):** Cho phép kiểm thử việc xe ra/vào (Check-in / Check-out) mà không cần thiết bị phần cứng thực.

---

## 🚀 Hướng dẫn khởi chạy toàn hệ thống

Để chạy dự án, bạn cần khởi động cả 2 phân hệ **Server** và **Client** ở các Terminal khác nhau. Hãy tham khảo hướng dẫn chi tiết trong từng thư mục:

👉 [Hướng dẫn chạy Backend (Server)](./smart-parking-server/README.md)  
👉 [Hướng dẫn chạy Frontend (Client)](./smart-parking-client/README.md)
