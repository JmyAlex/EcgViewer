#ifndef SCALE_H
#define SCALE_H

#include <QVector>
#include <cmath>

class Scale
{
public:
    Scale();
    Scale(const double &min, const double &max, const double &increment, int minorTicks = 1);
    Scale(const Scale &other);

    void setMin(const double &min);
    void setMax(const double &max);
    void setIncrement(const double &val);
    void setMinorTicks(int val);
    double min() const;
    double max() const;
    double increment() const;
    int minorTicks() const;
    double width() const;
    QVector<double> vectorOfMajorTics() const;
    QVector<double> vectorOfMinorTics() const;
    void adjust();

    bool operator==(const Scale &scale) const;
    bool operator!=(const Scale &scale) const;

    friend inline void scrollScale(Scale &scale, int dx);

private:
    double _min, _max;
    double _increment;
    int _minorTicks;
};

inline Scale::Scale()
{
    _min = 0.0;
    _max = 1.0;
    _increment = 0.1 * 1000;
    _minorTicks = 1;
}
//-----------------------------------------------------------------------------

inline Scale::Scale(const Scale &other)
{
    _min = other.min();
    _max = other.max();
    _increment = other.increment();
    _minorTicks = other.minorTicks();
}
//-----------------------------------------------------------------------------

inline Scale::Scale(const double &min, const double &max, const double &increment, int minorTicks)
    : _min(min), _max(max), _increment(increment), _minorTicks(minorTicks)
{
    if (_increment == 0.0)
        _increment = 0.1;
}
//-----------------------------------------------------------------------------

inline QVector<double> Scale::vectorOfMajorTics() const
{
    int nStart = (int)ceil(_min/_increment);
    int nEnd = (int)floor(_max/_increment);
    QVector<double> vTicks;
    for (int i = nStart; i <= nEnd; ++i) {
        vTicks.append(double(i * _increment));
    }
    return vTicks;
}
//-----------------------------------------------------------------------------

inline QVector<double> Scale::vectorOfMinorTics() const
{
    const double dx = _increment/(_minorTicks + 1);
    int nStart = (int)ceil(_min/dx);
    int nEnd = (int)floor(_max/dx);
    QVector<double> vMinorTicks;
    for (int i = nStart; i <= nEnd; ++i) {
        vMinorTicks.append(double(i * dx));
    }
    return vMinorTicks;
}
//-----------------------------------------------------------------------------

inline void Scale::adjust()
{
    const int MinTicks = 5;
    double grossStep = (_max - _min) / MinTicks;
    _increment = pow(10.0, floor(log10(grossStep)));

    if (5 * _increment < grossStep) {
        _increment *= 5;
    } else if (2 * _increment < grossStep) {
        _increment *= 2;
    }
}
//-----------------------------------------------------------------------------

inline void Scale::setMin(const double &min)
{
    _min = min;
}
//-----------------------------------------------------------------------------

inline void Scale::setMax(const double &max)
{
    _max = max;
}
//-----------------------------------------------------------------------------

inline void Scale::setIncrement(const double &val)
{
    if (val > 0.0)
        _increment = val;
}
//-----------------------------------------------------------------------------

inline void Scale::setMinorTicks(int val)
{
    if (val >= 0)
        _minorTicks = val;
}
//-----------------------------------------------------------------------------

inline double Scale::min() const
{
    return _min;
}
//-----------------------------------------------------------------------------

inline double Scale::max() const
{
    return _max;
}
//-----------------------------------------------------------------------------

inline double Scale::width() const
{
    return _max - _min;
}
//-----------------------------------------------------------------------------

inline double Scale::increment() const
{
    return _increment;
}
//-----------------------------------------------------------------------------

inline int Scale::minorTicks() const
{
    return _minorTicks;
}
//-----------------------------------------------------------------------------

inline bool Scale::operator==(const Scale &scale) const
{
    return ((scale.increment() == _increment) &&
            (scale.max() == _max) &&
            (scale.min() == _min) &&
            (scale.minorTicks() == _minorTicks));
}
//-----------------------------------------------------------------------------

inline bool Scale::operator!=(const Scale &scale) const
{
    return ((scale.increment() != _increment) ||
            (scale.max() != _max) ||
            (scale.min() != _min) ||
            (scale.minorTicks() != _minorTicks));
}
//-----------------------------------------------------------------------------

inline void scrollScale(Scale &scale, int dx)
{
    scale._min += dx * scale._increment;
    scale._max += dx * scale._increment;
}
//-----------------------------------------------------------------------------

#endif // SCALE_H
