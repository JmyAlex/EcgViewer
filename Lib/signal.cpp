

//#include "stdafx.h"
#include "signal.h"


Signal::Signal(): pData(0), fp(0), fpmap(0), lpMap(0),
                  SR(0.0), Bits(0), UmV(0), Lead(0), Length(0),
                  hh(0), mm(0), ss(0)

{
        wcscpy(EcgFileName, L"");
}
Signal::~Signal()
{
        if (EcgSignals.size()) {
                for (int i = 0; i < (int)EcgSignals.size(); i++) {
                        pData = EcgSignals[i];
                        delete[] pData;
                }
        }
}

double* Signal::ReadFile(const char* name)
{
        wchar_t ustr[_MAX_PATH] = L"";
        for (int i = 0; i < (int)strlen(name); i++)
                mbtowc(ustr + i, name + i, MB_CUR_MAX);
        return ReadFile(ustr);
}

double* Signal::ReadFile(const wchar_t* name)
{
        wcscpy(EcgFileName, name);

        if (!IsFileValid(name)) 
                return 0;

        if (IsBinFile) {
                if (!ReadDatFile()) 
                        return 0;
        } else {
                //if (!ReadCustomFile()) { //read text file
                        if (!ReadMitbihFile()) //read mit-bih file
                                return 0;
                //}
        }

        return GetData();
}

