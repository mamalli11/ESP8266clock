#include "AlarmManager.h"
#include "config.h"

AlarmManager::AlarmManager()
{
    // Initialize all alarms to default values
    for (uint8_t i = 0; i < MAX_ALARMS; i++)
    {
        alarms[i] = {0, 0, false};
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
            alarms[i].minute == timeInfo->tm_min)
        {
            triggerAlarm();
        }
    }
}

void AlarmManager::setAlarm(uint8_t index, uint8_t hour, uint8_t minute, bool active)
{
    if (index < MAX_ALARMS)
    {
        alarms[index] = {hour, minute, active};
        saveAlarms(); // Save changes to EEPROM
    }
}

Alarm AlarmManager::getAlarm(uint8_t index) const
{
    if (index < MAX_ALARMS)
    {
        return alarms[index];
    }
    return {0, 0, false}; // Return a default alarm if index is out of bounds
}

void AlarmManager::saveAlarms()
{
    EEPROM.begin(512);
    for (uint8_t i = 0; i < MAX_ALARMS; i++)
    {
        EEPROM.put(i * sizeof(Alarm), alarms[i]);
    }
    EEPROM.commit();
    EEPROM.end();
}

void AlarmManager::loadAlarms()
{
    EEPROM.begin(512);
    for (uint8_t i = 0; i < MAX_ALARMS; i++)
    {
        EEPROM.get(i * sizeof(Alarm), alarms[i]);
    }
    EEPROM.end();
}

void AlarmManager::triggerAlarm()
{
    // Activate the buzzer
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000); // Buzzer on for 1 second
    digitalWrite(BUZZER_PIN, LOW);
}