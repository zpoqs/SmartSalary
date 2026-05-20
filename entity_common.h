#ifndef ENTITY_COMMON_H
#define ENTITY_COMMON_H

#include <QString>

class date {
public:
    QString year, mon;
    date(const QString& year, const QString& mon) : year(year), mon(mon) {}
    date() = default;
    ~date() = default;
};

#endif // ENTITY_COMMON_H
