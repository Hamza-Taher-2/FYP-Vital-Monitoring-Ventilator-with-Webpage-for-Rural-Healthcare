
#include <WiFi.h>

// ---------- HTML (mockup only) ----------
static const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>Dr Akhter’s Monitor Log</title>
  <style>
    :root{
      --bg:#5a1140;
      --card:#ffffff;
      --text:#111;
      --header:#ffffff;
      --accent:#5a1140;
      --muted:#666;
      --shadow: 0 8px 24px rgba(0,0,0,.18);
      --radius: 26px;
    }
    *{box-sizing:border-box}
    body{
      margin:0;
      font-family: Arial, Helvetica, sans-serif;
      background: var(--bg);
      color:#fff;
    }
    .topbar{
      background: var(--header);
      color:#000;
      padding: 18px 22px;
      font-size: 42px;
      font-weight: 700;
      border-bottom-left-radius: var(--radius);
      border-bottom-right-radius: var(--radius);
      display:flex;
      align-items:center;
      justify-content:space-between;
    }
    .icons{
      display:flex; gap:18px; align-items:center;
    }
    .icon{
      width:42px;height:42px;border-radius:50%;
      display:grid;place-items:center;
      color:#000;
      border: 2px solid rgba(0,0,0,.12);
      font-size:20px;
      user-select:none;
    }

    .wrap{
      padding: 22px;
      display:grid;
      grid-template-columns: 1fr 1fr;
      gap: 18px;
      align-items: stretch;                 /* REPLACE start -> stretch */
      min-height: calc(100vh - 110px); 
    }

    .leftCol{
      display:flex;
      flex-direction:column;
      gap:18px;
      height: 100%; 
    }

    .card{
      background: var(--card);
      color: var(--text);
      border-radius: var(--radius);
      box-shadow: var(--shadow);
      padding: 18px 22px;
    }
    .card h2{
      margin: 0 0 12px 0;
      font-size: 26px;
      text-align:center;
    }

    .infoGrid{
      display:grid;
      grid-template-columns: 1fr 1fr;
      gap: 6px 20px;
      font-size: 23px;
      line-height: 1.35;
      overflow-y: auto;
    }
    .label{ color:#000; }
    .value{ color:#000; text-align:right; font-weight: 400; }

    .sensorGrid{
      display:grid;
      grid-template-columns: 1fr 1fr;
      gap: 8px 24px;
      font-size: 23px;
      line-height: 1.35;
    }

    .rightCol{
      display:flex;
      flex-direction:column;
      gap:18px;
      height: 100%;
    }


    /* Make the split line level: top/bottom cards each take half the column height */
    .leftCol .card, .rightCol .chartBox{
      flex: 1;
      min-height: 0;
    }


    .chartTitle{
      text-align:center;
      font-size: 26px;
      color: #333;
      margin-bottom: 8px;
      font-weight: 700;
    }
    .chartBox{
      background:#fff;
      flex: 1;  
      border-radius: var(--radius);
      padding: 18px 22px;   /* SAME as .card */
      box-shadow: var(--shadow);
      color:#000;
      display: flex;          /* ADD */
      flex-direction: column; /* ADD */
    }

    canvas{
      flex: 1;
      min-height: 0;    
      width: 100%;
      height: auto;
      border: 1px solid rgba(0,0,0,.12);
      border-radius: var(--radius);
      display: block;
    }

    /* responsive */
    @media (max-width: 980px){
      .topbar{font-size: 30px}
      .wrap{grid-template-columns:1fr}
      .infoGrid,.sensorGrid{font-size:20px}
    }

    /* ================= ALARM / INTERRUPT BAR ================= */
    #alarmBar{
      display:none;                 /* hidden unless alarm active */
      background:#d10000;           /* red */
      color:#ffffff;
      padding:14px 18px;
      font-weight:800;
      font-size:20px;
      border-radius:14px;
      margin:14px 18px;
     box-shadow: 0 8px 20px rgba(0,0,0,.25);
    }
    #alarmBar.show{
      display:block;
    }




  </style>
