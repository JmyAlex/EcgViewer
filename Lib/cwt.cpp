

//#include "stdafx.h"
#include "signal.h"
#include "cwt.h"


CWT::CWT(): ScaleType(LINEAR_SCALE), pCwtSpectrum(0), pReal(0), pImag(0)
{
}
CWT::~CWT()
{
        if (pReal) free(pReal);
        if (pImag) free(pImag);
        if (pCwtSpectrum) free(pCwtSpectrum);
}

void CWT::InitCWT(int size, enum WAVELET wavelet, double w, double sr)
{
        SignalSize = size;

        if (sr)
                SR = sr;

        w0 = w;
        pReal = (double *)malloc(sizeof(double) * (2 * SignalSize - 1));
        pImag = (double *)malloc(sizeof(double) * (2 * SignalSize - 1));
        pCwtSpectrum = (double *)malloc(sizeof(double) * (SignalSize));
        Wavelet = wavelet;        

        for (int i = 0; i < 2*SignalSize - 1; i++) {
                pReal[i] = 0;
                pImag[i] = 0;
        }
}

void  CWT::CloseCWT()
{
        if (pReal) {
                free(pReal);
                pReal = 0;
        }
        if (pImag) {
                free(pImag);
                pImag = 0;
        }
        if (pCwtSpectrum) {
                free(pCwtSpectrum);
                pCwtSpectrum = 0;
        }
}

