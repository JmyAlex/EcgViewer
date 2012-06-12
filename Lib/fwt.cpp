

//#include "stdafx.h"
#include "signal.h"
#include "fwt.h"


FWT::FWT(): tH(0), tG(0), H(0), G(0),
                thL(0), tgL(0), hL(0), gL(0), thZ(0), tgZ(0), hZ(0), gZ(0),
                pFwtSpectrum(0), pTmpSpectrum(0),
                J(0), Jnumbs(0)
{
}

FWT::~FWT()
{
        if (tH) delete[] tH;
        if (tG) delete[] tG;
        if (H) delete[] H;
        if (G) delete[] G;

        if (pFwtSpectrum) free(pFwtSpectrum);
        if (pTmpSpectrum) free(pTmpSpectrum);

        if (Jnumbs) delete[] Jnumbs;
}

bool FWT::InitFWT(const wchar_t* fltname, const double* data, int size)
{
        filter = _wfopen(fltname, L"rt");
        if (filter) {
                tH = LoadFilter(thL, thZ);
                tG = LoadFilter(tgL, tgZ);
                H = LoadFilter(hL, hZ);
                G = LoadFilter(gL, gZ);
                fclose(filter);

                LoBandSize = size;
                SignalSize = size;
                pFwtSpectrum = (double *)malloc(sizeof(double) * size);
                pTmpSpectrum = (double *)malloc(sizeof(double) * size);
                pLoData = pTmpSpectrum;
                pHiData = pTmpSpectrum + size;

                for (int i = 0; i < size; i++)
                        pFwtSpectrum[i] = data[i];
                memset(pTmpSpectrum, 0, sizeof(double)*size);

                J = 0;

                return true;
        } else
                return false;
}

double* FWT::LoadFilter(int& L, int& Z) const
{
        fscanf(filter, "%d", &L);
        fscanf(filter, "%d", &Z);

        double *flt = new double[L];

        for (int i = 0; i < L; i++)
                fscanf(filter, "%lf", &flt[i]);

        return flt;
}

void FWT::CloseFWT()
{
        if (tH) {
                delete[] tH;
                tH = 0;
        }
        if (tG) {
                delete[] tG;
                tG = 0;
        }
        if (H) {
                delete[] H;
                H = 0;
        }
        if (G) {
                delete[] G;
                G = 0;
        }

        if (pFwtSpectrum) {
                free(pFwtSpectrum);
                pFwtSpectrum = 0;
        }
        if (pTmpSpectrum) {
                free(pTmpSpectrum);
                pTmpSpectrum = 0;
        }

        if (Jnumbs) {
                delete[] Jnumbs;
                Jnumbs = 0;
        }
}


//////////////////////transforms///////////////////////////////////////////////////////////////////
void FWT::HiLoTrans()
{
        int n;
        double s, d;

        for (int k = 0; k < LoBandSize / 2; k++) {
                s = 0;
                d = 0;

                for (int m = -thZ; m < thL - thZ; m++) {
                        n = 2 * k + m;
                        if (n < 0) n = 0 - n;
                        if (n >= LoBandSize) n -= (2 + n - LoBandSize);
                        s += tH[m+thZ] * pFwtSpectrum[n];
                }

                for (int m = -tgZ; m < tgL - tgZ; m++) {
                        n = 2 * k + m;
                        if (n < 0) n = 0 - n;
                        if (n >= LoBandSize) n -= (2 + n - LoBandSize);
                        d += tG[m+tgZ] * pFwtSpectrum[n];
                }

                pLoData[k] = s;
                pHiData[k] = d;
        }

        for (int i = 0; i < SignalSize; i++)
                pFwtSpectrum[i] = pTmpSpectrum[i];
}

void FWT::FwtTrans(int scales)
{
        for (int j = 0; j < scales; j++) {
                pHiData -= LoBandSize / 2;
                HiLoTrans();

                LoBandSize /= 2;
                J++;
        }
}

