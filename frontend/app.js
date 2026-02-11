(function () {
  const wsIndicator = document.getElementById("ws-indicator");
  const wsLabel = document.getElementById("ws-label");
  const serialBadge = document.getElementById("serial-badge");
  const devBadge = document.getElementById("dev-badge");
  const logEl = document.getElementById("log");
  const buttons = document.querySelectorAll(".ctrl[data-cmd]");

  let ws = null;
  let reconnectTimer = null;

  const KEY_MAP = {
    ArrowUp: "FWD", w: "FWD", W: "FWD",
    ArrowDown: "BCK", s: "BCK", S: "BCK",
    ArrowLeft: "LFT", a: "LFT", A: "LFT",
    ArrowRight: "RGT", d: "RGT", D: "RGT",
    " ": "STP",
  };

  function log(msg) {
    const d = document.createElement("div");
    d.textContent = msg;
    logEl.prepend(d);
    while (logEl.children.length > 50) logEl.lastChild.remove();
  }

  function setConnected(connected) {
    wsIndicator.className = "indicator " + (connected ? "connected" : "disconnected");
    wsLabel.textContent = connected ? "Connected" : "Disconnected";
  }

  function connect() {
    if (ws && ws.readyState <= WebSocket.OPEN) return;
    const proto = location.protocol === "https:" ? "wss:" : "ws:";
    ws = new WebSocket(proto + "//" + location.host + "/ws");

    ws.onopen = function () {
      setConnected(true);
      log("WebSocket connected");
      clearTimeout(reconnectTimer);
    };

    ws.onmessage = function (e) {
      const data = JSON.parse(e.data);
      if (data.type === "status") {
        serialBadge.classList.toggle("hidden", !data.serial);
        devBadge.classList.toggle("hidden", data.serial);
      } else if (data.type === "ack") {
        log("ACK: " + data.cmd);
      } else if (data.type === "error") {
        log("ERR: " + data.msg);
      }
    };

    ws.onclose = function () {
      setConnected(false);
      log("WebSocket disconnected — reconnecting...");
      scheduleReconnect();
    };

    ws.onerror = function () {
      ws.close();
    };
  }

  function scheduleReconnect() {
    clearTimeout(reconnectTimer);
    reconnectTimer = setTimeout(connect, 1000);
  }

  function send(cmd) {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({ cmd: cmd }));
    }
  }

  // Keyboard controls — send command on keydown, stop on keyup
  const heldKeys = new Set();

  document.addEventListener("keydown", function (e) {
    const cmd = KEY_MAP[e.key];
    if (!cmd || heldKeys.has(e.key)) return;
    e.preventDefault();
    heldKeys.add(e.key);
    send(cmd);
    highlightButton(cmd, true);
  });

  document.addEventListener("keyup", function (e) {
    const cmd = KEY_MAP[e.key];
    if (!cmd) return;
    heldKeys.delete(e.key);
    if (cmd !== "STP") send("STP");
    highlightButton(cmd, false);
  });

  // On-screen button controls (touch + mouse)
  buttons.forEach(function (btn) {
    const cmd = btn.dataset.cmd;

    function down(e) {
      e.preventDefault();
      send(cmd);
      btn.classList.add("active");
    }
    function up(e) {
      e.preventDefault();
      if (cmd !== "STP") send("STP");
      btn.classList.remove("active");
    }

    btn.addEventListener("mousedown", down);
    btn.addEventListener("mouseup", up);
    btn.addEventListener("mouseleave", up);
    btn.addEventListener("touchstart", down, { passive: false });
    btn.addEventListener("touchend", up, { passive: false });
    btn.addEventListener("touchcancel", up, { passive: false });
  });

  function highlightButton(cmd, on) {
    buttons.forEach(function (btn) {
      if (btn.dataset.cmd === cmd) {
        btn.classList.toggle("active", on);
      }
    });
  }

  connect();
})();