float* CWT::CwtCreateFileHeader(wchar_t *name, PCWTHDR hdr, enum WAVELET wavelet, double w)
{
        wchar_t tmp[_MAX_PATH];

        int filesize;

        switch (wavelet) {
        case MHAT:
                wcscat(name, L"(mHat).w");
                break;
        case INV:
                wcscat(name, L"(Inv).w");
                break;
        case MORL:
                wcscat(name, L"(Morl).w");
                break;
        case MORLPOW:
                wcscat(name, L"(MPow).w");
                break;
        case MORLFULL:
                wcscat(name, L"(MComp");
                swprintf(tmp, L"%d", (int)w);
                wcscat(name, tmp);
                wcscat(name, L").w");
                break;

        case GAUS:
                wcscat(name, L"(Gaussian).w");
                break;
        case GAUS1:
                wcscat(name, L"(1Gauss).w");
                break;
        case GAUS2:
                wcscat(name, L"(2Gauss).w");
                break;
        case GAUS3:
                wcscat(name, L"(3Gauss).w");
                break;
        case GAUS4:
                wcscat(name, L"(4Gauss).w");
                break;
        case GAUS5:
                wcscat(name, L"(5Gauss).w");
                break;
        case GAUS6:
                wcscat(name, L"(6Gauss).w");
                break;
        case GAUS7:
                wcscat(name, L"(7Gauss).w");
                break;
        }

        if (hdr->type)
                filesize = hdr->size * (int)ceil((log(hdr->fmax) + hdr->fstep - log(hdr->fmin)) / hdr->fstep);
        else
                filesize = hdr->size * (int)ceil((hdr->fmax + hdr->fstep - hdr->fmin) / hdr->fstep);


        filesize = sizeof(float) * filesize + sizeof(CWTHDR);

        fp = CreateFileW(name, GENERIC_WRITE | GENERIC_READ, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (fp == INVALID_HANDLE_VALUE)
                return 0;
        fpmap = CreateFileMapping(fp, 0, PAGE_READWRITE, 0, filesize, 0);
        lpMap = MapViewOfFile(fpmap, FILE_MAP_WRITE, 0, 0, filesize);

        lpf = (float *)lpMap;

        memset(lpMap, 0, filesize);
        memcpy(lpMap, hdr, sizeof(CWTHDR));

        return (lpf + sizeof(CWTHDR) / sizeof(float));
}

float* CWT::CwtReadFile(const wchar_t *name)
{
        fp = CreateFileW(name, GENERIC_WRITE | GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (fp == INVALID_HANDLE_VALUE)
                return 0;
        fpmap = CreateFileMapping(fp, 0, PAGE_READWRITE, 0, 0, 0);
        lpMap = MapViewOfFile(fpmap, FILE_MAP_WRITE, 0, 0, 0);

        phdr = (PCWTHDR)lpMap;
        lpf = (float *)lpMap;


        if (memcmp(phdr->hdr, "WLET", 4))
                return 0;

        MinFreq = phdr->fmin;
        MaxFreq = phdr->fmax;
        FreqInterval = phdr->fstep;
        Length = phdr->size;
        SR = phdr->sr;
        if (phdr->type == LINEAR_SCALE)
                ScaleType = LINEAR_SCALE;
        else if(phdr->type == LOG_SCALE)
                ScaleType = LOG_SCALE;

        return (lpf + sizeof(CWTHDR) / sizeof(float));
}

double CWT::HzToScale(double f, double sr, enum WAVELET wavelet, double w) const
{
        double k;

        switch (wavelet) {
        case MHAT:
                k = 0.22222 * sr;
                break;
        case INV:
                k = 0.15833 * sr;
                break;
        case MORL:
        case MORLPOW:
                k = sr;
                break;
        case MORLFULL:
                k = sr * w * 0.1589;
                break;
        case GAUS:  
                k = 0.2 * sr;
                break;
        case GAUS1:
                k = 0.16 * sr;
                break;
        case GAUS2:
                k = 0.224 * sr;
                break;
        case GAUS3:
                k = 0.272 * sr;
                break;
        case GAUS4:
                k = 0.316 * sr;
                break;
        case GAUS5:
                k = 0.354 * sr;
                break;
        case GAUS6:
                k = 0.388 * sr;
                break;
        case GAUS7:
                k = 0.42 * sr;
                break;
        default:
                k = 0;
        }

        return (k / f);
}

void CWT::ConvertName(wchar_t *name, enum WAVELET wavelet, double w) const
{
        wchar_t tmp[_MAX_PATH];

        switch (wavelet) {
        case MHAT:
                wcscat(name, L"(mHat).w");
                break;
        case INV:
                wcscat(name, L"(Inv).w");
                break;
        case MORL:
                wcscat(name, L"(Morl).w");
                break;
        case MORLPOW:
                wcscat(name, L"(MPow).w");
                break;
        case MORLFULL:
                wcscat(name, L"(MComp");
                swprintf(tmp, L"%d", (int)w);
                wcscat(name, tmp);
                wcscat(name, L").w");
                break;

        case GAUS:
                wcscat(name, L"(Gaussian).w");
                break;
        case GAUS1:
                wcscat(name, L"(1Gauss).w");
                break;
        case GAUS2:
                wcscat(name, L"(2Gauss).w");
                break;
        case GAUS3:
                wcscat(name, L"(3Gauss).w");
                break;
        case GAUS4:
                wcscat(name, L"(4Gauss).w");
                break;
        case GAUS5:
                wcscat(name, L"(5Gauss).w");
                break;
        case GAUS6:
                wcscat(name, L"(6Gauss).w");
                break;
        case GAUS7:
                wcscat(name, L"(7Gauss).w");
                break;
        }
}


double* CWT::CwtTrans(const double *data, double freq, bool periodicboundary, double lval, double rval)
{
        IsPeriodicBoundary = periodicboundary;
        LeftVal = lval;
        RightVal = rval;

        IsPrecision = false;
        PrecisionSize = 0;                                      //0,0000001 float prsision
        double t, sn, cs, scale;

        scale = HzToScale(freq, SR, Wavelet, w0);

///////////wavelet calculation//////////////////////////////////////////////////////////////////
///////// center = SignalSize-1 in wavelet mass////////////////////////////////////////////////////

        for (int i = 0; i < SignalSize; i++) {                     //positive side
                t = ((double)i) / scale;

                if (Wavelet > INV && Wavelet < MORLFULL) {
                        sn = sin(6.28 * t);
                        cs = cos(6.28 * t);
                }
                if (Wavelet == MORLFULL) {
                        sn = sin(w0 * t);
                        cs = cos(w0 * t);
                }

                switch (Wavelet) {
                case MHAT:
                        pReal[(SignalSize-1)+i] = exp(-t * t / 2) * (-t * t + 1);
                        break;
                case INV:
                        pReal[(SignalSize-1)+i] = t * exp(-t * t / 2);
                        break;
                case MORL:
                        pReal[(SignalSize-1)+i] = exp(-t * t / 2) * (cs - sn);
                        break;
                case MORLPOW:
                        pReal[(SignalSize-1)+i] = exp(-t * t / 2) * cs;
                        pImag[(SignalSize-1)+i] = exp(-t * t / 2) * sn;
                        break;
                case MORLFULL:
                        pReal[(SignalSize-1)+i] = exp(-t * t / 2) * (cs - exp(-w0 * w0 / 2));
                        pImag[(SignalSize-1)+i] = exp(-t * t / 2) * (sn - exp(-w0 * w0 / 2));
                        break;

                case GAUS:
                        pReal[(SignalSize-1)+i] = exp(-t * t / 2);
                        break;
                case GAUS1:
                        pReal[(SignalSize-1)+i] = -t * exp(-t * t / 2);
                        break;
                case GAUS2:
                        pReal[(SignalSize-1)+i] = (t * t - 1) * exp(-t * t / 2);
                        break;
                case GAUS3:
                        pReal[(SignalSize-1)+i] = (2 * t + t - t * t * t) * exp(-t * t / 2);
                        break;
                case GAUS4:
                        pReal[(SignalSize-1)+i] = (3 - 6 * t * t + t * t * t * t) * exp(-t * t / 2);
                        break;
                case GAUS5:
                        pReal[(SignalSize-1)+i] = (-15 * t + 10 * t * t * t - t * t * t * t * t) * exp(-t * t / 2);
                        break;
                case GAUS6:
                        pReal[(SignalSize-1)+i] = (-15 + 45 * t * t - 15 * t * t * t * t + t * t * t * t * t * t) * exp(-t * t / 2);
                        break;
                case GAUS7:
                        pReal[(SignalSize-1)+i] = (105 * t - 105 * t * t * t + 21 * t * t * t * t * t - t * t * t * t * t * t * t) * exp(-t * t / 2);
                        break;
                }

                if (fabs(pReal[(SignalSize-1)+i]) < 0.0000001)
                        PrecisionSize++;

                if (PrecisionSize > 15) {
                        PrecisionSize = i;
                        IsPrecision = true;
                        break;
                }
        }
        if (IsPrecision == false)
                PrecisionSize = SignalSize;

        for (int i = -(PrecisionSize - 1); i < 0; i++) {               //negative side
                t = ((double)i) / scale;

                if (Wavelet > INV && Wavelet < MORLFULL) {
                        sn = sin(6.28 * t);
                        cs = cos(6.28 * t);
                }
                if (Wavelet == MORLFULL) {
                        sn = sin(w0 * t);
                        cs = cos(w0 * t);
                }

                switch (Wavelet) {
                case MHAT:
                        pReal[(SignalSize-1)+i] = exp(-t * t / 2) * (-t * t + 1);
                        break;
                case INV:
                        pReal[(SignalSize-1)+i] = t * exp(-t * t / 2);
                        break;
                case MORL:
                        pReal[(SignalSize-1)+i] = exp(-t * t / 2) * (cs - sn);
                        break;
                case MORLPOW:
                        pReal[(SignalSize-1)+i] = exp(-t * t / 2) * cs;
                        pImag[(SignalSize-1)+i] = exp(-t * t / 2) * sn;
                        break;
                case MORLFULL:
                        pReal[(SignalSize-1)+i] = exp(-t * t / 2) * (cs - exp(-w0 * w0 / 2));
                        pImag[(SignalSize-1)+i] = exp(-t * t / 2) * (sn - exp(-w0 * w0 / 2));
                        break;

                case GAUS:
                        pReal[(SignalSize-1)+i] = exp(-t * t / 2);    //gauss
                        break;
                case GAUS1:
                        pReal[(SignalSize-1)+i] = -t * exp(-t * t / 2);   //gauss1
                        break;
                case GAUS2:
                        pReal[(SignalSize-1)+i] = (t * t - 1) * exp(-t * t / 2);     //gauss2
                        break;
                case GAUS3:
                        pReal[(SignalSize-1)+i] = (2 * t + t - t * t * t) * exp(-t * t / 2);    //gauss3
                        break;
                case GAUS4:
                        pReal[(SignalSize-1)+i] = (3 - 6 * t * t + t * t * t * t) * exp(-t * t / 2);    //gauss4
                        break;
                case GAUS5:
                        pReal[(SignalSize-1)+i] = (-15 * t + 10 * t * t * t - t * t * t * t * t) * exp(-t * t / 2);    //gauss5
                        break;
                case GAUS6:
                        pReal[(SignalSize-1)+i] = (-15 + 45 * t * t - 15 * t * t * t * t + t * t * t * t * t * t) * exp(-t * t / 2);   //gauss6
                        break;
                case GAUS7:
                        pReal[(SignalSize-1)+i] = (105 * t - 105 * t * t * t + 21 * t * t * t * t * t - t * t * t * t * t * t * t) * exp(-t * t / 2);    //gauss7
                        break;
                }
        }
///////end wavelet calculations////////////////////////////////////////////

        pData = data;
        for (int x = 0; x < SignalSize; x++)
                pCwtSpectrum[x] = CwtTrans(x, scale);

        return pCwtSpectrum;
}