</head>
<body>
  <div class="topbar">
    <div>Dr Akhter’s Monitor Log</div>
    <div class="icons">
      <div class="icon">👤</div>
      <div class="icon">⚙️</div>
    </div>
  </div>

  <!-- ================= ALARM / INTERRUPT BAR ================= -->
  <div id="alarmBar">
    <span id="alarmText">INTERRUPT</span>
  </div>



  <div class="wrap">
    <!-- LEFT COLUMN -->
    <div class="leftCol">
      <div class="card">
        <h2>Patient Information</h2>
        <div class="infoGrid">
          <div class="label">Patient Name:</div><div class="value" id="pName">Sadia Miah</div>
          <div class="label">Patient ID:</div><div class="value" id="pId">49385</div>
          <div class="label">Age:</div><div class="value">67</div>
          <div class="label">Condition:</div><div class="value">Asthma</div>
          <div class="label">Location:</div><div class="value" id="pLoc">Khulna, Bangladesh</div>
          <div class="label">Date:</div><div class="value" id="pDate">09/02/2026</div>
          <div class="label">Time:</div><div class="value" id="pTime">12:08 BST</div>
        </div>
      </div>

      <div class="card">
        <h2>Sensor Readings</h2>
        <div class="sensorGrid">
          <div class="label">Heart Rate:</div><div class="value"><span id="hrNow">72</span> BPM</div>
          <div class="label">Average Heart Rate:</div><div class="value"><span id="hrAvg">75</span> BPM</div>
          <div class="label">Peak Heart Rate:</div><div class="value"><span id="hrPeak">112</span> BPM</div>
          <div class="label">Temperature:</div><div class="value"><span id="tNow">23.0</span>°C</div>
          <div class="label">Blood Oxygen:</div><div class="value"><span id="spo2">98</span>% SpO2</div>
          <div class="label">Air Pressure:</div><div class="value"><span id="press">80</span> PaO2</div>
          <div class="label">Humidity:</div><div class="value"><span id="hum">80</span>% HD</div>
        </div>
      </div>
    </div>

    <!-- RIGHT COLUMN -->
    <div class="rightCol">

  <!-- Heart Rate Graph -->
  <div class="chartBox">
    <div class="chartTitle">Heart Rate</div>
    <canvas id="hrChart" width="900" height="320"></canvas>
  </div>

  <!-- Temperature Graph -->
  <div class="chartBox">
    <div class="chartTitle">Temperature</div>
    <canvas id="tChart" width="900" height="320"></canvas>
  </div>

  </div>
</div>



