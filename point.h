#ifndef POINT_H
#define POINT_H

#include <QPoint>

class Point : public QPointF
{
public:
	enum waveType { wt_Unknown, wt_P, wt_QRS, wt_T, wt_U, wt_Cycle };
	enum pointType
	{
		pt_Unknown,
		pt_Start,
		pt_PeakP,
		pt_PeakQ,
		pt_PeakR,
		pt_PeakS,
		pt_PeakT,
		pt_PeakU,
		pt_Finish
	};

    Point();
	Point(const Point &other);
	Point(double sample, waveType wavetype, pointType pointtype);

	void setSample(const double sample);
	void setWaveType(const waveType wavetype);
	void setPointType(const pointType pointtype);

	double sample() const;
	waveType wavetype() const;
	pointType pointtype () const;

private:
	//int peakType;
	double _sample;
	waveType _wavetype;
	pointType _pointtype;
};

inline Point::Point()
{
	_sample = 0;
	_wavetype = wt_Unknown;
	_pointtype = pt_Unknown;
}

inline Point::Point(const Point &other)
{
	_sample = other.sample();
	_wavetype = other.wavetype();
	_pointtype = other.pointtype();
}

inline Point::Point(double sample, waveType wavetype, pointType pointtype)
	: _sample(sample), _wavetype(wavetype), _pointtype(pointtype)
{

}

inline void Point::setSample(const double sample)
{
	_sample = sample;
}

inline void Point::setWaveType(const waveType wavetype)
{
	_wavetype = wavetype;
}

inline void Point::setPointType(const pointType pointtype)
{
	_pointtype = pointtype;
}

inline double Point::sample() const
{
	return _sample;
}

inline Point::waveType Point::wavetype() const
{
	return _wavetype;
}

inline Point::pointType Point::pointtype() const
{
	return _pointtype;
}

#endif // POINT_H
