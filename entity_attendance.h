#ifndef ENTITY_ATTENDANCE_H
#define ENTITY_ATTENDANCE_H

#include <QString>
#include "entity_common.h"

class Attendance {
public:
    QString id;
    int late;
    int exit_early;
    date time;
    int overtime;

    Attendance(const QString& id, int late, int exit_early, const QString& year, const QString& mon, int overtime)
        : id(id), late(late), exit_early(exit_early), time(year, mon), overtime(overtime) {}
    Attendance() = default;
    ~Attendance() = default;
};

#endif // ENTITY_ATTENDANCE_H
