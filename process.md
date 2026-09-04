Ok, em đã hiểu bài toán. Dưới đây là phân tích và giải pháp chi tiết.

---

## 🔍 Phân tích vấn đề hiện tại

Hiện tại, code backend **`smart-parking-server/services/user_service.py`** có 4 chỗ gọi SMTP để gửi email:

| #   | Chức năng           | Gửi tới      |
| --- | ------------------- | ------------ |
| 1   | `check_in()`        | User + Admin |
| 2   | `check_out()`       | User + Admin |
| 3   | `send_fire_alert()` | Tất cả user  |
| 4   | `send_gas_alert()`  | Tất cả user  |

Vấn đề: Server deploy (ví dụ Render) không có SMTP, không thể gọi `smtplib` → toàn bộ thông báo bị lỗi im lặng (vì exception bị bỏ qua ở `except`).

---

## ✅ Giải pháp đề xuất: **In-app Notification lưu MongoDB**

Thay vì gửi email, ta **ghi thông báo vào collection `notifications`** trong MongoDB — sau đó gọi API từ Frontend để hiển thị.

### Kiến trúc tổng thể

```
┌─────────────┐     ┌───────────────────┐     ┌──────────────┐
│  Frontend    │────▶│  Backend API      │────▶│  MongoDB     │
│  (React)     │◀────│  (FastAPI)        │◀────│  notifs coll │
└─────────────┘     └───────────────────┘     └──────────────┘
             GET /notifications/user/{id}
             PUT /notifications/{id}/read
```

### Backend: Cấu trúc file mới

#### 1. `models/notification_model.py`

```python
class NotificationModel:
    id: str
    user_id: str | None    # None = gửi broadcast cho tất cả
    type: str              # "fire" | "gas" | "checkin" | "checkout" | "info"
    title: str
    message: str
    created_at: datetime
    is_read: bool
```

#### 2. `repositories/notification_repository.py`

- `create(data)` — insert one document
- `get_all(limit)` — lấy toàn bộ (admin xem)
- `get_by_user_id(user_id, limit)` — lấy của user (`user_id` = user đó hoặc `None` broadcast)
- `mark_as_read(id)` — đánh dấu đã đọc
- `mark_all_read(user_id)` — đánh dấu tất cả chưa đọc của user
- `count_unread(user_id)` — đếm số chưa đọc

#### 3. `services/notification_service.py`

- `notify_all(type, title, message)` — tạo broadcast
- `notify_user(user_id, type, title, message)` — tạo cho 1 user
- Các phương thức delegate xuống repository

#### 4. `schemas/notification_schema.py`

- `NotificationResponse`
- `UnreadCountResponse`

#### 5. `controllers/notification_controller.py`

- 5 endpoints handler

#### 6. `routes/notification_route.py` — API endpoints:

| Method | Path                                | Mục đích                  |
| ------ | ----------------------------------- | ------------------------- |
| `GET`  | `/notifications/`                   | Admin xem tất cả          |
| `GET`  | `/notifications/user/{user_id}`     | User xem thông báo của họ |
| `PUT`  | `/notifications/{id}/read`          | Đánh dấu 1 cái đã đọc     |
| `PUT`  | `/notifications/read-all/{user_id}` | Đánh dấu tất cả đã đọc    |
| `GET`  | `/notifications/unread/{user_id}`   | Đếm chưa đọc              |

### Backend: Sửa file hiện tại

**`services/user_service.py`** — thay 4 chỗ gọi `send_email()` thành gọi `NotificationService`:

- `check_in()`: `notify_user()` cho user check-in + `notify_all()` broadcast cho admin
- `check_out()`: tương tự
- `send_fire_alert()`: `notify_all(type="fire", ...)`
- `send_gas_alert()`: `notify_all(type="gas", ...)`
- **Xoá** các import `smtplib`, `email.mime.*`, `asyncio`, biến `SMTP_*`, method `send_email()` và `_send_smtp()`

**`main.py`** — thêm `notification_route` vào `include_router`

### Frontend: Mới

#### `src/app/notifications/page.js`

- Component `NotificationViewer` hiển thị danh sách thông báo theo user
- Đánh dấu badge "unread count"
- Mark as read / mark all read

#### Sửa `simulator/page.js`

- Text: Email notifications → In-app Notifications

---

## 📊 So sánh với giải pháp thay thế

| Giải pháp                    | Ưu                                                       | Nhược                          |
| ---------------------------- | -------------------------------------------------------- | ------------------------------ |
| **SMS (Twilio)**             | Real-time, không cần app                                 | Tốn phí, cần signup Twilio     |
| **Firebase Cloud Messaging** | Push notification real-time                              | Cần cấu hình FCM, phức tạp     |
| **✅ MongoDB Notification**  | Dễ nhất, không phụ thuộc bên thứ 3, dùng chung DB sẵn có | User phải vào web mới xem được |
| **WebSocket / SSE**          | Real-time khi đang ở web                                 | Phức tạp, cần quản lý kết nối  |

---

## 🚀 Kế hoạch triển khai

Nếu chuyển qua **Act mode**, em sẽ:

1. ✅ Tạo 6 file backend mới (model, repository, service, schema, controller, route)
2. ✅ Sửa `user_service.py` thay toàn bộ email → notification
3. ✅ Sửa `main.py` thêm router
4. ✅ Tạo trang Frontend xem thông báo
5. ✅ Sửa text trên simulator page
6. ✅ Kiểm tra server chạy được

---

Anh/chị muốn chuyển qua chế độ **Act** để em tiến hành code luôn không ạ?
