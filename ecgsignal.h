#ifndef ECGSIGNAL_H
#define ECGSIGNAL_H

#include <QVector>
#include <QString>
#include <QFile>

class EcgSignal    //Rename to SignalData ? 
{
public:
    EcgSignal();   //Move to private ?
	EcgSignal(const EcgSignal &other);
	EcgSignal(const QVector<double> &data, const int sampleRate);
	~EcgSignal();

	void setSize(const int size);
	void setSampleRate(const int sampleRate);
	void setData(const QVector<double> &data);

	int size() const;
	int sampleRate() const;
	QVector<double> data() const;

private:
    //int id;
	int _size;
	int _sampleRate;
	QVector<double> _data;

};

inline EcgSignal::EcgSignal() : _size(0), _sampleRate(1000)
{

}

inline EcgSignal::EcgSignal(const EcgSignal &other)
{
	_data = other.data();
	_sampleRate = other.sampleRate();
	_size = other.size();
}

inline EcgSignal::EcgSignal(const QVector<double> &data, const int sampleRate)
{
	_data = data;
	_sampleRate = sampleRate;
}

inline EcgSignal::~EcgSignal()
{
	_data.clear();
}

inline void EcgSignal::setSize(const int size)
{
	_size = size;
}

inline void EcgSignal::setSampleRate(const int sampleRate)
{
	_sampleRate = sampleRate;
}

inline void EcgSignal::setData(const QVector<double> &data)
{
	_data = data;
}

inline int EcgSignal::size() const
{
	return _size;
}

inline int EcgSignal::sampleRate() const
{
	return _sampleRate;
}

inline QVector<double> EcgSignal::data() const
{
	return _data;
}


#endif // ECGSIGNAL_H
