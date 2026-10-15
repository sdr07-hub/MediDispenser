#include "app_webserver.h"
#include "storage.h"
#include "lcd.h"
#include "history.h"
#include "dispenser.h"
#include "stepper.h"

void handleRoot() {
  String html = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0">
<title>Medicine Dispenser</title>
<style>
  * { margin:0; padding:0; box-sizing:border-box; -webkit-tap-highlight-color:transparent; }
  body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
    background: #f0f4f8;
    color: #1a2a4a;
    min-height: 100vh;
    padding-bottom: 30px;
  }

  /* HEADER */
  .header {
    background: white;
    padding: 16px 20px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    box-shadow: 0 2px 8px rgba(0,0,0,0.08);
    position: sticky;
    top: 0;
    z-index: 100;
  }
  .header-left { display: flex; align-items: center; gap: 10px; }
  .header-icon {
    width: 36px; height: 36px;
    background: #2563eb;
    border-radius: 10px;
    display: flex; align-items: center; justify-content: center;
    font-size: 18px;
  }
  .header-title { font-size: 1.1rem; font-weight: 700; color: #1a2a4a; }
  .header-sub   { font-size: 0.7rem; color: #888; }
  .conn-badge {
    display: flex; align-items: center; gap: 6px;
    background: #f0fdf4;
    border: 1px solid #bbf7d0;
    border-radius: 999px;
    padding: 4px 12px;
    font-size: 0.75rem;
    color: #16a34a;
    font-weight: 600;
  }
  .conn-dot {
    width: 8px; height: 8px;
    background: #22c55e;
    border-radius: 50%;
    animation: blink 2s infinite;
  }
  @keyframes blink { 0%,100%{opacity:1} 50%{opacity:0.4} }

  /* CARDS */
  .container { padding: 16px; max-width: 480px; margin: 0 auto; }
  .card {
    background: white;
    border-radius: 16px;
    padding: 20px;
    margin-bottom: 14px;
    box-shadow: 0 2px 8px rgba(0,0,0,0.06);
  }
  .card-title {
    font-size: 0.8rem;
    font-weight: 700;
    color: #2563eb;
    text-transform: uppercase;
    letter-spacing: 1px;
    margin-bottom: 12px;
    display: flex;
    align-items: center;
    gap: 6px;
  }

  /* COUNTDOWN */
  .countdown-card { background: linear-gradient(135deg, #eff6ff, #dbeafe); }
  .countdown-label { font-size: 0.85rem; color: #3b82f6; font-weight: 600; margin-bottom: 6px; }
  .countdown-time  { font-size: 2rem; font-weight: 800; color: #1d4ed8; line-height: 1; }
  .countdown-sub   { font-size: 0.8rem; color: #6b7280; margin-top: 6px; }

  /* MEDICINE WHEEL */
  .wheel-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 14px; }
  .wheel-wrap { position: relative; }
  svg.wheel   { width: 100%; max-width: 200px; display: block; margin: 0 auto; }

  /* STATUS */
  .status-active {
    background: #f0fdf4;
    border: 1.5px solid #bbf7d0;
    border-radius: 12px;
    padding: 14px;
    display: flex;
    align-items: center;
    gap: 12px;
    cursor: pointer;
    transition: all 0.2s;
    margin-bottom: 12px;
  }
  .status-active:active { transform: scale(0.98); }
  .status-inactive {
    background: #fef2f2;
    border-color: #fecaca;
  }
  .status-icon {
    width: 36px; height: 36px;
    background: #22c55e;
    border-radius: 50%;
    display: flex; align-items: center; justify-content: center;
    font-size: 18px;
    flex-shrink: 0;
  }
  .status-icon.off { background: #ef4444; }
  .status-text-main { font-size: 0.95rem; font-weight: 700; color: #15803d; }
  .status-text-sub  { font-size: 0.75rem; color: #6b7280; margin-top: 2px; }

  /* DISPENSE BUTTON */
  .dispense-btn {
    width: 100%;
    padding: 18px;
    background: #16a34a;
    color: white;
    border: none;
    border-radius: 14px;
    font-size: 1rem;
    font-weight: 700;
    letter-spacing: 1px;
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 10px;
    transition: background 0.15s;
    user-select: none;
    -webkit-user-select: none;
    position: relative;
    overflow: hidden;
    min-height: 58px;
  }
  .dispense-btn:active { background: #15803d; }
  .dispense-btn.disarmed { background: #9ca3af; cursor: not-allowed; }
  .dispense-btn .btn-label {
    position: relative;
    z-index: 2;
    pointer-events: none;
  }
  /* Liquid fill layer */
  .dispense-btn .fill-wave {
    position: absolute;
    left: 0; bottom: 0;
    width: 100%;
    height: 0%;
    background: rgba(255,255,255,0.25);
    transition: height 0.1s linear;
    z-index: 1;
  }
  .dispense-btn .fill-wave::before {
    content: '';
    position: absolute;
    top: -8px; left: -10%;
    width: 120%; height: 16px;
    background: rgba(255,255,255,0.3);
    border-radius: 50%;
    animation: waveMove 0.6s infinite linear;
  }
  @keyframes waveMove {
    0%   { transform: translateX(0); }
    100% { transform: translateX(10%); }
  }
  .dispense-sub { font-size: 0.72rem; color: #9ca3af; text-align: center; margin-top: 8px; }

  /* ALARM TIME SETTER */
  .time-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; }
  .time-block {
    background: #f8fafc;
    border: 1px solid #e2e8f0;
    border-radius: 12px;
    padding: 14px;
    text-align: center;
  }
  .time-block-label { font-size: 0.75rem; color: #9ca3af; margin-bottom: 8px; font-weight: 600; }
  .time-block-val   { font-size: 2rem; font-weight: 800; color: #1d4ed8; line-height: 1; }
  .time-btn-row     { display: flex; justify-content: center; gap: 10px; margin-top: 10px; }
  .time-btn {
    width: 38px; height: 38px;
    background: #2563eb;
    color: white;
    border: none;
    border-radius: 10px;
    font-size: 1.3rem;
    font-weight: 700;
    cursor: pointer;
    display: flex; align-items: center; justify-content: center;
    transition: all 0.15s;
  }
  .time-btn:active { background: #1d4ed8; transform: scale(0.92); }
  .set-alarm-btn {
    width: 100%;
    padding: 12px;
    background: #2563eb;
    color: white;
    border: none;
    border-radius: 12px;
    font-size: 0.9rem;
    font-weight: 700;
    cursor: pointer;
    margin-top: 14px;
    transition: all 0.15s;
  }
  .set-alarm-btn:active { background: #1d4ed8; transform: scale(0.98); }

  /* HISTORY */
  .log-entry {
    display: flex;
    align-items: flex-start;
    gap: 10px;
    padding: 10px 0;
    border-bottom: 1px solid #f1f5f9;
  }
  .log-entry:last-child { border-bottom: none; }
  .log-dot {
    width: 10px; height: 10px;
    border-radius: 50%;
    margin-top: 4px;
    flex-shrink: 0;
  }
  .dot-system { background: #3b82f6; }
  .dot-taken  { background: #22c55e; }
  .dot-manual { background: #9333ea; }
  .dot-missed { background: #ef4444; }
  .log-text   { font-size: 0.82rem; color: #374151; line-height: 1.4; }
  .log-time   { font-size: 0.75rem; color: #9ca3af; }

  /* FOOTER */
  .footer { text-align: center; color: #d1d5db; font-size: 0.72rem; margin-top: 20px; }

  /* TOAST */
  .toast {
    position: fixed;
    bottom: 20px; left: 50%;
    transform: translateX(-50%) translateY(80px);
    background: #1a2a4a;
    color: white;
    padding: 10px 22px;
    border-radius: 999px;
    font-size: 0.85rem;
    font-weight: 600;
    transition: transform 0.3s;
    z-index: 999;
    white-space: nowrap;
  }
  .toast.show { transform: translateX(-50%) translateY(0); }

  /* STATUS BANNER */
  .alarm-banner {
    display: none;
    background: #fef2f2;
    border: 1.5px solid #fecaca;
    border-radius: 12px;
    padding: 14px;
    text-align: center;
    margin-bottom: 14px;
    animation: alertpulse 1s infinite;
  }
  @keyframes alertpulse { 0%,100%{border-color:#fecaca} 50%{border-color:#ef4444} }
  .alarm-banner.show { display: block; }
  .alarm-banner-title { font-size: 1rem; font-weight: 800; color: #dc2626; }
  .alarm-banner-sub   { font-size: 0.8rem; color: #9ca3af; margin-top: 4px; }

  /* ==========================================
     RESPONSIVE — DESKTOP / TABLET MODE
     Mobile-first styles above stay the default;
     these widen and reflow the layout on bigger screens.
     ========================================== */
  .desktop-grid { display: block; }
  .desktop-col  { display: block; }

  @media (min-width: 720px) {
    .header { padding: 18px 32px; }
    .header-title { font-size: 1.3rem; }
    .container { max-width: 920px; padding: 28px 32px; }

    /* Two-column desktop grid: left = wheel+status+dispense, right = alarm setter+history */
    .desktop-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 20px;
      align-items: start;
    }
    .wheel-grid { grid-template-columns: 220px 1fr; gap: 20px; }
    svg.wheel { max-width: 220px; }
    .card { padding: 24px; }
    .dispense-btn { min-height: 64px; font-size: 1.05rem; }
  }
  @media (min-width: 1100px) {
    .container { max-width: 1080px; }
  }
</style>
</head>
<body>

<!-- HEADER -->
<div class="header">
  <div class="header-left">
    <div class="header-icon">💊</div>
    <div>
      <div class="header-title">Medicine Dispenser</div>
      <div class="header-sub">Your daily medicine helper</div>
    </div>
  </div>
  <div class="conn-badge">
    <div class="conn-dot"></div>
    <span id="connLabel">Connected</span>
  </div>
</div>

<div class="container">

  <!-- ALARM BANNER (shows during alarm) -->
  <div class="alarm-banner" id="alarmBanner">
    <div class="alarm-banner-title">🔔 Time for your medicine!</div>
    <div class="alarm-banner-sub">Place your hand under the dispenser</div>
  </div>

  <!-- COUNTDOWN CARD -->
  <div class="card countdown-card">
    <div class="countdown-label">⏱ Your Next Medicine Time</div>
    <div class="countdown-time" id="countdown">Calculating...</div>
    <div class="countdown-sub" id="countdownSub">Scheduled for <b id="alarmDisplay">08:00</b> every day</div>
  </div>

  <div class="desktop-grid">
  <div class="desktop-col">

  <!-- WHEEL + STATUS GRID -->
  <div class="wheel-grid">

    <!-- MEDICINE WHEEL -->
    <div class="card" style="padding:16px;">
      <div class="card-title">🔄 Medicine Wheel</div>
      <div class="wheel-wrap">
        <svg class="wheel" viewBox="0 0 200 200" id="wheelSvg">
          <!-- Drawn by JS -->
        </svg>
      </div>
      <div style="text-align:center;font-size:0.75rem;color:#9ca3af;margin-top:8px;">
        Today: <b id="todayLabel">--</b>
      </div>
      <div style="display:flex;justify-content:center;gap:14px;margin-top:10px;font-size:0.68rem;color:#9ca3af;">
        <span><span style="display:inline-block;width:8px;height:8px;border-radius:50%;background:#16a34a;margin-right:4px;"></span>Scheduled</span>
        <span><span style="display:inline-block;width:8px;height:8px;border-radius:50%;background:#0891b2;margin-right:4px;"></span>Manual</span>
      </div>
    </div>

    <!-- STATUS + DISPENSE -->
    <div style="display:flex;flex-direction:column;gap:10px;">

      <!-- SYSTEM STATUS -->
      <div class="card" style="padding:14px;">
        <div class="card-title">⚡ System Status</div>
        <div class="status-active" id="statusToggle" onclick="toggleArm()">
          <div class="status-icon" id="statusIcon">✓</div>
          <div>
            <div class="status-text-main" id="statusText">Active & Safe</div>
            <div class="status-text-sub" id="statusSub">Tap to disarm</div>
          </div>
        </div>
        <div style="font-size:0.72rem;color:#9ca3af;text-align:center;" id="stateLabel">STANDBY</div>
      </div>

      <!-- MANUAL DISPENSE -->
      <div class="card" style="padding:14px;">
        <div class="card-title">💊 Need a dose now?</div>
        <button class="dispense-btn" id="dispenseBtn"
          onmousedown="startHold()" onmouseup="endHold()" onmouseleave="endHold()"
          ontouchstart="startHold(event)" ontouchend="endHold()">
          <div class="fill-wave" id="fillWave"></div>
          <span class="btn-label" id="btnLabel">👋 PRESS &amp; HOLD</span>
        </button>
        <div class="dispense-sub">Hold for 2 seconds to release a dose</div>
      </div>

    </div>
  </div>

  </div>
  <div class="desktop-col">

  <!-- ALARM TIME SETTER -->
  <div class="card">
    <div class="card-title">⏰ Daily Medicine Time</div>
    <div style="font-size:0.8rem;color:#9ca3af;margin-bottom:12px;">
      Currently set to <b id="alarmSetDisplay">08:00 AM</b>
    </div>
    <div class="time-grid">
      <div class="time-block">
        <div class="time-block-label">HOUR</div>
        <div class="time-block-val" id="hourDisplay">08</div>
        <div class="time-btn-row">
          <button class="time-btn" onclick="adjustHour(-1)">−</button>
          <button class="time-btn" onclick="adjustHour(1)">+</button>
        </div>
      </div>
      <div class="time-block">
        <div class="time-block-label">MINUTE</div>
        <div class="time-block-val" id="minDisplay">00</div>
        <div class="time-btn-row">
          <button class="time-btn" onclick="adjustMin(-1)">−</button>
          <button class="time-btn" onclick="adjustMin(1)">+</button>
        </div>
      </div>
    </div>
    <button class="set-alarm-btn" onclick="setAlarm()">✓ Set Alarm Time</button>
  </div>

  <!-- PILL SUPPLY -->
  <div class="card">
    <div class="card-title">💊 Pill Supply</div>
    <div id="refillBanner" style="display:none;align-items:center;gap:10px;background:#fef2f2;border:1px solid #fecaca;border-radius:10px;padding:10px 12px;margin-bottom:12px;">
      <span style="font-size:1.1rem;">⚠️</span>
      <div style="flex:1;font-size:0.78rem;color:#b91c1c;">Carousel is empty — please refill all 7 slots.</div>
      <button onclick="confirmRefill()" style="background:#dc2626;color:#fff;border:none;border-radius:8px;padding:6px 10px;font-size:0.72rem;font-weight:700;cursor:pointer;">I've Refilled</button>
    </div>
    <div style="background:#f1f5f9;border-radius:8px;height:10px;overflow:hidden;">
      <div id="pillsBarFill" style="height:100%;width:100%;background:#22c55e;transition:width 0.4s ease, background 0.4s ease;"></div>
    </div>
    <div id="pillsBarLabel" style="text-align:center;font-size:0.75rem;color:#9ca3af;margin-top:8px;">7 of 7 doses left this week</div>
  </div>

  <!-- HISTORY LOG -->
  <div class="card">
    <div class="card-title">📋 History Log</div>
    <div style="font-size:0.75rem;color:#9ca3af;margin-bottom:10px;">What happened recently</div>
    <div id="historyLog">
      <div style="color:#d1d5db;font-size:0.8rem;text-align:center;padding:20px;">No history yet</div>
    </div>
  </div>

  </div>
  </div>

  <div class="footer">♥ Made with care for your wellbeing</div>
</div>

<!-- tiny demo/debug toggle, bottom-left corner -->
<div id="debugToggle" onclick="toggleDebugMode()"
     style="position:fixed;bottom:6px;left:8px;font-size:0.62rem;color:#d1d5db;cursor:pointer;user-select:none;z-index:50;">test mode</div>

<!-- TOAST -->
<div class="toast" id="toast"></div>

<script>
// ==========================================
// STATE
// ==========================================
let alarmHour = 8;
let alarmMin  = 0;
let editHour  = 8;
let editMin   = 0;
let holdTimer = null;
let holdProgress = 0;
let armed = true;
let userEditingAlarm = false;  // track if user is touching the alarm controls
let debugMode = false;
let manualTakenToday = false;
let needsRefill = false;

const days = ['Sun','Mon','Tue','Wed','Thu','Fri','Sat'];

// ==========================================
// FETCH STATUS FROM ESP32 EVERY 2 SECONDS
// ==========================================
let missedPolls = 0;
function fetchStatus() {
  fetch('/api/status?_=' + Date.now(), { cache: 'no-store' })
    .then(r => r.json())
    .then(d => {
      missedPolls = 0;
      updateUI(d);
    })
    .catch(() => {
      missedPolls++;
      if (missedPolls < 2) return;   // ignore a single blip — only flag real drops
      const badge = document.querySelector('.conn-badge');
      badge.style.background = '#fef2f2';
      badge.style.color = '#dc2626';
      badge.style.borderColor = '#fecaca';
      document.querySelector('.conn-dot').style.background = '#ef4444';
      document.getElementById('connLabel').textContent = 'Reconnecting...';
    });
}

function updateUI(d) {
  // Connection badge
  const badge = document.querySelector('.conn-badge');
  badge.style.background = '#f0fdf4';
  badge.style.color = '#16a34a';
  badge.style.borderColor = '#bbf7d0';
  document.querySelector('.conn-dot').style.background = '#22c55e';
  document.getElementById('connLabel').textContent = 'Connected';

  // Alarm settings — only sync edit fields if user isn't actively adjusting
  alarmHour = d.alarmHour;
  alarmMin  = d.alarmMinute;
  if (!userEditingAlarm) {
    editHour  = alarmHour;
    editMin   = alarmMin;
    updateAlarmDisplays();
  }

  // Countdown
  updateCountdown(d);

  // State label
  document.getElementById('stateLabel').textContent = d.state || 'STANDBY';

  // Alarm banner
  const banner = document.getElementById('alarmBanner');
  if (d.state === 'ALARM_ACTIVE') {
    banner.classList.add('show');
  } else {
    banner.classList.remove('show');
  }

  // Medicine wheel — always redraw from server data
  const serverDay = d.currentDay !== undefined ? d.currentDay : 0;
  drawWheel(d.displaySlot ?? d.slot ?? 0, serverDay, d.slotTypes, !!d.needsRefill);
  document.getElementById('todayLabel').textContent = d.carouselLabel || days[serverDay];

  // Status toggle
  armed = d.armed;
  updateArmDisplay();

  // Debug mode + daily manual-dose limit
  debugMode = !!d.debug;
  manualTakenToday = !!d.manualTakenToday;
  needsRefill = !!d.needsRefill;
  updateDispenseButtonState();
  updateDebugIndicator();
  updatePillsBar(d);

  // History
  if (d.history && d.history.length > 0) {
    let html = '';
    for (let i = d.history.length - 1; i >= 0; i--) {
      const h = d.history[i];
      const dotClass = h.type === 0 ? 'dot-system' : h.type === 1 ? 'dot-taken' : h.type === 3 ? 'dot-manual' : 'dot-missed';
      html += `<div class="log-entry">
        <div class="log-dot ${dotClass}"></div>
        <div class="log-text">${h.entry}</div>
      </div>`;
    }
    document.getElementById('historyLog').innerHTML = html;
  }
}

// ==========================================
// COUNTDOWN TIMER
// ==========================================
function updateCountdown(d) {
  if (!d.hour && d.hour !== 0) return;
  const now  = d.hour * 60 + d.minute;
  const alarm = alarmHour * 60 + alarmMin;
  let diff = alarm - now;
  if (diff < 0) diff += 1440; // next day

  if (d.state === 'ALARM_ACTIVE') {
    document.getElementById('countdown').textContent = '🔔 Time Now!';
    document.getElementById('countdownSub').innerHTML = 'Place your hand under the dispenser';
    return;
  }
  if (d.state === 'DISPENSING') {
    document.getElementById('countdown').textContent = '⚙ Dispensing...';
    document.getElementById('countdownSub').innerHTML = d.manual ?
      '👋 Manual dose requested from app' : '⏰ Scheduled automatic dose';
    return;
  }
  if (d.doseGivenToday && !d.debug) {
    document.getElementById('countdown').textContent = '✓ Today\'s dose given';
    const ampm2 = alarmHour >= 12 ? 'PM' : 'AM';
    const h122  = alarmHour % 12 || 12;
    document.getElementById('countdownSub').innerHTML =
      'Next automatic dose tomorrow at <b>' + pad(h122) + ':' + pad(alarmMin) + ' ' + ampm2 + '</b>';
    return;
  }

  const h = Math.floor(diff / 60);
  const m = diff % 60;
  let txt = '';
  if (h > 0) txt += h + ' Hour' + (h > 1 ? 's' : '') + ', ';
  txt += m + ' Minute' + (m !== 1 ? 's' : '');
  document.getElementById('countdown').textContent = 'Next dose in: ' + txt;

  const ampm = alarmHour >= 12 ? 'PM' : 'AM';
  const h12  = alarmHour % 12 || 12;
  document.getElementById('countdownSub').innerHTML =
    'Scheduled for <b>' + pad(h12) + ':' + pad(alarmMin) + ' ' + ampm + '</b> every day';
}

// ==========================================
// MEDICINE WHEEL SVG
// ==========================================
function drawWheel(currentSlot, currentDay, slotTypes, needsRefill) {
  const svg   = document.getElementById('wheelSvg');
  const cx    = 100, cy = 100, r = 72, sr = 20;
  const labels = ['Sun','Mon','Tue','Wed','Thu','Fri','Sat','LOAD'];
  slotTypes = slotTypes || [];
  currentSlot = Math.max(0, Math.min(7, currentSlot || 0));
  let html = '';

  html += `<circle cx="${cx}" cy="${cy}" r="${r+8}" fill="none" stroke="#e2e8f0" stroke-width="1.5"/>`;

  for (let i = 0; i < 8; i++) {
    const angle = (i * 45 - 90) * Math.PI / 180;
    const x = cx + r * Math.cos(angle);
    const y = cy + r * Math.sin(angle);
    const doseType = slotTypes[i] || 0;
    let fill = '#f8fafc', stroke = '#e2e8f0', textCol = '#64748b';

    if (i === 7) {
      fill = needsRefill ? '#fef2f2' : '#f1f5f9';
      stroke = needsRefill ? '#ef4444' : '#cbd5e1';
      textCol = needsRefill ? '#dc2626' : '#94a3b8';
    }

    if (doseType === 2) {
      fill = '#ecfeff'; stroke = '#67e8f9'; textCol = '#0891b2';
    } else if (doseType === 1) {
      fill = '#f0fdf4'; stroke = '#bbf7d0'; textCol = '#16a34a';
    }

    if (i === currentSlot) {
      fill = needsRefill && i === 7 ? '#fee2e2' : '#dbeafe';
      stroke = needsRefill && i === 7 ? '#dc2626' : '#2563eb';
      textCol = needsRefill && i === 7 ? '#b91c1c' : '#1d4ed8';
      html += `<circle cx="${x}" cy="${y}" r="${sr+5}" fill="none" stroke="${stroke}" stroke-width="1" opacity="0.3"/>`;
    }

    html += `<circle cx="${x}" cy="${y}" r="${sr}" fill="${fill}" stroke="${stroke}" stroke-width="2"/>`;
    html += `<text x="${x}" y="${y}" text-anchor="middle" dominant-baseline="central"
               font-size="8" font-weight="700" fill="${textCol}">${labels[i]}</text>`;
  }

  const centerLabel = needsRefill ? 'Loading' : 'Next';
  const centerValue = needsRefill ? 'Bay' : labels[currentSlot];
  html += `<circle cx="${cx}" cy="${cy}" r="22" fill="white" stroke="#e2e8f0" stroke-width="1.5"/>`;
  html += `<text x="${cx}" y="${cy-5}" text-anchor="middle" dominant-baseline="central"
             font-size="7" fill="#94a3b8">${centerLabel}</text>`;
  html += `<text x="${cx}" y="${cy+7}" text-anchor="middle" dominant-baseline="central"
             font-size="10" font-weight="800" fill="#1d4ed8">${centerValue}</text>`;

  svg.innerHTML = html;
}

// ==========================================
// ARM / DISARM
// ==========================================
function toggleArm() {
  armed = !armed;
  fetch('/api/arm?state=' + (armed ? 1 : 0) + '&_=' + Date.now(), { cache: 'no-store' })
    .then(() => {
      updateArmDisplay();
      showToast(armed ? '✓ System Armed' : '⚠ System Disarmed');
      fetchStatus();
    });
}

function updateArmDisplay() {
  const toggle  = document.getElementById('statusToggle');
  const icon    = document.getElementById('statusIcon');
  const text    = document.getElementById('statusText');
  const sub     = document.getElementById('statusSub');
  const btn     = document.getElementById('dispenseBtn');
  const lbl     = document.getElementById('btnLabel');
  if (armed) {
    toggle.className = 'status-active';
    icon.className   = 'status-icon';
    icon.textContent = '✓';
    text.style.color = '#15803d';
    text.textContent = 'Active & Safe';
    sub.textContent  = 'Tap to disarm';
    btn.classList.remove('disarmed');
    lbl.textContent  = '👋 PRESS & HOLD';
  } else {
    toggle.className = 'status-active status-inactive';
    icon.className   = 'status-icon off';
    icon.textContent = '✕';
    text.style.color = '#dc2626';
    text.textContent = 'System Disarmed';
    sub.textContent  = 'Tap to arm';
    btn.classList.add('disarmed');
    lbl.textContent  = '🔒 SYSTEM DISARMED';
  }
}

function updateDispenseButtonState() {
  if (!armed) return; // updateArmDisplay already handled the disarmed look
  const btn = document.getElementById('dispenseBtn');
  const lbl = document.getElementById('btnLabel');
  const sub = document.querySelector('.dispense-sub');
  if (needsRefill && !debugMode) {
    btn.classList.add('disarmed');
    lbl.textContent = '🧺 Refill needed';
    if (sub) sub.textContent = 'Carousel is on the loading bay';
  } else if (manualTakenToday && !debugMode) {
    btn.classList.add('disarmed');
    lbl.textContent = '✓ Already taken today';
    if (sub) sub.textContent = 'Come back tomorrow for another dose';
  } else {
    btn.classList.remove('disarmed');
    lbl.textContent = '👋 PRESS & HOLD';
    if (sub) sub.textContent = 'Hold for 2 seconds to release a dose';
  }
}

function updateDebugIndicator() {
  const el = document.getElementById('debugToggle');
  if (!el) return;
  el.textContent = debugMode ? 'test mode: ON' : 'test mode';
  el.style.color = debugMode ? '#f59e0b' : '#d1d5db';
}

function toggleDebugMode() {
  debugMode = !debugMode;
  fetch('/api/debug?state=' + (debugMode ? 1 : 0) + '&_=' + Date.now(), { cache: 'no-store' })
    .then(() => {
      updateDebugIndicator();
      showToast(debugMode ? '🧪 Test mode ON — daily limits bypassed' : 'Test mode off');
      fetchStatus();
    });
}

function updatePillsBar(d) {
  const bar    = document.getElementById('pillsBarFill');
  const label  = document.getElementById('pillsBarLabel');
  const banner = document.getElementById('refillBanner');
  if (!bar) return;
  const remaining = (d.pillsRemaining !== undefined) ? d.pillsRemaining : 7;
  const pct = Math.max(0, Math.min(100, (remaining / 7) * 100));
  bar.style.width = pct + '%';
  bar.style.background = remaining <= 2 ? '#ef4444' : remaining <= 4 ? '#f59e0b' : '#22c55e';
  label.textContent = d.needsRefill ? '0 of 7 doses left — carousel is on loading bay' : (remaining + ' of 7 doses left this week');

  if (banner) {
    if (d.needsRefill) {
      banner.style.display = 'flex';
    } else {
      banner.style.display = 'none';
    }
  }
}

function confirmRefill() {
  fetch('/api/refill?_=' + Date.now(), { cache: 'no-store' })
    .then(() => {
      showToast('✓ Carousel marked as refilled');
      fetchStatus();
    });
}

// ==========================================
// PRESS & HOLD TO DISPENSE
// ==========================================
function startHold(e) {
  if (e) e.preventDefault();
  if (!armed) {
    showToast('⚠ System is disarmed!');
    return;
  }
  if (needsRefill && !debugMode) {
    showToast('🧺 Carousel empty — refill first');
    return;
  }
  if (manualTakenToday && !debugMode) {
    showToast('⏳ Already used today — come back tomorrow');
    return;
  }
  holdProgress = 0;
  const btn  = document.getElementById('dispenseBtn');
  const wave = document.getElementById('fillWave');
  const lbl  = document.getElementById('btnLabel');
  btn.style.background = '#dc2626';
  wave.style.height = '0%';
  lbl.textContent = '⏳ Hold to dispense...';

  holdTimer = setInterval(() => {
    holdProgress += 5;
    wave.style.height = holdProgress + '%';
    if (holdProgress >= 100) {
      clearInterval(holdTimer);
      holdTimer = null;
      btn.style.background = '#16a34a';
      wave.style.height = '0%';
      lbl.textContent = '👋 PRESS & HOLD';
      fetch('/api/dispense?_=' + Date.now(), { cache: 'no-store' })
        .then(r => r.json())
        .then(data => {
          if (data.ok === false && data.reason === 'disarmed') {
            showToast('⚠ System is disarmed!');
          } else if (data.ok === false && data.reason === 'already_taken_today') {
            showToast('⏳ Already used today — come back tomorrow');
          } else if (data.ok === false && data.reason === 'needs_refill') {
            showToast('🧺 Carousel empty — refill first');
          } else {
            showToast('💊 Dispensing medicine...');
          }
          fetchStatus();
        });
    }
  }, 100); // 100ms × 20 steps = 2 seconds
}

function endHold() {
  if (holdTimer) {
    clearInterval(holdTimer);
    holdTimer = null;
    const btn  = document.getElementById('dispenseBtn');
    const wave = document.getElementById('fillWave');
    const lbl  = document.getElementById('btnLabel');
    btn.style.background = '#16a34a';
    wave.style.height = '0%';
    lbl.textContent = '👋 PRESS & HOLD';
  }
}

// ==========================================
// ALARM TIME CONTROLS
// ==========================================
function adjustHour(d) {
  userEditingAlarm = true;
  editHour = (editHour + d + 24) % 24;
  updateAlarmDisplays();
}

function adjustMin(d) {
  userEditingAlarm = true;
  editMin = (editMin + d + 60) % 60;
  updateAlarmDisplays();
}

function updateAlarmDisplays() {
  document.getElementById('hourDisplay').textContent = pad(editHour);
  document.getElementById('minDisplay').textContent  = pad(editMin);
  const ampm = editHour >= 12 ? 'PM' : 'AM';
  const h12  = editHour % 12 || 12;
  document.getElementById('alarmDisplay').textContent    = pad(h12) + ':' + pad(editMin) + ' ' + ampm;
  document.getElementById('alarmSetDisplay').textContent = pad(h12) + ':' + pad(editMin) + ' ' + ampm;
}

function setAlarm() {
  fetch('/api/setalarm?hour=' + editHour + '&min=' + editMin + '&_=' + Date.now(), { cache: 'no-store' })
    .then(() => {
      alarmHour = editHour;
      alarmMin  = editMin;
      userEditingAlarm = false;  // allow server sync again after saving
      showToast('✓ Alarm set to ' + pad(editHour) + ':' + pad(editMin));
      fetchStatus();
    });
}

// ==========================================
// HELPERS
// ==========================================
function pad(n) { return n.toString().padStart(2, '0'); }

function showToast(msg) {
  const t = document.getElementById('toast');
  t.textContent = msg;
  t.classList.add('show');
  setTimeout(() => t.classList.remove('show'), 2500);
}

// ==========================================
// START POLLING
// ==========================================
fetchStatus();
setInterval(fetchStatus, 1000);
</script>
</body>
</html>
)rawhtml";

  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
  server.send(200, "text/html", html);
}

// ==========================================
// API — /api/status
// Returns all device state as JSON
// ==========================================
void handleApiStatus() {
  DateTime now = rtc.now();

  // Calculate minutes until next alarm
  int nowMins   = now.hour() * 60 + now.minute();
  int alarmMins = alarmHour * 60 + alarmMinute;
  int diff      = alarmMins - nowMins;
  if (diff < 0) diff += 1440;

  // State string
  const char* stateStr;
  switch (currentState) {
    case STANDBY:       stateStr = "STANDBY";       break;
    case ALARM_ACTIVE:  stateStr = "ALARM_ACTIVE";  break;
    case DISPENSING:    stateStr = "DISPENSING";    break;
    case DONE:          stateStr = "DONE";          break;
    case TEST_COMPLETE: stateStr = "TEST_COMPLETE"; break;
    default:            stateStr = "UNKNOWN";       break;
  }

  // Build JSON manually — no library needed
  int safeDoses = dosesThisWeek;
  if (safeDoses < 0) safeDoses = 0;
  if (safeDoses > PILL_SLOTS) safeDoses = PILL_SLOTS;
  int pillsRemaining = PILL_SLOTS - safeDoses;
  bool needsRefill = safeDoses >= PILL_SLOTS;
  int displaySlot = needsRefill ? LOADING_BAY_SLOT : safeDoses;
  const char* carouselLabel = needsRefill ? "Loading Bay" : dayNames[displaySlot];

  String json = "{";
  json += "\"state\":\"" + String(stateStr) + "\",";
  json += "\"slot\":" + String(currentSlot) + ",";
  json += "\"displaySlot\":" + String(displaySlot) + ",";
  json += "\"carouselLabel\":\"" + String(carouselLabel) + "\",";
  json += "\"alarmHour\":" + String(alarmHour) + ",";
  json += "\"alarmMinute\":" + String(alarmMinute) + ",";
  json += "\"armed\":" + String(systemArmed ? "true" : "false") + ",";
  json += "\"manual\":" + String(lastDispenseManual ? "true" : "false") + ",";
  json += "\"hour\":" + String(now.hour()) + ",";
  json += "\"minute\":" + String(now.minute()) + ",";
  json += "\"second\":" + String(now.second()) + ",";
  json += "\"currentDay\":" + String(now.dayOfTheWeek()) + ",";
  json += "\"minsUntil\":" + String(diff) + ",";
  json += "\"debug\":" + String(debugMode ? "true" : "false") + ",";
  json += "\"doseGivenToday\":" + String(lastDispensedDay == now.dayOfTheWeek() ? "true" : "false") + ",";
  json += "\"manualTakenToday\":" + String(lastManualDay == now.dayOfTheWeek() ? "true" : "false") + ",";
  json += "\"dosesThisWeek\":" + String(safeDoses) + ",";
  json += "\"pillsRemaining\":" + String(pillsRemaining) + ",";
  json += "\"needsRefill\":" + String(needsRefill ? "true" : "false") + ",";

  // Slot dose-type array — 0=untouched, 1=auto(green), 2=manual(cyan)
  json += "\"slotTypes\":[";
  for (int i = 0; i < 8; i++) {
    if (i > 0) json += ",";
    json += String(slotDoseType[i]);
  }
  json += "],";

  // History array
  json += "\"history\":[";
  for (int i = 0; i < doseHistoryCount; i++) {
    if (i > 0) json += ",";
    json += "{\"entry\":\"" + String(doseHistory[i].entry) + "\",";
    json += "\"type\":"     + String(doseHistory[i].type) + "}";
  }
  json += "]}";

  // CORS + cache-busting headers — without these, some browsers (Brave/Chrome)
  // aggressively cache this JSON and the UI looks frozen until a hard refresh
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "application/json", json);
}

// ==========================================
// API — /api/setalarm?hour=8&min=0
// ==========================================
void handleApiSetAlarm() {
  if (server.hasArg("hour")) alarmHour   = server.arg("hour").toInt();
  if (server.hasArg("min"))  alarmMinute = server.arg("min").toInt();

  prefs.putInt("alarmHour", alarmHour);
  prefs.putInt("alarmMin",  alarmMinute);

  char msg[40];
  snprintf(msg, sizeof(msg), "Alarm set to %02d:%02d", alarmHour, alarmMinute);
  addLog(msg, 0);

  char lcdMsg[17];
  snprintf(lcdMsg, sizeof(lcdMsg), "Next Dose %02d:%02d", alarmHour, alarmMinute);
  lcdLine(1, lcdMsg);

  Serial.print(">> Alarm set to ");
  Serial.print(alarmHour); Serial.print(":"); Serial.println(alarmMinute);

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"ok\":true}");
}

// ==========================================
// API — /api/dispense (manual from app)
// ==========================================
void handleApiDispense() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-store");

  if (!systemArmed) {
    // Show disarmed message on LCD for 2 seconds then restore
    lcdShow2("** OFFLINE **", "Arm to dispense");
    server.send(200, "application/json", "{\"ok\":false,\"reason\":\"disarmed\"}");
    Serial.println(">> DISPENSE BLOCKED: system disarmed");
    return;
  }

  if (!debugMode && dosesThisWeek >= PILL_SLOTS) {
    lcdShow2("Refill Needed", "Loading Bay");
    server.send(200, "application/json", "{\"ok\":false,\"reason\":\"needs_refill\"}");
    Serial.println(">> DISPENSE BLOCKED: carousel empty/loading bay");
    return;
  }

  DateTime now = rtc.now();
  if (!debugMode && lastManualDay == now.dayOfTheWeek()) {
    lcdShow2("Already Taken", "Come back tomorrow");
    server.send(200, "application/json", "{\"ok\":false,\"reason\":\"already_taken_today\"}");
    Serial.println(">> DISPENSE BLOCKED: manual dose already used today");
    return;
  }

  server.send(200, "application/json", "{\"ok\":true}");

  if (currentState == STANDBY) {
    lastDispenseManual = true;
    triggerDispense(now);
    Serial.println(">> MANUAL DISPENSE from app");
  }
}

// ==========================================
// API — /api/arm?state=1
// ==========================================
void handleApiArm() {
  if (server.hasArg("state")) {
    systemArmed = server.arg("state").toInt() == 1;
    prefs.putBool("armed", systemArmed);
    if (systemArmed) {
      tempMsgText  = "SYSTEM ONLINE";
      tempMsgUntil = millis() + 2000;
    }
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", "{\"ok\":true}");
  Serial.println(systemArmed ? ">> ARMED" : ">> DISARMED");
}

// ==========================================
// API — /api/debug?state=1   (demo/test mode — bypasses once-per-day limits)
// ==========================================
void handleApiDebug() {
  if (server.hasArg("state")) {
    debugMode = server.arg("state").toInt() == 1;
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", "{\"ok\":true,\"debug\":" + String(debugMode ? "true" : "false") + "}");
  Serial.println(debugMode ? ">> DEBUG MODE ON (no daily limits)" : ">> DEBUG MODE OFF");
}

// ==========================================
// API — /api/refill   (call after physically refilling the carousel)
// ==========================================
void handleApiRefill() {
  // If the carousel is sitting on the loading bay after the 7th dose,
  // physically advance one more 45-degree slot so the first filled slot
  // becomes the new Sunday/start position.
  bool movedToSunday = false;
  if (currentSlot == LOADING_BAY_SLOT || dosesThisWeek >= PILL_SLOTS) {
    Serial.println(">> REFILL: moving from Loading Bay to Sunday");
    rotateOneSlot();
    movedToSunday = true;
  } else {
    Serial.printf(">> REFILL: software reset only, currentSlot=%d, doses=%d/%d\n",
                  currentSlot, dosesThisWeek, PILL_SLOTS);
  }

  resetCarouselState();
  addLog(movedToSunday ? "Carousel refilled - moved to Sunday" : "Carousel refilled - starting at Sunday", 0);
  lcdShow2("Refilled", "Sunday 7/7");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", movedToSunday ? "{\"ok\":true,\"moved\":true}" : "{\"ok\":true,\"moved\":false}");
  Serial.println(">> CAROUSEL REFILLED — reset to Sunday, 7/7 remaining");
}


void setupWebServer() {
  server.on("/",             handleRoot);
  server.on("/api/status",   handleApiStatus);
  server.on("/api/setalarm", handleApiSetAlarm);
  server.on("/api/dispense", handleApiDispense);
  server.on("/api/arm",      handleApiArm);
  server.on("/api/debug",    handleApiDebug);
  server.on("/api/refill",   handleApiRefill);
  server.begin();
}
