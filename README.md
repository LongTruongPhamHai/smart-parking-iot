# 🚗 Hệ thống Quản lý Bãi đỗ xe thông minh (Smart Parking System)

> **Bài tập lớn môn:** Kết nối vạn vật và ứng dụng (IoT)  
> **Trường:** Đại học Thủy Lợi (TLU)  
> **Mô tả chung:** Đây là dự án kết hợp giữa thiết bị phần cứng (IoT) và hệ thống phần mềm quản lý để tạo thành một giải pháp bãi đỗ xe thông minh. _(Hiện tại, do điều kiện thực tế, phần cứng đã được thay thế bằng các trình mô phỏng - Simulator trên phần mềm)._

---

## 📌 Tổng quan kiến trúc hệ thống

Hệ thống được chia làm 3 phân hệ chính, mỗi phân hệ được đặt trong một thư mục riêng biệt:

1. **[Phân hệ Server (Backend)](./smart-parking-server)**: Đóng vai trò làm bộ não trung tâm xử lý logic nghiệp vụ, quản lý trạng thái bãi đỗ, tính toán chi phí và giao tiếp với cơ sở dữ liệu.
2. **[Phân hệ Client (Frontend)](./smart-parking-client)**: Giao diện tương tác người dùng, cung cấp Dashboard cho Quản trị viên (Admin) và Khách hàng (Customer), cùng với một giao diện giả lập (Simulator).
3. **[Phân hệ IoT (Hardware/Simulator)](#)**: Mô phỏng hoặc kết nối với các thiết bị phần cứng IoT (Cảm biến nhận diện xe, đóng mở cổng barie). _(Sử dụng Simulator tích hợp trong Frontend)_

---

## ⚙️ Công nghệ sử dụng

- **Backend:** Python 3.10+, FastAPI, MongoDB (Motor Async Driver)
- **Frontend:** Next.js 16 (React 19), TailwindCSS, Framer Motion
- **Database:** MongoDB (NoSQL)
- **Hardware/IoT:** Mô phỏng qua giao diện Simulator (HTTP API)

---

## 🌟 Chức năng chính

### 👥 Dành cho Customer (Khách hàng)

- **Xem trạng thái bãi đỗ:** Hiển thị các slot đỗ xe Available (Trống) hoặc Occupied (Đã có xe)
- **Quản lý tài khoản:** Chỉnh sửa thông tin cá nhân, đổi mật khẩu
- **Nạp tiền (Top-up):** Nạp tiền vào số dư tài khoản để thanh toán phí đỗ xe
- **Xem lịch sử gửi xe:**
  - Bảng Invoice hiển thị lịch sử check-in/check-out
  - **Sort theo thời gian:** Newest First / Oldest First
  - **Phân trang:** 15 invoices mỗi trang
- **Simulator:** Mô phỏng quẹt thẻ xe vào (Check-in) và xe ra (Check-out)

### 🔐 Dành cho Admin (Quản trị viên)

- **Dashboard tổng quan:** Hiển thị thống kê tổng số slot, available, occupied
- **Quản lý Invoices:**
  - Xem toàn bộ hóa đơn của hệ thống
  - Sort mới → cũ (mặc định)
  - Phân trang 15 items/page
- **Activity Logs:**
  - Xem log hoạt động gửi tới Admin (check-in, check-out, fire/gas alerts)
  - Phân trang 15 items/page
- **Quản lý Users:**
  - Thêm, sửa, xóa tài khoản người dùng
  - Phân quyền Admin/Customer
  - Tìm kiếm user theo tên/email
  - Phân trang 15 users/page

### 🔔 Hệ thống Thông báo

- **In-app Notifications:** Lưu thông báo trong database MongoDB
- **Admin Activity Logs:** Admin xem được các event quan trọng (xe vào/ra, cảnh báo cháy/khí gas)
- ~~Email notifications~~ (Đã loại bỏ - chỉ dùng notification nội bộ)

### 💰 Tự động tính phí

- Tính toán chi phí dựa trên thời gian thực gửi xe
- Tự động trừ tiền từ số dư khi Check-out
- Kiểm tra số dư trước khi cho phép Check-out

---

## 🚀 Hướng dẫn khởi chạy toàn hệ thống

### Yêu cầu hệ thống

- **Node.js** 18+ và npm
- **Python** 3.10+
- **MongoDB** (local hoặc MongoDB Atlas)

### Khởi động hệ thống

Để chạy dự án, bạn cần khởi động cả 2 phân hệ **Server** và **Client** ở các Terminal khác nhau:

👉 [Hướng dẫn chạy Backend (Server)](./smart-parking-server/README.md)  
👉 [Hướng dẫn chạy Frontend (Client)](./smart-parking-client/README.md)

### Truy cập hệ thống

- **Frontend:** http://localhost:3000
- **Backend API:** http://127.0.0.1:8000
- **API Docs:** http://127.0.0.1:8000/docs

---

## 📁 Cấu trúc thư mục

```
smart-parking-iot/
├── smart-parking-client/    # Frontend (Next.js)
│   ├── src/
│   │   ├── app/             # Pages (App Router)
│   │   │   ├── page.js      # Trang chủ - hiển thị slot status
│   │   │   ├── admin/       # Admin dashboard
│   │   │   ├── customer/    # Customer dashboard
│   │   │   ├── simulator/   # IoT Simulator
│   │   │   ├── signin/      # Đăng nhập
│   │   │   └── signup/      # Đăng ký
│   │   └── components/      # UI components
│   └── package.json
│
├── smart-parking-server/    # Backend (FastAPI)
│   ├── controllers/         # API endpoints
│   ├── services/            # Business logic
│   ├── repositories/        # Database operations
│   ├── models/              # Data models
│   ├── schemas/             # Request/Response schemas
│   ├── routes/              # API routers
│   ├── main.py              # FastAPI app entry
│   └── requirements.txt
│
└── README.md               # File này
```

---

## 🔑 Tài khoản mặc định (Test)

**Admin:**

- Phone: `011`
- Password: `011`

**Customer:**

- Phone: `012`
- Password: `012`

_(Lưu ý: Hãy tạo tài khoản mới hoặc đổi mật khẩu khi triển khai thực tế)_

---

## 📝 Lưu ý quan trọng

- Hệ thống sử dụng **MongoDB** để lưu trữ dữ liệu. Đảm bảo MongoDB đang chạy trước khi khởi động Backend.
- Phần cứng IoT đã được thay thế bằng **Simulator** (giao diện web) để dễ dàng demo và test.
- Hệ thống không sử dụng WebSocket - dữ liệu được cập nhật qua HTTP polling hoặc manual refresh.
- Notification được lưu trong database, không gửi qua email hay push notification.

---

## 👨‍💻 Nhóm phát triển

Dự án được thực hiện bởi sinh viên Đại học Thủy Lợi - Môn Kết nối vạn vật và ứng dụng (IoT).
