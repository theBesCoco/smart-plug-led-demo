
const express = require("express");
const path = require("path");

const app = express();
const PORT = process.env.PORT || 3000;
const DEVICE_KEY = process.env.DEVICE_KEY || "change-this-device-key";

app.use(express.json());
app.use(express.static(path.join(__dirname, "public")));

// สถานะ Relay ทั้ง 4 ช่อง
const relays = Array.from({ length: 4 }, (_, i) => ({
  id: i + 1,
  on: false,
  autoOffAt: null
}));

let history = [];

function addLog(message) {
  history.unshift({
    time: new Date().toISOString(),
    message
  });
  history = history.slice(0, 30);
}

function applyState(channel, on, minutes = 0) {
  const relay = relays.find(r => r.id === channel);
  if (!relay) return false;

  relay.on = Boolean(on);

  if (relay.on && minutes > 0) {
    relay.autoOffAt = Date.now() + minutes * 60 * 1000;
    addLog(`Relay ${channel} เปิด ตั้งเวลาปิด ${minutes} นาที`);
  } else if (relay.on) {
    relay.autoOffAt = null;
    addLog(`Relay ${channel} เปิด`);
  } else {
    relay.autoOffAt = null;
    addLog(`Relay ${channel} ปิด`);
  }

  return true;
}

function checkAutoOff() {
  relays.forEach(relay => {
    if (relay.on && relay.autoOffAt &&
        Date.now() >= relay.autoOffAt) {
      relay.on = false;
      relay.autoOffAt = null;
      addLog(`Relay ${relay.id} ปิดอัตโนมัติเมื่อครบเวลา`);
    }
  });
}

function getState() {
  checkAutoOff();

  return {
    deviceName: "Smart Plug 4 Channel",
    channels: relays.map(relay => ({
      id: relay.id,
      on: relay.on,
      remainingSeconds: relay.on && relay.autoOffAt
        ? Math.max(0, Math.ceil(
            (relay.autoOffAt - Date.now()) / 1000
          ))
        : null
    })),
    history
  };
}

// เว็บไซต์อ่านสถานะ
app.get("/api/state", (req, res) => {
  res.json(getState());
});

// เว็บไซต์สั่งเปิด-ปิด Relay
app.post("/api/switch", (req, res) => {
  const { channel, on } = req.body;

  const minutes = Math.max(
    0,
    Math.min(1440, Number(req.body.minutes) || 0)
  );

  // รองรับเว็บไซต์เก่า: ไม่มี channel = สั่งทั้ง 4 ช่อง
  if (channel === undefined) {
    if (typeof on !== "boolean") {
      return res.status(400).json({ error: "Invalid on value" });
    }

    relays.forEach(relay => {
      applyState(relay.id, on, on ? minutes : 0);
    });

    return res.json(getState());
  }

  const id = Number(channel);

  if (!Number.isInteger(id) || id < 1 || id > 4 ||
      typeof on !== "boolean") {
    return res.status(400).json({
      error: "Invalid channel or on value"
    });
  }

  applyState(id, on, on ? minutes : 0);

  res.json(getState());
});

// ESP32 อ่านสถานะ Relay ทั้ง 4 ช่อง
// ตัวอย่าง 1010 = ช่อง 1 และ 3 เปิด
app.get("/api/esp", (req, res) => {
  if (req.get("x-device-key") !== DEVICE_KEY) {
    return res.status(401).type("text/plain").send("unauthorized");
  }

  checkAutoOff();

  const output = relays
    .map(relay => relay.on ? "1" : "0")
    .join("");

  res.type("text/plain").send(output);
});

// ตรวจสอบ Server
app.get("/ping", (req, res) => {
  res.type("text/plain").send("ok");
});

app.listen(PORT, () => {
  console.log(`Smart Plug 4 Channel running on port ${PORT}`);
});
