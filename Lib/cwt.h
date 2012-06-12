

#ifndef CWT_h
#define CWT_h

typedef struct _cwthdr {
        char hdr[4];
        float fmin;
        float fmax;
        float fstep;
        unsigned int size;
        float sr;
        unsigned char type;   //1-log; 0-norm scale

        char rsrv[15];
} CWTHDR, *PCWTHDR;

class Signal;

class CWT : public Signal
{
public:
        CWT();
        //CWT(const CWT& cwt);
        ~CWT();

// Data
        enum WAVELET {MHAT, INV, MORL, MORLPOW, MORLFULL, GAUS, GAUS1, GAUS2, GAUS3, GAUS4, GAUS5, GAUS6, GAUS7};
        enum SCALETYPE {LINEAR_SCALE, LOG_SCALE};

// Operators
        //const CWT& operator=(const CWT& cwt);

// Operations
        float* CwtCreateFileHeader(wchar_t *name, PCWTHDR hdr, enum WAVELET wavelet, double w);
        float* CwtReadFile(const wchar_t *name);
        double HzToScale(double f, double sr, enum WAVELET wavelet, double w) const;
        void ConvertName(wchar_t *name, enum WAVELET wavelet, double w) const;

        void InitCWT(int size, enum WAVELET wavelet, double w, double sr);
        void CloseCWT();
        double* CwtTrans(const double *data, double freq, bool periodicboundary = true, double lv = 0, double rv = 0);

// Access
        inline double GetMinFreq() const;
        inline double GetMaxFreq() const;
        inline double GetFreqInterva() const;
        inline int GetScaleType() const;         
        inline int GetFreqRange() const;

// Inquiry

protected:
private:
        CWT(const CWT& cwt);
        const CWT& operator=(const CWT& cwt);

        inline double CwtTrans(int x, double scale);

        PCWTHDR phdr;

        double MinFreq;
        double MaxFreq;
        double FreqInterval;
        double w0;        
        
        enum SCALETYPE ScaleType;        
        enum WAVELET Wavelet;                   //Wavelet

        int SignalSize; 
        const double *pData;               //pointer to original signal
        double *pCwtSpectrum;              //buffer with spectra
        double *pReal, *pImag;             //Re,Im parts of wavelet
        
        bool IsPrecision; 
        int PrecisionSize; 

        bool IsPeriodicBoundary;                //periodic boundary extension
        double LeftVal, RightVal;          //left/right value for signal ext at boundaries


};

/*//////////////////////////////////////////////
        CWT *cwt;
        cwt->InitCWT(size, CWT::MHAT, w0, SR);
        pCwtSpectrum = cwt->CwtTrans(signal, freq);
        cwt->CloseCWT();
//////////////////////////////////////////////*/

// Inlines
inline double CWT::CwtTrans(int x, double scale)
{
        double res = 0, Re = 0, Im = 0;

        for (int t = 0; t < SignalSize; t++) {                   //main
                if (IsPrecision == true) {
                        if (t < x - PrecisionSize) 
                                t = x - (PrecisionSize - 1);  //continue;
                        if (t >= PrecisionSize + x) 
                                break;
                }

                Re += pReal[((SignalSize-1)-x) + t] * pData[t];
                if (Wavelet == MORLPOW || Wavelet == MORLFULL)
                        Im += pImag[((SignalSize-1)-x) + t] * pData[t];
        }
        
        ////////////////////boundaries///////////////////////////////////////////////
        int p = 0;
        for (int i = (SignalSize - PrecisionSize); i < (SignalSize - 1) - x; i++) {        // Left edge calculations
                if (IsPeriodicBoundary) {
                        Re += pReal[i] * pData[(SignalSize-1)-i-x];  //IsPeriodicBoundary
                } else {
                        if (LeftVal != 0.0)
                                Re += pReal[i] * LeftVal;
                        else
                                Re += pReal[i] * pData[0];
                }

                if (Wavelet == MORLPOW || Wavelet == MORLFULL) { //Im part for complex wavelet
                        if (IsPeriodicBoundary) {
                                Im += pImag[i] * pData[(SignalSize-1)-i-x];
                        } else {
                                if (LeftVal != 0.0)
                                        Im += pImag[i] * LeftVal;
                                else
                                        Im += pImag[i] * pData[0];
                        }
                }
        }
        int q = 0;
        for (int i = 2 * SignalSize - (x + 1); i < SignalSize + PrecisionSize - 1; i++) {     // Right edge calculations
                if (IsPeriodicBoundary)
                        Re += pReal[i] * pData[(SignalSize-2)-q]; //IsPeriodicBoundary
                else {
                        if (RightVal != 0.0)
                                Re += pReal[i] * RightVal;
                        else
                                Re += pReal[i] * pData[SignalSize-1];
                }

                if (Wavelet == MORLPOW || Wavelet == MORLFULL) {
                        if (IsPeriodicBoundary) {
                                Im += pImag[i] * pData[(SignalSize-2)-q];
                        } else {
                                if (RightVal != 0.0)
                                        Im += pImag[i] * RightVal;
                                else
                                        Im += pImag[i] * pData[SignalSize-1];
                        }
                }
                q++;
        }
        ////////////////////boundaries///////////////////////////////////////////////


        switch (Wavelet) {
        case MORL:
                res = (1 / sqrt(6.28)) * Re;
                break;
        case MORLPOW:
                res = sqrt(Re * Re + Im * Im);
                res *= (1 / sqrt(6.28));
                break;
        case MORLFULL:
                res = sqrt(Re * Re + Im * Im);
                res *= (1 / pow(3.14, 0.25));
                break;

        default:
                res = Re;
        }

        res = (1 / sqrt(scale)) * res;

        return res;
}

inline int CWT::GetFreqRange() const
{
        if (ScaleType == LINEAR_SCALE)
                return (int)((MaxFreq + FreqInterval - MinFreq) / FreqInterval);
        else if (ScaleType == LINEAR_SCALE)
                return (int)((log(MaxFreq) + FreqInterval - log(MinFreq)) / FreqInterval);
        else 
                return 0;
}

inline double CWT::GetMinFreq() const
{
        return MinFreq;
}

inline double CWT::GetMaxFreq() const
{
        return MaxFreq;
}

inline double CWT::GetFreqInterva() const 
{
        return FreqInterval;
}

inline int CWT::GetScaleType() const
{
        return ScaleType;
}

#endif CWT_h

