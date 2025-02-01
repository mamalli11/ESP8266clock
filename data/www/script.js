document.addEventListener("DOMContentLoaded", () => {
  loadCurrentSettings();

  // Form Submit Handlers
  document
    .getElementById("wifiForm")
    .addEventListener("submit", handleWifiSubmit);
  document
    .getElementById("timeForm")
    .addEventListener("submit", handleTimeSubmit);
  document
    .getElementById("alarmForm")
    .addEventListener("submit", handleAlarmSubmit);
  document
    .getElementById("displayForm")
    .addEventListener("submit", handleDisplaySubmit);
});

async function loadCurrentSettings() {
  try {
    const response = await fetch("/get-settings");
    const settings = await response.json();

    // Populate WiFi Settings
    document.querySelector('[name="ssid"]').value = settings.wifi.ssid || "";
    document.querySelector('[name="password"]').value =
      settings.wifi.password || "";

    // Populate Time Settings
    document.querySelector('[name="city"]').value = settings.time.city || "";
    document.querySelector('[name="country"]').value =
      settings.time.country || "";
    document.querySelector('[name="timeFormat"]').value = settings.time.format;

    // Populate Alarms
    settings.alarms.forEach((alarm, index) => {
      addAlarmField(alarm.hour, alarm.minute, alarm.active);
    });

    // Populate Display Settings
    document.querySelector('[name="brightness"]').value =
      settings.display.brightness;
    document.querySelector('[name="color"]').value = settings.display.color;
  } catch (error) {
    showToast("Error loading settings!", "danger");
  }
}

function addAlarmField(hour = "", minute = "", active = false) {
  const div = document.createElement("div");
  div.className = "input-group mb-2";
  div.innerHTML = `
        <input type="number" class="form-control" placeholder="Hour" min="0" max="23" value="${hour}">
        <span class="input-group-text">:</span>
        <input type="number" class="form-control" placeholder="Minute" min="0" max="59" value="${minute}">
        <div class="input-group-text">
            <input class="form-check-input" type="checkbox" ${
              active ? "checked" : ""
            }>
        </div>
        <button class="btn btn-outline-danger" type="button" onclick="this.parentElement.remove()">×</button>
    `;
  document.getElementById("alarmFields").appendChild(div);
}

async function handleWifiSubmit(e) {
  e.preventDefault();
  const formData = {
    ssid: e.target.ssid.value,
    password: e.target.password.value,
  };

  try {
    await postData("/set-wifi", formData);
    showToast("WiFi settings saved!", "success");
  } catch (error) {
    showToast("Error saving WiFi!", "danger");
  }
}

// Similar handlers for other forms...

async function postData(url = "", data = {}) {
  const response = await fetch(url, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(data),
  });
  return response.json();
}

function showToast(message, type = "success") {
  const toast = document.createElement("div");
  toast.className = `toast align-items-center text-white bg-${type} border-0 show`;
  toast.innerHTML = `
        <div class="d-flex">
            <div class="toast-body">${message}</div>
            <button type="button" class="btn-close btn-close-white me-2 m-auto" data-bs-dismiss="toast"></button>
        </div>
    `;
  document.body.appendChild(toast);
  setTimeout(() => toast.remove(), 3000);
}
