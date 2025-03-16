#include "AlarmManager.h"

AlarmManager::AlarmManager()
{
    for (uint8_t i = 0; i < MAX_ALARMS; i++)
    {
        alarms[i].hour = 0;
        alarms[i].minute = 0;
        alarms[i].active = false;
        memset(alarms[i].name, 0, sizeof(alarms[i].name)); // مقداردهی رشته
        memset(alarms[i].days, 0, sizeof(alarms[i].days)); // مقداردهی آرایه روزها
    }
}

void AlarmManager::begin()
{
    loadAlarms(); // Load alarms from EEPROM
}

void AlarmManager::checkAlarms(const tm *timeInfo)
{
    for (uint8_t i = 0; i < MAX_ALARMS; i++)
    {
        if (alarms[i].active &&
            alarms[i].hour == timeInfo->tm_hour &&
            alarms[i].minute == timeInfo->tm_min &&
            alarms[i].days[timeInfo->tm_wday] == 1) // بررسی فعال بودن در روز جاری
        {
            triggerAlarm(alarms[i]);
        }
    }
}

void AlarmManager::setAlarm(uint8_t index, uint8_t hour, uint8_t minute, bool active, const char *name, const uint8_t days[7])
{
    if (index < MAX_ALARMS)
    {
        alarms[index].hour = hour;
        alarms[index].minute = minute;
        alarms[index].active = active;
        strncpy(alarms[index].name, name, sizeof(alarms[index].name) - 1);
        alarms[index].name[sizeof(alarms[index].name) - 1] = '\0'; // اطمینان از پایان رشته
        memcpy(alarms[index].days, days, sizeof(alarms[index].days));

        saveAlarms();
    }
}

Alarm AlarmManager::getAlarm(uint8_t index) const
{
    Alarm defaultAlarm;
    defaultAlarm.hour = 0;
    defaultAlarm.minute = 0;
    defaultAlarm.active = false;
    memset(defaultAlarm.name, 0, sizeof(defaultAlarm.name));
    memset(defaultAlarm.days, 0, sizeof(defaultAlarm.days));

    if (index < MAX_ALARMS)
    {
        return alarms[index];
    }
    return defaultAlarm;
}

void AlarmManager::saveAlarms()
{
    // EEPROM.begin(EEPROM_SIZE);
    for (uint8_t i = 0; i < MAX_ALARMS; i++)
    {
        EEPROM.put(i * sizeof(Alarm), alarms[i]);
    }
    EEPROM.commit();
    EEPROM.end();
}

void AlarmManager::loadAlarms()
{
    // EEPROM.begin(EEPROM_SIZE);
    for (uint8_t i = 0; i < MAX_ALARMS; i++)
    {
        Alarm alarm;
        EEPROM.get(350 + (i * sizeof(Alarm)), alarm);

        // مقداردهی پیش‌فرض برای جلوگیری از داده‌های نادرست
        if (alarm.hour > 23)
            alarm.hour = 0;
        if (alarm.minute > 59)
            alarm.minute = 0;
        if (alarm.active > 1)
            alarm.active = false;
        for (uint8_t j = 0; j < 7; j++)
        {
            if (alarm.days[j] > 1)
                alarm.days[j] = 0;
        }
        if (alarm.name[0] == 255)
            memset(alarm.name, 0, sizeof(alarm.name));

        alarms[i] = alarm;
    }
    EEPROM.end();
}

void AlarmManager::triggerAlarm(const Alarm &alarm)
{
    Serial.print("Alarm Triggered: ");
    Serial.println(alarm.name);

    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
}