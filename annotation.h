#ifndef ANNOTATION_H
#define ANNOTATION_H

#include<QVector>
#include<QPointF>
#include<QString>

#include "ecgsignal.h"
#include "point.h"

class Annotation
{
public:
	Annotation();
	~Annotation();

	void setSignal(const EcgSignal &inSignal);   //Доработать интерфейс !!!!!!11

	QVector<Point> points() const;
	QVector<QString> annotation() const;

	bool getAnnotation();

private:
	EcgSignal *signal;
	QVector<Point> _points;
	QVector<double> samples;
	QVector<int> peakType;
	QVector<QString> _annotation;

	bool findWaves();
	void analyzeWaves();
	void getPlotData();
};

inline Annotation::Annotation()
{
	signal = new EcgSignal;
}

inline Annotation::~Annotation()
{
	delete signal;
}

inline void Annotation::setSignal(const EcgSignal &inSignal)
{
	*signal = inSignal;
}

inline QVector<Point> Annotation::points() const
{
	return _points;
}

inline QVector<QString> Annotation::annotation() const
{
	return _annotation;
}

#endif // ANNOTATION_H