<script>
  const THEME_PURPLE = "#5a1140";
  let hrFingerOn = true;

  function makeGrowingChart(canvasId, yMin, yMax, yLabel, maxPoints = 3600, xLabel = "Time") {
    const c = document.getElementById(canvasId);
    if (!c) return { push: () => {} };          // safety
    const ctx = c.getContext("2d");

    const data = [];
    const labels = [];

    let viewPoints = 30;
    let offset = 0;
    let hasFocus = false;

    function clampOffset() {
      const maxOffset = Math.max(0, data.length - viewPoints);
      offset = Math.min(Math.max(offset, 0), maxOffset);
    }

    function draw() {
      const w = c.width, h = c.height;
      ctx.clearRect(0, 0, w, h);

      const padL = 72, padR = 20, padT = 20, padB = 62;
      const pw = w - padL - padR;
      const ph = h - padT - padB;

      // background
      ctx.fillStyle = "#fff";
      ctx.fillRect(0, 0, w, h);

      // grid
      ctx.strokeStyle = "rgba(0,0,0,0.10)";
      ctx.lineWidth = 1;

      const gridY = 5;
      for (let i = 0; i <= gridY; i++) {
        const y = padT + (ph / gridY) * i;
        ctx.beginPath(); ctx.moveTo(padL, y); ctx.lineTo(w - padR, y); ctx.stroke();
      }

      const gridX = 6;
      for (let i = 0; i <= gridX; i++) {
        const x = padL + (pw / gridX) * i;
        ctx.beginPath(); ctx.moveTo(x, padT); ctx.lineTo(x, h - padB); ctx.stroke();
      }

      // axes
      ctx.strokeStyle = "rgba(0,0,0,0.45)";
      ctx.lineWidth = 1.5;
      ctx.beginPath();
      ctx.moveTo(padL, padT);
      ctx.lineTo(padL, h - padB);
      ctx.lineTo(w - padR, h - padB);
      ctx.stroke();

      // Y axis label
      ctx.fillStyle = "#333";
      ctx.font = "17px Arial";
      ctx.save();
      ctx.translate(15, padT + ph / 2);
      ctx.rotate(-Math.PI / 2);
      ctx.textAlign = "center";
      ctx.textBaseline = "middle";
      ctx.fillText(yLabel, 0, 0);
      ctx.restore();

      // X axis label
      ctx.font = "14px Arial";
      ctx.fillStyle = "#333";
      ctx.textAlign = "center";
      ctx.fillText(xLabel, padL + pw / 2, h - 6);
      ctx.textAlign = "start";

      // Y ticks
      ctx.font = "14px Arial";
      for (let i = 0; i <= gridY; i++) {
        const v = (yMax - (yMax - yMin) * (i / gridY));
        const y = padT + (ph / gridY) * i;

        ctx.fillStyle = "#555";
        ctx.fillText(v.toFixed(0), padL - 32, y + 5);

        ctx.strokeStyle = "rgba(0,0,0,0.45)";
        ctx.beginPath();
        ctx.moveTo(padL - 6, y);
        ctx.lineTo(padL, y);
        ctx.stroke();
      }

      // visible range
      clampOffset();
      const n = data.length;
      const end = Math.max(0, n - offset);
      const start = Math.max(0, end - viewPoints);

      // X ticks (start/mid/end)
      function drawXTick(realIdx, x) {
        ctx.strokeStyle = "rgba(0,0,0,0.45)";
        ctx.beginPath();
        ctx.moveTo(x, h - padB);
        ctx.lineTo(x, h - padB + 6);
        ctx.stroke();

        ctx.fillStyle = "#555";
        ctx.font = "14px Arial";
        ctx.fillText(labels[realIdx] || "", x - 26, h - 30);
      }

      if (end - start > 0) {
        const mid = Math.floor((start + end - 1) / 2);
        drawXTick(start, padL);
        drawXTick(mid, padL + pw * 0.5);
        drawXTick(end - 1, padL + pw);
      }

      // build points (NaN = gap)
      const pts = [];
      for (let i = start; i < end; i++) {
        const v = data[i];
        if (!Number.isFinite(v)) { pts.push(null); continue; }

        const idxInView = i - start;
        const x = padL + (pw * (idxInView / Math.max(1, (viewPoints - 1))));
        const t = (v - yMin) / (yMax - yMin);
        const y = (h - padB) - (ph * t);
        pts.push({ x, y });
      }

      // draw line with breaks + dots
      if (pts.length) {
        ctx.strokeStyle = THEME_PURPLE;
        ctx.lineWidth = 3;

        let started = false;
        ctx.beginPath();
        for (const p of pts) {
          if (!p) { started = false; continue; }
          if (!started) { ctx.moveTo(p.x, p.y); started = true; }
          else ctx.lineTo(p.x, p.y);
        }
        ctx.stroke();

        ctx.fillStyle = THEME_PURPLE;
        for (const p of pts) {
          if (!p) continue;
          ctx.beginPath();
          ctx.arc(p.x, p.y, 5, 0, Math.PI * 2);
          ctx.fill();
        }
      }

      // LIVE
      if (offset === 0) {
        ctx.fillStyle = "rgba(0,0,0,0.55)";
        ctx.font = "12px Arial";
        ctx.fillText("LIVE", w - padR - 34, padT + 14);
      }
    }

    function push(value, label) {
      data.push(Number(value));          // NaN stays NaN -> creates a gap
      labels.push(String(label));

      if (data.length > maxPoints) {
        data.shift();
        labels.shift();
        offset = Math.max(0, offset - 1);
      }
      draw();
    }

    // zoom
    c.addEventListener("wheel", (e) => {
      e.preventDefault();
      const delta = Math.sign(e.deltaY);
      viewPoints = Math.max(5, Math.min(600, viewPoints + delta * 5));
      draw();
    }, { passive:false });

    // drag pan
    let dragging = false, lastX = 0;
    c.addEventListener("mousedown", (e) => { dragging = true; lastX = e.clientX; hasFocus = true; });
    window.addEventListener("mouseup", () => dragging = false);
    window.addEventListener("mousemove", (e) => {
      if (!dragging) return;
      const dx = e.clientX - lastX;
      lastX = e.clientX;
      offset += Math.round(dx * viewPoints / Math.max(1, c.clientWidth));
      clampOffset();
      draw();
    });

    c.addEventListener("mouseenter", () => hasFocus = true);
    c.addEventListener("mouseleave", () => hasFocus = false);

    window.addEventListener("keydown", (e) => {
      if (!hasFocus) return;
      if (e.key === "ArrowLeft")  { offset += 10; clampOffset(); draw(); }
      if (e.key === "ArrowRight") { offset -= 10; clampOffset(); draw(); }
      if (e.key === "l" || e.key === "L") { offset = 0; draw(); }
    });

    draw();
    return { push };
  }

  // charts
  const hr = makeGrowingChart("hrChart", 0, 150, "Heart Rate (BPM)", 600, "Time");
  const tc = makeGrowingChart("tChart", 0, 50, "Temperature (°C)", 600, "Time");

  function toNum(v) {
    if (v === null || v === undefined) return NaN;
    if (v === "--") return NaN;
    const n = Number(v);
    return Number.isFinite(n) ? n : NaN;
  }

  async function poll() {
    try {
      const r = await fetch("/api/data", { cache: "no-store" });
      const d = await r.json();
      const label = new Date().toLocaleTimeString();

      if (d.date) document.getElementById("pDate").textContent = d.date;
      if (d.time) document.getElementById("pTime").textContent = d.time;

      const bpm   = toNum(d.bpm);
      const tempC = toNum(d.tempC);
      const spo2  = toNum(d.spo2);
      const press = toNum(d.pressure ?? d.kpa);
      const hum   = toNum(d.humidity ?? d.hum);

      // HR gap when finger off
      const fingerOnHR = Number.isFinite(bpm) && bpm > 0; // tweak threshold if needed
      if (fingerOnHR) {
        document.getElementById("hrNow").textContent = Math.round(bpm);
        hrFingerOn = true;
        hr.push(Math.round(bpm), label);
      } else {
        document.getElementById("hrNow").textContent = "--";
        if (hrFingerOn) {
          hrFingerOn = false;
          hr.push(NaN, label); // gap marker
        }
      }

      // Temp
      if (Number.isFinite(tempC)) {
        document.getElementById("tNow").textContent = tempC.toFixed(1);
        tc.push(Number(tempC.toFixed(1)), label);
      }

      // Other readings
      document.getElementById("spo2").textContent  = Number.isFinite(spo2) ? Math.round(spo2) : "--";
      if (Number.isFinite(press)) document.getElementById("press").textContent = press.toFixed(1);
      if (Number.isFinite(hum))   document.getElementById("hum").textContent   = hum.toFixed(0);


      // Alarm banner
      const alarmBar  = document.getElementById("alarmBar");
      const alarmText = document.getElementById("alarmText");

      const alarmOn  = (d.alarm === true) || (d.alarm === "true") || (d.alarm === 1) || (d.alarm === "1");
      const alarmTxt = (d.alarmText && String(d.alarmText).trim().length) ? String(d.alarmText) : "Limits exceeded";

      if (alarmOn) {
        alarmText.textContent = "INTERRUPT: " + alarmTxt;
        alarmBar.classList.add("show");
      } else {
        alarmBar.classList.remove("show");
      }







    } catch (e) {}
  }

  poll();
  setInterval(poll, 1000);