bool Signal::IsFileValid(const wchar_t* name)
{
        fp = CreateFileW(name, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (fp == INVALID_HANDLE_VALUE)
                return false;
        fpmap = CreateFileMapping(fp, 0, PAGE_READONLY, 0, sizeof(DATAHDR), 0);
        lpMap = MapViewOfFile(fpmap, FILE_MAP_READ, 0, 0, sizeof(DATAHDR));

        pEcgHeader = (PDATAHDR)lpMap;

        if (lpMap != 0 && !memcmp(pEcgHeader->hdr, "DATA", 4)) {
                Length = pEcgHeader->size;
                SR = pEcgHeader->sr;
                Bits = pEcgHeader->bits;
                Lead = pEcgHeader->lead;
                UmV = pEcgHeader->umv;
                hh = pEcgHeader->hh;
                mm = pEcgHeader->mm;
                ss = pEcgHeader->ss;
                IsBinFile = true;
        } else
                IsBinFile = false;

        CloseFile();
        return true;
}

bool Signal::ReadDatFile()
{
        short tmp;

        fp = CreateFileW(EcgFileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (fp == INVALID_HANDLE_VALUE)
                return false;
        fpmap = CreateFileMapping(fp, 0, PAGE_READONLY, 0, 0, 0);
        lpMap = MapViewOfFile(fpmap, FILE_MAP_READ, 0, 0, 0);
        if (lpMap == 0) {
                CloseFile();
                return false;
        }

        pEcgHeader = (PDATAHDR)lpMap;
        EcgHeaders.push_back(*pEcgHeader);

        lpf = (float *)lpMap;
        lps = (short *)lpMap;
        lpu = (unsigned short *)lpMap;
        lpc = (char *)lpMap;

        Length = pEcgHeader->size;
        SR = pEcgHeader->sr;
        Bits = pEcgHeader->bits;
        Lead = pEcgHeader->lead;
        UmV = pEcgHeader->umv;

        lpf += sizeof(DATAHDR) / sizeof(float);
        lps += sizeof(DATAHDR) / sizeof(short);
        lpc += sizeof(DATAHDR);

        pData = new double[Length];
        EcgSignals.push_back(pData);

        for (int i = 0; i < Length; i++)
                switch (Bits) {
                case 12:                                             //212 format   12bit
                        if (i % 2 == 0) {
                                tmp = MAKEWORD(lpc[0], lpc[1] & 0x0f);
                                if (tmp > 0x7ff)
                                        tmp |= 0xf000;
                                pData[i] = (double)tmp / (double)UmV;
                        } else {
                                tmp = MAKEWORD(lpc[2], (lpc[1] & 0xf0) >> 4);
                                if (tmp > 0x7ff)
                                        tmp |= 0xf000;
                                pData[i] = (double)tmp / (double)UmV;
                                lpc += 3;
                        }
                        break;

                case 16:                                             //16format
                        pData[i] = (double)lps[i] / (double)UmV;
                        break;

                case 0:
                case 32:                                             //32bit float
                        pData[i] = (double)lpf[i];
                        break;
                }


        CloseFile();
        return true;
}

bool Signal::ReadTxtFile()
{
        vector<double> Buffer;
        double tmp;
        int res;

        if ((in = _wfopen(EcgFileName, L"rt")) == 0)
                return false;

        for (;;) {
                res = fscanf(in, "%lf", &tmp);
                if (res == EOF || res == 0)
                        break;
                else
                        Buffer.push_back(tmp);
        }

        fclose(in);
        Length = (int)Buffer.size();
        if (Length < 2)
                return false;

        pData = new double[Length];
        for (int i = 0; i < Length; i++)
                pData[i] = Buffer[i];
        EcgSignals.push_back(pData);

        DATAHDR hdr;
        memset(&hdr, 0, sizeof(DATAHDR));
        hdr.size = Length;
        hdr.umv = 1;
        hdr.bits = 32;
        EcgHeaders.push_back(hdr);

        return true;
}

//bool Signal::ReadCustomFile()
//{
//        vector<double> Buffer;
//        double tmp;
//
//		ifstream InFile;
//		InFile.open(EcgFileName, ios::binary);
//		InFile.seekg(0, ios::end);
//		Length = (int)InFile.tellg() / sizeof(double);
//		InFile.seekg(0, ios::beg);
//
//		for (int i = 0; i < Length; i++)
//			{
//				InFile.read(reinterpret_cast<char*>(&tmp),sizeof(double));
//				Buffer.push_back(tmp);
//			}
//
//		InFile.close();
//        if (Length < 2)
//                return false;
//
//        pData = new double[Length];
//        for (int i = 0; i < Length; i++)
//                pData[i] = Buffer[i];
//        EcgSignals.push_back(pData);
//
//        DATAHDR hdr;
//        memset(&hdr, 0, sizeof(DATAHDR));
//        hdr.size = Length;
//        hdr.umv = 1;
//        hdr.bits = 32;
//        EcgHeaders.push_back(hdr);
//
//        return true;
//}

bool Signal::ReadMitbihFile()
{
        wchar_t HeaFile[_MAX_PATH];
        wcscpy(HeaFile, EcgFileName);

        ChangeExtension(HeaFile, L".hea");
        FILE* fh = _wfopen(HeaFile, L"rt");
        if (!fh) 
                return false;

        if (ParseMitbihHeader(fh)) {
                fclose(fh);

                pEcgHeader = &EcgHeaders[0];
                int lNum = (int)EcgHeaders.size();
                int size = pEcgHeader->size;

                fp = CreateFileW(EcgFileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                if (fp == INVALID_HANDLE_VALUE || (GetFileSize(fp, 0) != (lNum * pEcgHeader->size * pEcgHeader->bits) / 8))
                        return false;
                fpmap = CreateFileMapping(fp, 0, PAGE_READONLY, 0, 0, 0);
                lpMap = MapViewOfFile(fpmap, FILE_MAP_READ, 0, 0, 0);
                if (lpMap == 0) {
                        CloseFile();
                        return false;
                }

                lps = (short *)lpMap;
                lpc = (char *)lpMap;

                for (int i = 0; i < lNum; i++) {
                        pData = new double[pEcgHeader->size];
                        EcgSignals.push_back(pData);
                }

                short tmp;
                for (int s = 0; s < size; s++) {
                        for (int n = 0; n < lNum; n++) {
                                pData = EcgSignals[n];
                                pEcgHeader = &EcgHeaders[n];

                                switch (pEcgHeader->bits) {
                                case 12:                                             //212 format   12bit
                                        if ((s*lNum + n) % 2 == 0) {
                                                tmp = MAKEWORD(lpc[0], lpc[1] & 0x0f);
                                                if (tmp > 0x7ff)
                                                        tmp |= 0xf000;
                                                tmp -= pEcgHeader->bline;
                                                pData[s] = (double)tmp / (double)pEcgHeader->umv;
                                        } else {
                                                tmp = MAKEWORD(lpc[2], (lpc[1] & 0xf0) >> 4);
                                                if (tmp > 0x7ff)
                                                        tmp |= 0xf000;
                                                tmp -= pEcgHeader->bline;
                                                pData[s] = (double)tmp / (double)pEcgHeader->umv;
                                                lpc += 3;
                                        }
                                        break;

                                case 16:                                      //16format
                                        pData[s] = (double)(*lps++ - pEcgHeader->bline) / (double)pEcgHeader->umv;
                                        break;
                                }
                        }
                }
                
                CloseFile();
                return true;
        } else {
                fclose(fh);
                return false;
        }
}

double* Signal::GetData(int index)
{
        if (!EcgSignals.size()) 
                return 0;
        if (index > (int)EcgSignals.size() - 1) 
                index = (int)EcgSignals.size() - 1;
        else if (index < 0) 
                index = 0;

        pEcgHeader = &EcgHeaders[index];
        pData = EcgSignals[index];
        SR = pEcgHeader->sr;
        Lead = pEcgHeader->lead;
        UmV = pEcgHeader->umv;
        Bits = pEcgHeader->bits;
        Length = pEcgHeader->size;
        hh = pEcgHeader->hh;
        mm = pEcgHeader->mm;
        ss = pEcgHeader->ss;

        return pData;
}

int Signal::ParseMitbihHeader(FILE* fh)
{
        char leads[18][6] =  {"I", "II", "III", "aVR", "aVL", "aVF", "v1", "v2",
                              "v3", "v4", "v5", "v6", "MLI", "MLII", "MLIII", "vX", "vY", "vZ"
                             };
        char str[10][256];

        if (ReadLine(fh, str[0]) <= 0)
                return false;

        int sNum, size;
        float sr;
        int res = sscanf(str[0], "%s %s %s %s %s", str[1], str[2], str[3], str[4], str[5]);
        if (res < 4)
                return 0;
        if (res == 5 && strlen(str[5]) == 8) {
                char* ptime = str[5];
                hh = atoi(ptime);
                mm = atoi(ptime + 3);
                ss = atoi(ptime + 6);
        }

        sNum = atoi(str[2]);
        sr = (float)atof(str[3]);
        size = atoi(str[4]);

        int eNum = 0;
        for (int i = 0; i < sNum; i++) {
                if (ReadLine(fh, str[0]) <= 0)
                        return 0;

                int umv, bits, bline;
                memset(str[9], 0, 256);

                res = sscanf(str[0], "%s %s %s %s %s %s %s %s %s", str[1], str[2], str[3], str[4], str[5], str[6], str[7], str[8], str[9]);
                if (res < 5) return 0;

                bits = atoi(str[2]);
                umv = atoi(str[3]);
                bline = atoi(str[5]);

                int offs = (int)strlen(str[1]), j = 0;
                for (j = 0; j < offs; j++) {
                        if (EcgFileName[(wcslen(EcgFileName)-offs)+j] != str[1][j])  //wctomb
                                break;
                }
                if (j == offs) {
                        eNum++;
                        DATAHDR hdr;
                        memset(&hdr, 0, sizeof(DATAHDR));
                        hdr.sr = sr;
                        hdr.bits = (bits == 212) ? 12 : bits;
                        hdr.umv = (umv == 0) ? 200 : umv;
                        hdr.bline = bline;
                        hdr.size = size;
                        hdr.hh = hh;
                        hdr.mm = mm;
                        hdr.ss = ss;
                        for (int l = 0; l < 18; l++) {
                                if (!stricmp(leads[l], str[9])) {
                                        hdr.lead = l + 1;
                                        break;
                                }
                        }
                        EcgHeaders.push_back(hdr);
                }
        }
        return eNum;
}

bool Signal::SaveFile(const char* name, const double* buffer, PDATAHDR hdr)
{
        wchar_t ustr[_MAX_PATH] = L"";
        for (int i = 0; i < (int)strlen(name); i++)
                mbtowc(ustr + i, name + i, MB_CUR_MAX);
        return SaveFile(ustr, buffer, hdr);
}

bool Signal::SaveFile(const wchar_t* name, const double* buffer, PDATAHDR hdr)
{
        int filesize;
        int tmp;

        switch (hdr->bits) {
        case 12:
                if (hdr->size % 2 != 0)
                        filesize = int((double)(hdr->size + 1) * 1.5);
                else
                        filesize = int((double)(hdr->size) * 1.5);
                break;
        case 16:
                filesize = hdr->size * 2;
                break;
        case 0:
        case 32:
                filesize = hdr->size * 4;
                break;
        }


        fp = CreateFileW(name, GENERIC_WRITE | GENERIC_READ, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (fp == INVALID_HANDLE_VALUE)
                return false;
        fpmap = CreateFileMapping(fp, 0, PAGE_READWRITE, 0, filesize + sizeof(DATAHDR), 0);
        lpMap = MapViewOfFile(fpmap, FILE_MAP_WRITE, 0, 0, filesize + sizeof(DATAHDR));
        if (lpMap == 0) {
                CloseFile();
                return false;
        }

        lpf = (float *)lpMap;
        lps = (short *)lpMap;
        lpu = (unsigned short *)lpMap;
        lpc = (char *)lpMap;

        memset(lpMap, 0, filesize + sizeof(DATAHDR));
        memcpy(lpMap, hdr, sizeof(DATAHDR));

        lpf += sizeof(DATAHDR) / sizeof(float);
        lps += sizeof(DATAHDR) / sizeof(short);
        lpc += sizeof(DATAHDR);


        for (unsigned int i = 0; i < hdr->size; i++) {
                switch (hdr->bits) {
                case 12:                                               //212 format   12bit
                        tmp = int(buffer[i] * (double)hdr->umv);
                        if (tmp > 2047) tmp = 2047;
                        if (tmp < -2048) tmp = -2048;

                        if (i % 2 == 0) {
                                lpc[0] = LOBYTE((short)tmp);
                                lpc[1] = 0;
                                lpc[1] = HIBYTE((short)tmp) & 0x0f;
                        } else {
                                lpc[2] = LOBYTE((short)tmp);
                                lpc[1] |= HIBYTE((short)tmp) << 4;
                                lpc += 3;
                        }
                        break;

                case 16:                                               //16format
                        tmp = int(buffer[i] * (double)hdr->umv);
                        if (tmp > 32767) tmp = 32767;
                        if (tmp < -32768) tmp = -32768;
                        *lps++ = (short)tmp;
                        break;

                case 0:
                case 32:                                               //32bit float
                        *lpf++ = (float)buffer[i];
                        break;
                }
        }

        CloseFile();
        return true;
}

bool Signal::ToTxt(const wchar_t* name, const double* buffer, int size)
{
        in = _wfopen(name, L"wt");

        if (in) {
                for (int i = 0; i < size; i++)
                        fwprintf(in, L"%lf\n", buffer[i]);

                fclose(in);
                return true;
        } else
                return false;
}

void Signal::ChangeExtension(wchar_t* path, const wchar_t* ext) const
{
        for (int i = (int)wcslen(path) - 1; i > 0; i--) {
                if (path[i] == '.') {
                        path[i] = 0;
                        wcscat(path, ext);
                        return;
                }
        }

        wcscat(path, ext);
}

int Signal::ReadLine(FILE* fh, char* buffer) const
{
        int res = 0;
        char* pbuffer = buffer;

        while ((short)res != EOF) {
                res = fgetc(fh);
                if (res == 0xD || res == 0xA) {
                        if (pbuffer == buffer) continue;

                        *pbuffer = 0;
                        return 1;
                }
                if ((short)res != EOF) {
                        *pbuffer++ = (char)res;
                }
        }

        return (short)res;
}

void Signal::mSecToTime(int msec, int& h, int& m, int& s, int& ms) const
{
        ms = msec % 1000;
        msec /= 1000;

        if (msec < 60) {
                h = 0;
                m = 0;
                s = msec;                 //sec to final
        } else {
                double tmp;
                tmp = (double)(msec % 60) / 60;
                tmp *= 60;
                s = int(tmp);
                msec /= 60;

                if (msec < 60) {
                        h = 0;
                        m = msec;
                } else {
                        h = msec / 60;
                        tmp = (double)(msec % 60) / 60;
                        tmp *= 60;
                        m = int(tmp);
                }
        }
}

void Signal::MinMax(const double* buffer, int size, double& min, double& max) const
{
        max = buffer[0];
        min = buffer[0];
        for (int i = 1; i < size; i++) {
                if (buffer[i] > max)max = buffer[i];
                if (buffer[i] < min)min = buffer[i];
        }
}

void Signal::CloseFile()
{
        if (lpMap != 0)
                UnmapViewOfFile(lpMap);
        if (fpmap != 0)
                CloseHandle(fpmap);
        if (fp != 0 && fp != INVALID_HANDLE_VALUE)
                CloseHandle(fp);
}

double Signal::Mean(const double* buffer, int size) const
{
        double mean = 0;

        for (int i = 0; i < size; i++)
                mean += buffer[i];

        return mean / (double)size;
}

double Signal::Std(const double* buffer, int size) const
{
        double mean, disp = 0;

        mean = Mean(buffer, size);

        for (int i = 0; i < size; i++)
                disp += (buffer[i] - mean) * (buffer[i] - mean);

        return (sqrt(disp / (double)(size - 1)));
}

void  Signal::nMinMax(double* buffer, int size, double a, double b) const
{
        double min, max;
        MinMax(buffer, size, min, max);

        for (int i = 0; i < size; i++) {
                if (max - min)
                        buffer[i] = (buffer[i] - min) * ((b - a) / (max - min)) + a;
                else
                        buffer[i] = a;
        }
}

void Signal::nMean(double* buffer, int size) const
{
        double mean = Mean(buffer, size);

        for (int i = 0; i < size; i++)
                buffer[i] = buffer[i] - mean;
}

void Signal::nZscore(double* buffer, int size) const
{
        double mean = Mean(buffer, size);
        double disp = Std(buffer, size);

        if (disp == 0.0) disp = 1.0;
        for (int i = 0; i < size; i++)
                buffer[i] = (buffer[i] - mean) / disp;
}

void Signal::nSoftmax(double* buffer, int size) const
{
        double mean = Mean(buffer, size);
        double disp = Std(buffer, size);

        if (disp == 0.0) disp = 1.0;
        for (int i = 0; i < size; i++)
                buffer[i] = 1.0 / (1 + exp(-((buffer[i] - mean) / disp)));
}

void Signal::nEnergy(double* buffer, int size, int L) const
{
        double enrg = 0.0;
        for (int i = 0; i < size; i++)
                enrg += pow(fabs(buffer[i]), (double)L);

        enrg = pow(enrg, 1.0 / (double)L);
        if (enrg == 0.0) enrg = 1.0;

        for (int i = 0; i < size; i++)
                buffer[i] /= enrg;
}

double Signal::MINIMAX(const double* buffer, int size) const
{
        return Std(buffer, size)*(0.3936 + 0.1829*log((double)size));
}

double Signal::FIXTHRES(const double* buffer, int size) const
{
        return Std(buffer, size)*sqrt(2.0*log((double)size));
}

double Signal::SURE(const double* buffer, int size) const
{
        return Std(buffer, size)*sqrt(2.0*log((double)size*log((double)size)));
}

void Signal::Denoise(double* buffer, int size, int window, int type, bool soft) const
{
        double TH;

        for (int i = 0; i < size / window; i++) {
                switch (type) {
                case 0:
                        TH = MINIMAX(buffer, window);
                        break;
                case 1:
                        TH = FIXTHRES(buffer, window);
                        break;
                case 2:
                        TH = SURE(buffer, window);
                        break;
                }
                if (soft)
                        SoftTH(buffer, window, TH);
                else
                        HardTH(buffer, window, TH);

                buffer += window;
        }

        if (size % window > 5) { //skip len=1
                switch (type) {
                case 0:
                        TH = MINIMAX(buffer, size % window);
                        break;
                case 1:
                        TH = FIXTHRES(buffer, size % window);
                        break;
                case 2:
                        TH = SURE(buffer, size % window);
                        break;
                }
                if (soft)
                        SoftTH(buffer, size % window, TH);
                else
                        HardTH(buffer, size % window, TH);
        }
}

void  Signal::HardTH(double* buffer, int size, double TH, double l) const
{
        for (int i = 0; i < size; i++)
                if (fabs(buffer[i]) <= TH)
                        buffer[i] *= l;
}

void  Signal::SoftTH(double* buffer, int size, double TH, double l) const
{
        for (int i = 0; i < size; i++) {
                if (fabs(buffer[i]) <= TH) {
                        buffer[i] *= l;
                } else {
                        if (buffer[i] > 0) {
                                buffer[i] -= TH * (1 - l);
                        } else {
                                buffer[i] += TH * (1 - l);
                        }
                }
        }
}


void Signal::AutoCov(double* buffer, int size) const
{
        double* rk, mu;
        int t;
        rk = new double[size];

        mu = Mean(buffer, size);

        for (int k = 0; k < size; k++) {
                rk[k] = 0;

                t = 0;
                while (t + k != size) {
                        rk[k] += (buffer[t] - mu) * (buffer[t+k] - mu);
                        t++;
                }

                rk[k] /= (double)t;                       // rk[k] /= t ?  autocovariance
        }

        for (int i = 0; i < size; i++)
                buffer[i] = rk[i];

        delete[] rk;        
}

void Signal::AutoCov1(double* buffer, int size) const
{
        double* rk, mu;        
        rk = new double[size];

        mu = Mean(buffer, size);

        for (int k = 0; k < size; k++) {
                rk[k] = 0;

                for (int t = 0; t < size; t++) {
                        if (t + k >= size)
                                rk[k] += (buffer[t] - mu) * (buffer[2*size-(t+k+2)] - mu);
                        else
                                rk[k] += (buffer[t] - mu) * (buffer[t+k] - mu);
                }

                rk[k] /= (double)size;
        }

        for (int i = 0; i < size; i++)
                buffer[i] = rk[i];

        delete[] rk;        
}

void Signal::AutoCor(double* buffer, int size) const
{
        double* rk, mu, std;
        int t;
        rk = new double[size];

        mu = Mean(buffer, size);
        std = Std(buffer, size);

        for (int k = 0; k < size; k++) {
                rk[k] = 0;

                t = 0;
                while (t + k != size) {
                        rk[k] += (buffer[t] - mu) * (buffer[t+k] - mu);
                        t++;
                }

                rk[k] /= (double)t * std * std;
        }

        for (int i = 0; i < size; i++)
                buffer[i] = rk[i];

        delete[] rk;        
}

void Signal::AutoCor1(double* buffer, int size) const
{
        double* rk, mu, std;
        rk = new double[size];

        mu = Mean(buffer, size);
        std = Std(buffer, size);

        for (int k = 0; k < size; k++) {
                rk[k] = 0;

                for (int t = 0; t < size; t++) {
                        if (t + k >= size)
                                rk[k] += (buffer[t] - mu) * (buffer[2*size-(t+k+2)] - mu);
                        else
                                rk[k] += (buffer[t] - mu) * (buffer[t+k] - mu);
                }

                rk[k] /= (double)size * std * std;
        }

        for (int i = 0; i < size; i++)
                buffer[i] = rk[i];

        delete[] rk;        
}