void FWT::HiLoSynth()
{
        int n;
        double s2k, s2k1;

        for (int i = 0; i < SignalSize; i++)
                pTmpSpectrum[i] = pFwtSpectrum[i];

        for (int k = 0; k < LoBandSize; k++) {
                s2k = 0;
                s2k1 = 0;

                for (int m = -hZ; m < hL - hZ; m++) {       //s2k and s2k1 for H[]
                        n = k - m;
                        if (n < 0) n = 0 - n;
                        if (n >= LoBandSize) n -= (2 + n - LoBandSize);

                        if (2*m >= -hZ && 2*m < hL - hZ)
                                s2k += H[(2*m)+hZ] * pLoData[n];
                        if ((2*m + 1) >= -hZ && (2*m + 1) < hL - hZ)
                                s2k1 += H[(2*m+1)+hZ] * pLoData[n];
                }

                for (int m = -gZ; m < gL - gZ; m++) {      //s2k and s2k1 for G[]
                        n = k - m;
                        if (n < 0) n = 0 - n;
                        if (n >= LoBandSize) n -= (2 + n - LoBandSize);

                        if (2*m >= -gZ && 2*m < gL - gZ)
                                s2k += G[(2*m)+gZ] * pHiData[n];
                        if ((2*m + 1) >= -gZ && (2*m + 1) < gL - gZ)
                                s2k1 += G[(2*m+1)+gZ] * pHiData[n];
                }

                pFwtSpectrum[2*k] = 2.0 * s2k;
                pFwtSpectrum[2*k+1] = 2.0 * s2k1;
        }
}

void FWT::FwtSynth(int scales)
{
        for (int j = 0; j < scales; j++) {
                HiLoSynth();
                pHiData += Jnumbs[j];

                LoBandSize *= 2;
                J--;
        }
}
////////////////////////////////////////////////////////////////////////////////////////////////


int* FWT::GetJnumbs(int j, int size)
{
        if (Jnumbs) delete[] Jnumbs;

        Jnumbs = new int[j];

        for (int i = 0; i < j; i++)
                Jnumbs[i] = size / (int)pow(2, (double)(j - i));

        return Jnumbs;
}

void FWT::HiLoNumbs(int j, int size, int &hinum, int &lonum) const
{
        lonum = 0;
        hinum = 0;

        for (int i = 0; i < j; i++) {
                size /= 2;
                hinum += size;
        }
        lonum = size;
}

