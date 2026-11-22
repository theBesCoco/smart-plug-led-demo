# Smart Plug LED Test

เว็บควบคุมปลั๊กไฟจำลอง โดยทดสอบเอาต์พุตด้วย LED แรงดันต่ำและ ESP32

## ภาพรวมการทำงาน

เว็บไซต์บน Render → Node.js/Express → ESP32 เรียก `GET /api/esp` ทุก 2 วินาที → GPIO ของ ESP32 → LED

- `POST /api/switch` สั่งเปิด/ปิดจากหน้าเว็บ
- `GET /api/state` อ่านสถานะสำหรับหน้าเว็บ
- `GET /api/esp` ส่ง `1` หรือ `0` ให้ ESP32 (ต้องส่ง header `x-device-key`)
- `GET /ping` health check

## 1) ทดสอบ Server ในเครื่อง

ต้องติดตั้ง Node.js 18 ขึ้นไป

```bash
npm install
```

กำหนดคีย์อุปกรณ์ (ใช้คีย์เดียวกันใน ESP32):

Windows PowerShell:
```powershell
$env:DEVICE_KEY="ใส่คีย์ยาวที่เดายาก"
npm start
```

macOS/Linux:
```bash
DEVICE_KEY="ใส่คีย์ยาวที่เดายาก" npm start
```

เปิด `http://localhost:3000`

## 2) Deploy บน Render

1. อัปโหลดไฟล์ทั้งหมดในโฟลเดอร์นี้ขึ้น GitHub repository
2. ใน Render สร้าง **New → Web Service** แล้วเชื่อม repository
3. Runtime: Node
4. Build Command: `npm install`
5. Start Command: `npm start`
6. เพิ่ม Environment Variable ชื่อ `DEVICE_KEY` และกำหนดค่าเป็นคีย์ยาวที่เดายาก
7. Deploy แล้วจด URL ของบริการ เช่น `https://ชื่อบริการ.onrender.com`

## 3) ตั้งค่า ESP32

เปิดไฟล์ `esp32_smart_plug_led.ino` ใน Arduino IDE และแก้ไข:
- `WIFI_SSID`
- `WIFI_PASSWORD`
- `SERVER_URL` ให้เป็น URL Render ของตนเอง ตามด้วย `/api/esp`
- `DEVICE_KEY` ให้ตรงกับค่า Environment Variable `DEVICE_KEY` ใน Render

เลือกบอร์ด ESP32 และพอร์ตให้ถูกต้อง แล้ว Upload

## 4) ต่อ LED สำหรับทดสอบ

- GPIO 2 → ตัวต้านทาน 220–330 โอห์ม → ขั้วบวก LED (ขายาว)
- ขั้วลบ LED (ขาสั้น) → GND ของ ESP32

หากบอร์ดของคุณใช้ GPIO 2 ไม่ได้ ให้เลือก GPIO ที่รองรับ output และแก้ `LED_PIN` ให้ตรงกัน

## หมายเหตุสำคัญ

- โปรเจกต์นี้เป็น **การจำลองปลั๊กไฟด้วย LED แรงดันต่ำเท่านั้น**
- ห้ามต่อไฟบ้าน 220V เข้ากับ LED, ESP32 หรือเบรดบอร์ด
- ยังไม่มีรีเลย์ควบคุมเต้ารับจริง และยังไม่ใช่อุปกรณ์ปลั๊กไฟที่พร้อมใช้งานจริง
- สถานะและประวัติใน Server ตัวอย่างนี้เก็บในหน่วยความจำ จึงอาจหายเมื่อ Render รีสตาร์ตหรือเริ่ม instance ใหม่
- สำหรับใช้งานจริงต้องเพิ่มการยืนยันตัวตน/ความปลอดภัย และให้นักไฟฟ้าที่มีคุณสมบัติเหมาะสมออกแบบวงจรไฟบ้าน กล่องปิด ฉนวน ฟิวส์ และอุปกรณ์ป้องกัน
