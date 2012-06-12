

#ifndef ECGDenosie_h
#define ECGDenosie_h

class Signal;
class FWT;

class EcgDenoise : public FWT
{
public:
        EcgDenoise();
        //EcgDenoise(const EcgDenoise& ecgdenoise);
        ~EcgDenoise();

// Operators
        //const EcgDenoise& operator=(const EcgDenoise& ecgdenoise);

// Operations
        void InitDenoise(const wchar_t* fltdir, double* data, int size, double sr, bool mirror = true);
        void CloseDenoise();

        bool LFDenoise();         //baseline wander removal
        bool HFDenoise();         //hf denoising
        bool LFHFDenoise();       //baseline and hf denoising

// Access
// Inquiry

protected:
private:
        EcgDenoise(const EcgDenoise& ecgdenoise);
        const EcgDenoise& operator=(const EcgDenoise& ecgdenoise);

        wchar_t FiltersDir[_MAX_PATH];             //filters dir

        double* pEcgData;            //pointer to [original sig]
        double* pTmpData;           //[SRadd][original sig][SRadd]

};

/*
        wchar_t *dir = "c:\\dir_for_fwt_filters\\filters";
        double *sig;   //signal massive
        double SR;     //sampling rate of the signal
        int size;   //size of the signal

        EcgDenoise enoise;

        enoise.InitDenoise(dir, sig, size, SR);
        enoise.LFDenoise();     //baseline wander removal
        //or  enoise.HFDenoise();    //high frequency noise removal
        //or  enoise.LFHFDenoise();  //both noises removal
        enoise.CloseDenoise();
*/

// Inlines


#endif ECGDenosie_h

