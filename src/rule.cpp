/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "rule_p.h"

#include <QDateTime>

using namespace KOpeningHours;

QDebug operator<<(QDebug debug, const Time &time)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << time.hour << ':' << time.minute;
    if (time.event != Time::NoEvent) {
        debug.nospace() << "[Event" << time.event << ']';
    }
    return debug;
}

int Timespan::requiredCapabilities() const
{
    if (begin.event != Time::NoEvent || end.event != Time::NoEvent) {
        return Capability::Location;
    }
    return next ? next->requiredCapabilities() : Capability::None;
}

Interval Timespan::nextInterval(const Interval &interval) const
{
    auto i = interval;
    i.setBegin(QDateTime(interval.begin().date(), {begin.hour, begin.minute}));
    i.setEnd(QDateTime(interval.begin().date(), {end.hour, end.minute}));
    return i;
}

QDebug operator<<(QDebug debug, Timespan *timeSpan)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "T " << timeSpan->begin << '-' << timeSpan->end;
    if (timeSpan->next) {
        debug.nospace() << ", " << timeSpan->next.get();
    }
    return debug;
}

int WeekdayRange::requiredCapabilities() const
{
    switch (holiday) {
        case NoHoliday:
            return (next ? next->requiredCapabilities() : Capability::None) | (next2 ? next2->requiredCapabilities() : Capability::None);
        case PublicHoliday:
            return Capability::PublicHoliday;
        case SchoolHoliday:
            return Capability::SchoolHoliday;
    }
    return Capability::NotImplemented;
}

QDebug operator<<(QDebug debug, WeekdayRange *weekdayRange)
{
    debug << "WD" << weekdayRange->beginDay << weekdayRange->endDay << weekdayRange->nthMask << weekdayRange->offset << weekdayRange->holiday;
    if (weekdayRange->next) {
        debug << "  " << weekdayRange->next.get();
    }
    if (weekdayRange->next2) {
        debug << "  " << weekdayRange->next2.get();
    }
    return debug;
}

QDebug operator<<(QDebug debug, Week *week)
{
    debug.nospace() << "W " << week->beginWeek << '-' << week->endWeek << '/' << week->interval;
    if (week->next) {
        debug << ", " << week->next.get();
    }
    return debug;
}

QDebug operator<<(QDebug debug, const Date &date)
{
    switch (date.variableDate) {
        case Date::FixedDate:
            debug.nospace() << date.year << '-' << date.month << '-' << date.day;
            break;
        case Date::Easter:
            debug << "easter";
            break;
    }
    return debug;
}

QDebug operator<<(QDebug debug, MonthdayRange *monthdayRange)
{
    debug.nospace() << "M " << monthdayRange->begin << '-' << monthdayRange->end;
    if (monthdayRange->next) {
        debug << ", " << monthdayRange->next.get();
    }
    return debug;
}

void Rule::setComment(const char *str, int len)
{
    m_comment = QString::fromUtf8(str, len);
}

int Rule::requiredCapabilities() const
{
    int c = Capability::None;
    c |= m_timeSelector ? m_timeSelector->requiredCapabilities() : Capability::None;
    c |= m_weekdaySelector ? m_weekdaySelector->requiredCapabilities() : Capability::None;

    return c;
}

Interval Rule::nextInterval(const QDateTime &dt) const
{
    Interval i;
    // ### temporary
    i.setBegin(QDateTime(dt.date(), {}));
    i.setEnd(QDateTime(dt.date(), {}));

    if (m_timeSelector) {
        i = m_timeSelector->nextInterval(i);
    }

    i.setState(m_state);
    return i;
}
