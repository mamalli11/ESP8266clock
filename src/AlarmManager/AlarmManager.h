#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include "Config.h"
#include <Arduino.h>
#include <EEPROM.h>

struct Alarm
{
    uint8_t hour;
    uint8_t minute;
    bool active;
    char name[20];   // نام آلارم (اختیاری)
    uint8_t days[7]; // روزهایی که آلارم فعال است (0: غیرفعال، 1: فعال)
};

class AlarmManager
{
public:
    AlarmManager();
    void begin();
    void checkAlarms(const tm *timeInfo);
    void setAlarm(uint8_t index, uint8_t hour, uint8_t minute, bool active, const char *name, const uint8_t days[7]);
    Alarm getAlarm(uint8_t index) const;
    void saveAlarms();
    void loadAlarms();

private:
    Alarm alarms[MAX_ALARMS];
    void triggerAlarm(const Alarm &alarm);
};

#endif