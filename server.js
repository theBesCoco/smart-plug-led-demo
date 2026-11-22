const express = require("express");
const path = require("path");

const app = express();
const PORT = process.env.PORT || 3000;
const DEVICE_KEY = process.env.DEVICE_KEY || "change-this-device-key";

app.use(express.json());
app.use(express.static(path.join(__dirname, "public")));

let isOn = false;
let autoOffAt = null;
let history = [];

function addLog(message) {
  history.unshift({ time: new Date().toISOString(), message });
  history = history.slice(0, 20);
}

function applyState(on, minutes = 0) {
  isOn = Boolean(on);
  if (isOn && Number(minutes) > 0) {
    autoOffAt = Date.now() + Number(minutes) * 60 * 1000;
    addLog(`เปิดสวิตช์ ตั้งเวลาปิด ${Number(minutes)} นาที`);
  } else if (isOn) {
    autoOffAt = null;
    addLog("เปิดสวิตช์");
  } else {
    autoOffAt = null;
    addLog("ปิดสวิตช์");
  }
}

function checkAutoOff() {
  if (isOn && autoOffAt && Date.now() >= autoOffAt) {
    isOn = false;
    autoOffAt = null;
    addLog("ปิดอัตโนมัติเมื่อครบเวลา");
  }
}

function getState() {
  checkAutoOff();
  return {
    deviceName: "Smart Plug (LED Test)",
    on: isOn,
    remainingSeconds: isOn && autoOffAt
      ? Math.max(0, Math.ceil((autoOffAt - Date.now()) / 1000))
      : null,
    esp32Mode: "ESP32 polls /api/esp",
    history
  };
}

// Website reads current state
app.get("/api/state", (req, res) => {
  res.json(getState());
});

// Website turns the demo output on/off
app.post("/api/switch", (req, res) => {
  const on = Boolean(req.body.on);
  const minutes = Math.max(0, Math.min(1440, Number(req.body.minutes) || 0));
  applyState(on, minutes);
  res.json(getState());
});

// ESP32 checks this endpoint. It returns plain text: 1 = LED ON, 0 = LED OFF.
app.get("/api/esp", (req, res) => {
  if (req.get("x-device-key") !== DEVICE_KEY) {
    return res.status(401).type("text/plain").send("unauthorized");
  }
  checkAutoOff();
  res.type("text/plain").send(isOn ? "1" : "0");
});

// Basic health check for hosting services
app.get("/ping", (req, res) => res.type("text/plain").send("ok"));

app.listen(PORT, () => {
  console.log(`Smart Plug LED demo server listening on port ${PORT}`);
});