</script>

</body>
</html>
)HTML";






// ================= SEND DASHBOARD =================
static void sendDashboard(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=utf-8");
  client.println("Connection: close");
  client.println();
  client.print(INDEX_HTML);
}



// ---------- SEND API DATA ----------
static float safeNum(float v) {
  if (!isfinite(v)) return NAN;
  if (fabs(v) > 1e6) return NAN;
  return v;
}



static void sendApiData(WiFiClient& client,
                        float bpm, float tempC,
                        float spo2, float pressure, float humidity) {

  auto numOrNull = [](float v, int dp) -> String {
    if (!isfinite(v)) return "null";
    return String(v, dp);
  };


  String dateS = dateStringNow();
  String timeS = timeStringNow();


  float avgBpm  = (g_bpmCount > 0) ? (g_bpmSum / (float)g_bpmCount) : NAN;
  float peakBpm = (g_bpmPeak >= 0) ? g_bpmPeak : NAN;
  float lowBpm  = (g_bpmLow < 9999) ? g_bpmLow : NAN;


  String json = "{";
  json += "\"date\":\"" + dateS + "\",";
json += "\"time\":\"" + timeS + "\",";
  json += "\"bpm\":"      + numOrNull(bpm, 1) + ",";
  json += "\"avg\":"      + numOrNull(avgBpm, 1) + ",";
  json += "\"peak\":"     + numOrNull(peakBpm, 1) + ",";
  json += "\"low\":"      + numOrNull(lowBpm, 1) + ",";
  json += "\"tempC\":"    + numOrNull(tempC, 1) + ",";
  json += "\"spo2\":"     + numOrNull(spo2, 0) + ",";
  json += "\"pressure\":" + numOrNull(pressure, 1) + ",";
  json += "\"humidity\":" + numOrNull(humidity, 1);
  // Alarm fields (from Alarm_6.ino)
  json += "\"alarm\":" + String(Alarm_active() ? "true" : "false") + ",";
  json += "\"alarmText\":\"" + Alarm_textJsonEscaped() + "\"";
  json += "}";


  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.println();
  client.print(json);
}

















