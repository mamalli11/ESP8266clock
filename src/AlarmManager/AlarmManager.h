#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include <Arduino.h>
#include <EEPROM.h>

struct Alarm
{
    uint8_t hour;
    uint8_t minute;
    bool active;
};

class AlarmManager
{
public:
    AlarmManager();
    void begin();
    void checkAlarms(const tm *timeInfo);
    void setAlarm(uint8_t index, uint8_t hour, uint8_t minute, bool active);
    Alarm getAlarm(uint8_t index) const;
    void saveAlarms();
    void loadAlarms();

private:
    static const uint8_t MAX_ALARMS = 3; // Maximum number of alarms
    Alarm alarms[MAX_ALARMS];

    void triggerAlarm();
};

#endif