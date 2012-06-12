#include "annotation.h"
#include "Lib/lib.h"

#include <iostream>
#include <algorithm>
#include <QDebug>

bool Annotation::findWaves()
{
	int** qrsAnn;
	int** ANN;
	int annNum = 0;
	EcgAnnotation ann;
	int size = signal->size();
	int sampleRate = signal->sampleRate();
	QVector<double> data = signal->data();
	double *ecgSignal = new double [size];
	for(int i = 0; i < size; ++i)
		ecgSignal[i] = data[i];
	qrsAnn = ann.GetQRS(ecgSignal, size, sampleRate, L"filters");
	if (qrsAnn)
	{
		ann.GetEctopics(qrsAnn, ann.GetQrsNumber(), sampleRate);
		ANN = ann.GetPTU(ecgSignal, size, sampleRate, L"filters", qrsAnn, ann.GetQrsNumber());
		if (ANN)
		{
			annNum = ann.GetEcgAnnotationSize();
		}
		else
		{
			ANN = qrsAnn;
			annNum = 2 * ann.GetQrsNumber();
		}

		for (int i = 0; i < annNum; i++)
		{
			samples.append(ANN[i][0]);
			peakType.append(ANN[i][1]);
		}
        qDebug() << "annNum = " << annNum;

		return true;
	}
	else
	{
		std::cerr << "Epic Fail" << endl;
		return false;
	}
}

void Annotation::analyzeWaves()
{
	for (int i = 0; i < samples.count(); i++)
	{
		switch (peakType[i])
		{
		case 15: case 17:
			_points.append(Point(samples[i], Point::wt_QRS, Point::pt_PeakQ));
			break;
		case 24:
			_points.append(Point(samples[i], Point::wt_P, Point::pt_PeakP));
			break;
		case 27:
			_points.append(Point(samples[i], Point::wt_T, Point::pt_PeakT));
			break;
		case 29:
			_points.append(Point(samples[i], Point::wt_U, Point::pt_PeakU));
			break;
		case 39: case 1: case 46:
			_points.append(Point(samples[i], Point::wt_QRS, Point::pt_Start));
			break;
		case 40:
			_points.append(Point(samples[i], Point::wt_QRS, Point::pt_Finish));
			break;
		case 42:
			_points.append(Point(samples[i], Point::wt_P, Point::pt_Start));
			break;
		case 43:
			_points.append(Point(samples[i], Point::wt_P, Point::pt_Finish));
			break;
		case 44:
			_points.append(Point(samples[i], Point::wt_T, Point::pt_Start));
			break;
		case 45:
			_points.append(Point(samples[i], Point::wt_T, Point::pt_Finish));
			break;
		case 47: case 48:
			_points.append(Point(samples[i], Point::wt_QRS, Point::pt_PeakR));
			break;
		case 49: case 50:
			_points.append(Point(samples[i], Point::wt_QRS, Point::pt_PeakS));
			break;
		default:
			_points.append(Point(samples[i], Point::wt_Unknown, Point::pt_Unknown));
			break;
		}
	}
}

void Annotation::getPlotData()
{
	QVector <double> ecg_signal = signal->data();
	double* xCord = new double [signal->size()];
	double* yCord = new double [samples.size()];
	for (int i = 0; i < signal->size(); i++)
		xCord[i] = i;
	for (int i = 0; i < samples.size(); i++)
	{
		double* ptr;
		ptr = std::find(xCord, xCord + signal->size(), samples[i]);
		yCord[i] = ecg_signal[ptr - xCord];
		_points[i].setX(samples[i]);
		_points[i].setY(yCord[i]);
	}
	delete [] xCord;
	delete [] yCord;
}

bool Annotation::getAnnotation()
{
	if (findWaves())
	{
		analyzeWaves();
		getPlotData();
		for (int i = 0; i < _points.count(); i++)
		{
			switch (_points[i].pointtype())
			{
			case Point::pt_PeakQ:
				_annotation.push_back("Q");
				break;
			case Point::pt_PeakP:
				_annotation.push_back("P");
				break;
			case Point::pt_PeakT:
				_annotation.push_back("T");
				break;
			case Point::pt_PeakU:
				_annotation.push_back("U");
				break;
			case Point::pt_Start:
				_annotation.push_back("b");
				break;
			case Point::pt_Finish:
				_annotation.push_back("e");
				break;
			case Point::pt_PeakR:
				_annotation.push_back("R");
				break;
			case Point::pt_PeakS:
				_annotation.push_back("S");
				break;
			default:
				_annotation.push_back("Unknown");
				break;
			}
		}
		return true;
	}
	else
		return false;
}
