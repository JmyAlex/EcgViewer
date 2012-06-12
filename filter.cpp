#include "filter.h"
#include "Lib/lib.h"

//Filter::Filter()
//{
//}

bool Filter::filterSignal()
{
    QVector<double> data = _signal->data();
    int size = _signal->size();
    int sampleRate = _signal->sampleRate();
    double *signal = new double [size];
    for(int i = 0; i < size; ++i)
        signal[i] = data[i];
    EcgDenoise enoise;
    enoise.InitDenoise(L"filters", signal, size, sampleRate);
    enoise.LFDenoise();             //baseline wander removal
    enoise.HFDenoise();             //high frequency noise removal
    enoise.LFHFDenoise();           //both noises removal
    enoise.CloseDenoise();
    data.clear();
    for (int i = 0; i < size; i++)
    {
       data.push_back(signal[i]);
    }
    delete _signal;
    _signal = new EcgSignal;
    _signal->setData(data);
    delete signal;
    return true;
}
