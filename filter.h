#ifndef FILTER_H
#define FILTER_H

#include "ecgsignal.h"
#include <QVector>

class Filter
{
public:
    Filter();
    Filter(const Filter &other);
    Filter(const EcgSignal &signal);  //Убрать или переделать!!

    bool filterSignal();

    void setSignal(const EcgSignal &signal);
    EcgSignal getSignal() const;
    QVector<double> data() const;

private:
    EcgSignal *_signal;

};

inline Filter::Filter()
{
    _signal = new EcgSignal;
}

inline Filter::Filter(const Filter &other)
{
    *_signal = other.getSignal();
}

inline Filter::Filter(const EcgSignal &signal)
{
    *_signal = signal;
}

inline void Filter::setSignal(const EcgSignal &signal)
{
    *_signal = signal;
}

inline EcgSignal Filter::getSignal() const
{
    return *_signal;
}

inline QVector<double> Filter::data() const
{
    return _signal->data();
}

#endif // FILTER_H
