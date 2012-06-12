

//#include "stdafx.h"
#include "signal.h"
#include "fwt.h"
#include "ecgdenoise.h"


EcgDenoise::EcgDenoise() : pTmpData(0)
{
}

EcgDenoise::~EcgDenoise()
{
        if (pTmpData != 0) 
                free(pTmpData);
}

void EcgDenoise::InitDenoise(const wchar_t* fltdir, double* data, int size, double sr, bool mirror)
{
        wcscpy(FiltersDir, fltdir);

        pEcgData = data;
        SR = sr;
        Length = size;

        if (pTmpData) free(pTmpData);
        pTmpData = (double *)malloc(sizeof(double) * (Length + 2 * SR));    // [SR add] [sig] [SR add]

        for (int i = 0; i < Length; i++)           //signal
                pTmpData[i+(int)SR] = pEcgData[i];

        if (Length < SR) mirror = false;

        if (mirror) { //mirror extension of signal
                for (int i = 0; i < (int)SR; i++)          //left side
                        pTmpData[i] = pEcgData[(int)SR-i];
                for (int i = Length + SR; i < Length + 2*SR; i++)         //right side
                        pTmpData[i] = pEcgData[(Length-2) - (i-(Length+(int)SR))];
        } else {
                for (int i = 0; i < (int)SR; i++)          //left side
                        pTmpData[i] = pEcgData[0];
                for (int i = Length + SR; i < Length + 2*SR; i++)         //right side
                        pTmpData[i] = pEcgData[Length-1];
        }
}

void EcgDenoise::CloseDenoise()
{
        if (pTmpData) {
                free(pTmpData);
                pTmpData = 0;
        }
}

bool  EcgDenoise::LFDenoise()
{
        wchar_t filter[_MAX_PATH];


        //get base line J///////
        int J = ceil(log2(SR / 0.8)) - 1;

        wcscpy(filter, FiltersDir);
        wcscat(filter, L"\\daub2.flt");
        if (InitFWT(filter, pTmpData, Length + 2*SR) == false)
                return false;


        //transform////////////////////////
        FwtTrans(J);
        ///////////////////////////////////


        int *Jnumbs = GetJnumbs(J, Length + 2 * SR);
        double *lo = GetFwtSpectrum();
        for (int i = 0; i < Jnumbs[0]; i++)
                lo[i] = 0.0;


        //synth/////////////////////////////
        FwtSynth(J);
        ////////////////////////////////////

        for (int i = 0; i < Length; i++)
                pEcgData[i] = lo[i+(int)SR];


        CloseFWT();
        return true;
}

bool EcgDenoise::HFDenoise()
{
        wchar_t filter[_MAX_PATH];


        //get HF scale J///////
        int J = ceil(log2(SR / 23.0)) - 2;     //[30Hz - ...] hf denoising

        wcscpy(filter, FiltersDir);
        wcscat(filter, L"\\bior97.flt");
        if (InitFWT(filter, pTmpData, Length + 2*SR) == false)
                return false;


        //transform////////////////////////
        FwtTrans(J);
        ///////////////////////////////////


        int *Jnumbs = GetJnumbs(J, Length + 2 * SR);
        int hinum, lonum;
        HiLoNumbs(J, Length + 2*SR, hinum, lonum);
        double *lo = GetFwtSpectrum();
        double *hi = GetFwtSpectrum() + (int(Length + 2 * SR) - hinum);

        int window;   //3sec window
        for (int j = J; j > 0; j--) {
                window = 3.0 * SR / pow(2.0, (double)j);

                Denoise(hi, Jnumbs[J-j], window);
                hi += Jnumbs[J-j];
        }



        //synth/////////////////////////////
        FwtSynth(J);
        ////////////////////////////////////

        for (int i = 0; i < Length; i++)
                pEcgData[i] = lo[i+(int)SR];


        CloseFWT();
        return true;
}

bool EcgDenoise::LFHFDenoise()
{
        wchar_t filter[_MAX_PATH];
        int J, *Jnumbs;
        double *lo, *hi;


        //get base line J///////
        J = ceil(log2(SR / 0.8)) - 1;

        wcscpy(filter, FiltersDir);
        wcscat(filter, L"\\daub2.flt");
        if (InitFWT(filter, pTmpData, Length + 2*SR) == false)
                return false;


        //transform////////////////////////
        FwtTrans(J);
        ///////////////////////////////////

        Jnumbs = GetJnumbs(J, Length + 2 * SR);
        lo = GetFwtSpectrum();
        for (int i = 0; i < Jnumbs[0]; i++)
                lo[i] = 0.0;

        //synth/////////////////////////////
        FwtSynth(J);
        ////////////////////////////////////

        for (int i = 0; i < Length + 2*SR; i++)
                pTmpData[i] = lo[i];

        ////////get min max///////////////////
        double min, max;
        MinMax(&lo[(int)SR], Length, min, max);

        CloseFWT();



        //get HF scale J///////
        J = ceil(log2(SR / 23.0)) - 2;     //[30Hz - ...] hf denoising

        wcscpy(filter, FiltersDir);
        wcscat(filter, L"\\bior97.flt");
        if (InitFWT(filter, pTmpData, Length + 2*SR) == false)
                return false;


        //transform////////////////////////
        FwtTrans(J);
        ///////////////////////////////////

        Jnumbs = GetJnumbs(J, Length + 2 * SR);
        int hinum, lonum;
        HiLoNumbs(J, Length + 2*SR, hinum, lonum);
        lo = GetFwtSpectrum();
        hi = GetFwtSpectrum() + (int(Length + 2 * SR) - hinum);

        int window;   //3sec window
        for (int j = J; j > 0; j--) {
                window = 3.0 * SR / pow(2.0, (double)j);

                Denoise(hi, Jnumbs[J-j], window);
                hi += Jnumbs[J-j];
        }

        //synth/////////////////////////////
        FwtSynth(J);
        ////////////////////////////////////

        for (int i = 0; i < Length; i++)
                pEcgData[i] = lo[i+(int)SR];

        //renormalize
        nMinMax(pEcgData, Length, min, max);

        CloseFWT();
        return true;
}