bool FWT::FwtSaveFile(const wchar_t *name, const double *hipass, const double *lopass, PFWTHDR hdr)
{
        int filesize;
        short tmp;

        HiLoNumbs(hdr->J, hdr->size, HiNum, LoNum);

        switch (hdr->bits) {
        case 12:
                if ((HiNum + LoNum) % 2 != 0)
                        filesize = int((double)((HiNum + LoNum) + 1) * 1.5);
                else
                        filesize = int((double)(HiNum + LoNum) * 1.5);
                break;
        case 16:
                filesize = (HiNum + LoNum) * 2;
                break;
        case 0:
        case 32:
                filesize = (HiNum + LoNum) * 4;
                break;
        }

        fp = CreateFileW(name, GENERIC_WRITE | GENERIC_READ, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (fp == INVALID_HANDLE_VALUE) {
                fp = 0;
                return false;
        }
        fpmap = CreateFileMapping(fp, 0, PAGE_READWRITE, 0, filesize + sizeof(FWTHDR), 0);
        lpMap = MapViewOfFile(fpmap, FILE_MAP_WRITE, 0, 0, filesize + sizeof(FWTHDR));

        lpf = (float *)lpMap;
        lps = (short *)lpMap;
        lpc = (char *)lpMap;

        memset(lpMap, 0, filesize + sizeof(FWTHDR));
        memcpy(lpMap, hdr, sizeof(FWTHDR));

        lpf += sizeof(FWTHDR) / sizeof(float);
        lps += sizeof(FWTHDR) / sizeof(short);
        lpc += sizeof(FWTHDR);

        for (int i = 0; i < HiNum + LoNum; i++) {
                if (i < HiNum)
                        tmp = short(hipass[i] * (double)hdr->umv);
                else
                        tmp = short(lopass[i-HiNum] * (double)hdr->umv);

                switch (hdr->bits) {
                case 12:
                        if (i % 2 == 0) {
                                lpc[0] = LOBYTE(tmp);
                                lpc[1] = 0;
                                lpc[1] = HIBYTE(tmp) & 0x0f;
                        } else {
                                lpc[2] = LOBYTE(tmp);
                                lpc[1] |= HIBYTE(tmp) << 4;
                                lpc += 3;
                        }
                        break;

                case 16:                                               //16format
                        *lps++ = tmp;
                        break;

                case 0:
                case 32:
                        if (i < HiNum)                                        //32bit float
                                *lpf++ = (float)hipass[i];
                        else
                                *lpf++ = (float)lopass[i-HiNum];
                        break;
                }
        }

        CloseFile();
        return true;
}

bool FWT::FwtReadFile(const wchar_t *name, const char *appdir)
{
        short tmp;

        fp = CreateFileW(name, GENERIC_WRITE | GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (fp == INVALID_HANDLE_VALUE) {
                fp = 0;
                return false;
        }
        fpmap = CreateFileMapping(fp, 0, PAGE_READWRITE, 0, 0, 0);
        lpMap = MapViewOfFile(fpmap, FILE_MAP_WRITE, 0, 0, 0);

        phdr = (PFWTHDR)lpMap;
        lpf = (float *)lpMap;
        lps = (short *)lpMap;
        lpc = (char *)lpMap;

        Length = phdr->size;
        SR = phdr->sr;
        Bits = phdr->bits;
        Lead = phdr->lead;
        UmV = phdr->umv;
        strcpy(FilterName, phdr->wlet);
        J = phdr->J;


        lpf += sizeof(FWTHDR) / sizeof(float);
        lps += sizeof(FWTHDR) / sizeof(short);
        lpc += sizeof(FWTHDR);

        HiLoNumbs(J, Length, HiNum, LoNum);
        SignalSize = LoNum + HiNum;
        LoBandSize = LoNum;

        pFwtSpectrum = new double[SignalSize];
        pTmpSpectrum = new double[SignalSize];
        memset(pTmpSpectrum, 0, sizeof(double)*SignalSize);
        pLoData = pFwtSpectrum;
        pHiData = pFwtSpectrum + LoNum;

        for (int i = 0; i < SignalSize; i++) {
                switch (Bits) {
                case 12:                                             //212 format   12bit
                        if (i % 2 == 0) {
                                tmp = MAKEWORD(lpc[0], lpc[1] & 0x0f);
                                if (tmp > 0x7ff)
                                        tmp |= 0xf000;
                        } else {
                                tmp = MAKEWORD(lpc[2], (lpc[1] & 0xf0) >> 4);
                                if (tmp > 0x7ff)
                                        tmp |= 0xf000;
                                lpc += 3;
                        }

                        if (i < HiNum) 
                                pHiData[i] = (double)tmp / (double)UmV;
                        else 
                                pLoData[i-HiNum] = (double)tmp / (double)UmV;                        
                        break;

                case 16:                                             //16format
                        if (i < HiNum) 
                                pHiData[i] = (double)lps[i] / (double)UmV;
                        else 
                                pLoData[i-HiNum] = (double)lps[i] / (double)UmV;                        
                        break;

                case 0:
                case 32:                                             //32bit float
                        if (i < HiNum) 
                                pHiData[i] = (double)lpf[i];
                        else 
                                pLoData[i-HiNum] = (double)lpf[i];                        
                        break;
                }

        }

        pLoData = pTmpSpectrum;
        pHiData = pTmpSpectrum + LoNum;


        if (appdir) { //load filter for synthesis
                char flt[256];
                strcpy(flt, appdir);
                strcat(flt, "\\filters\\");
                strcat(flt, FilterName);
                strcat(flt, ".flt");

                filter = fopen(flt, "rt");
                if (filter) {
                        tH = LoadFilter(thL, thZ);
                        tG = LoadFilter(tgL, tgZ);
                        H = LoadFilter(hL, hZ);
                        G = LoadFilter(gL, gZ);
                        fclose(filter);
                } else {
                        CloseFile();
                        return false;
                }
        }


        CloseFile();
        return true;
}

