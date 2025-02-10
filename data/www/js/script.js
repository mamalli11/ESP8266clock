document.addEventListener("DOMContentLoaded", () => {
  console.log("Started DOMContentLoaded");
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

// **بارگذاری تنظیمات هنگام لود شدن صفحه**
async function loadCurrentSettings() {
  try {
    const response = await fetch("/api/get/config");
    const settings = await response.json();

    // WiFi Settings
    document.querySelector('[name="ssid"]').value = settings.wifi.ssid || "";
    document.querySelector('[name="password"]').value =
      settings.wifi.password || "";

    // Time Settings
    document.querySelector('[name="city"]').value = settings.time.city || "";
    document.querySelector('[name="country"]').value =
      settings.time.country || "";
    document.querySelector('[name="timeFormat"]').value = settings.time["24h"]
      ? "24"
      : "12";

    // Display Settings
    document.querySelector('[name="brightness"]').value =
      settings.display.brightness;
    document.querySelector('[name="color"]').value = settings.display.theme;

    // Alarm Settings
    document.getElementById("alarmFields").innerHTML = ""; // حذف مقادیر قبلی
    if (Array.isArray(settings.alarms)) {
      settings.alarms.forEach((alarm) =>
        addAlarmField(
          alarm.name,
          alarm.hour,
          alarm.minute,
          alarm.active,
          alarm.days
        )
      );
    }

    console.log(settings);
    showToast("Settings loaded successfully!", "success");
  } catch (error) {
    showToast("Error loading settings!", "danger");
    console.error("Error fetching settings:", error);
  }
}

// **افزودن فیلد جدید برای آلارم**
function addAlarmField(
  name = "",
  hour = "",
  minute = "",
  active = false,
  days = [0, 0, 0, 0, 0, 0, 0]
) {
  const div = document.createElement("div");
  div.className = "input-group mb-2";
  div.innerHTML = `
        <input type="text" class="form-control name" placeholder="Name" value="${name}">
        <input type="number" class="form-control hour" placeholder="Hour" min="0" max="23" required value="${hour}">
        <span class="input-group-text">:</span>
        <input type="number" class="form-control minute" placeholder="Minute" min="0" max="59" required value="${minute}">
        <div class="input-group-text">
            <input class="form-check-input active" type="checkbox" ${
              active ? "checked" : ""
            }>
        </div>
        <div class="days">
            ${["Sat", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri"]
              .map(
                (day, index) => `
                <label class="form-check-label">
                    <input type="checkbox" class="day" data-day="${index}" ${
                  days[index] ? "checked" : ""
                }> ${day}
                </label>
            `
              )
              .join(" ")}
        </div>
        <button class="btn btn-outline-danger" type="button" onclick="this.parentElement.remove()">×</button>
    `;
  document.getElementById("alarmFields").appendChild(div);
}

// **ارسال تنظیمات WiFi**
async function handleWifiSubmit(e) {
  e.preventDefault();

  const formData = {
    ssid: e.target.ssid.value,
    password: e.target.password.value,
  };

  try {
    await postData("/api/update/wifi", formData);
    showToast("WiFi settings saved! Restarting...", "success");
  } catch (error) {
    showToast("Error saving WiFi!", "danger");
  }
}

// **ارسال تنظیمات زمان**
async function handleTimeSubmit(e) {
  e.preventDefault();

  const formData = {
    city: e.target.city.value,
    country: e.target.country.value,
    timeFormat: e.target.timeFormat.value,
  };

  try {
    await postData("/api/update/time", formData);
    showToast("Time settings saved!", "success");
  } catch (error) {
    console.log(error);
    showToast("Error saving time settings!", "danger");
  }
}

// **ارسال تنظیمات آلارم**
async function handleAlarmSubmit(e) {
  e.preventDefault();

  let alarms = [];
  document.querySelectorAll("#alarmFields .input-group").forEach((alarmDiv) => {
    let alarm = {
      name: alarmDiv.querySelector(".name").value,
      hour: parseInt(alarmDiv.querySelector(".hour").value),
      minute: parseInt(alarmDiv.querySelector(".minute").value),
      active: alarmDiv.querySelector(".active").checked,
      days: [...alarmDiv.querySelectorAll(".day")].map((d) =>
        d.checked ? 1 : 0
      ),
    };
    alarms.push(alarm);
  });

  try {
    const response = await fetch("/api/update/alarms", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(alarms),
    });

    const text = await response.text();
    console.log("Response:", text);
    showToast("Alarms saved successfully!", "success");
  } catch (error) {
    console.error("Error saving alarms:", error);
    showToast("Error saving alarms!", "danger");
  }
}

// **ارسال تنظیمات نمایشگر**
async function handleDisplaySubmit(e) {
  e.preventDefault();

  const formData = {
    brightness: e.target.brightness.value,
    theme: e.target.color.value,
  };

  try {
    await postData("/api/update/display", formData);
    showToast("Display settings saved!", "success");
  } catch (error) {
    showToast("Error saving display settings!", "danger");
  }
}

// **ارسال درخواست POST**
async function postData(url = "", data) {
  const response = await fetch(url, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(data),
  });
  // const content = await response.json();
  console.log(response);

  return response.text();
}

// **نمایش پیغام‌های Toast**
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

function toggle_showPassword() {
  var x = document.getElementById("Inputpassword");
  if (x.type === "password") {
    x.type = "text";
  } else {
    x.type = "password";
  }
}
