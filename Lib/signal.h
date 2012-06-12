

#ifndef Signal_h
#define Signal_h

#include <stdio.h>
#include <vector>
using namespace std;
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#include <fstream>

typedef struct _datahdr {
        char hdr[4];
        unsigned int size;
        float sr;
        unsigned char bits;
        unsigned char lead;
        unsigned short umv;
        unsigned short bline;
        unsigned char hh;
        unsigned char mm;
        unsigned char ss;
        char rsrv[19];
} DATAHDR, *PDATAHDR;

class Signal
{
public:
        Signal();
        //Signal(const Signal& signal);
        virtual ~Signal();

// Operators
        //const Signal& operator=(const Signal& signal);

// Operations
        double* ReadFile(const wchar_t* name);
        double* ReadFile(const char* name);
        bool IsFileValid(const wchar_t* name);
        bool SaveFile(const wchar_t* name, const double* buffer, PDATAHDR hdr);
        bool SaveFile(const char* name, const double* buffer, PDATAHDR hdr);
        bool ToTxt(const wchar_t* name, const double* buffer, int size);
        void CloseFile();
        void mSecToTime(int msec, int& h, int& m, int& s, int& ms) const;
        void MinMax(const double* buffer, int size, double& min, double& max) const;

        //normalization
        void nMinMax(double* buffer, int size, double a, double b) const;
        void nMean(double* buffer, int size) const;
        void nZscore(double* buffer, int size) const;
        void nSoftmax(double* buffer, int size) const;
        void nEnergy(double* buffer, int size, int L = 2) const;

        //math routines////////////////////////////////////////////////////////////
        double Std(const double* buffer, int size) const;
        double Mean(const double* buffer, int size) const;
        inline double log2(double x) const;

        void AutoCov(double* buffer, int size) const;
        void AutoCov1(double* buffer, int size) const;
        void AutoCor(double* buffer, int size) const;
        void AutoCor1(double* buffer, int size) const;
        
        //denoising////////////////////////////////////////////////////////////////
        double MINIMAX(const double* buffer, int size) const;
        double FIXTHRES(const double* buffer, int size) const;
        double SURE(const double* buffer, int size) const;
        void Denoise(double* buffer, int size, int window, int type = 0, bool soft = true) const;
        void HardTH(double* buffer, int size, double TH, double l = 0.0) const;
        void SoftTH(double* buffer, int size, double TH, double l = 0.0) const;

// Access        
        double* GetData(int index = 0);
        inline int GetLength() const;
        inline int GetBits() const;
        inline int GetUmV() const;
        inline int GetLead() const;
        inline int GetLeadsNum() const;
        inline double GetSR() const;
        inline int GetH() const;
        inline int GetM() const;
        inline int GetS() const;

// Inquiry

protected:
        double* pData, SR;
        int Lead, UmV, Bits, Length;
        int hh, mm, ss;                //start time of the record

        HANDLE fp, fpmap;
        LPVOID lpMap;

        FILE* in;
        float* lpf;
        short* lps;
        unsigned short* lpu;
        char* lpc;


private:
        Signal(const Signal& Signal);
        const Signal& operator=(const Signal& Signal);


        bool IsBinFile;                         //binary or text file
        wchar_t EcgFileName[_MAX_PATH];         //file name

        PDATAHDR pEcgHeader;                    //pointer to ECG data header
        vector<DATAHDR> EcgHeaders;             //arrays of headers
        vector<double *> EcgSignals;       //arrays of signals        

        bool ReadDatFile();                     //read custom data file
        bool ReadTxtFile();                     //read text file data
                //bool ReadCustomFile();
        bool ReadMitbihFile();                  //read mit-bih dat file
        int ParseMitbihHeader(FILE* fh);        //parse mit-bih header
        
        void ChangeExtension(wchar_t* path, const wchar_t* ext) const;
        int ReadLine(FILE* fh, char* buffer) const;


};

/*////////info////////////////////////////////
to OPEN file:
        Signal Ecg;
        double* pEcgData;

        pEcgData = Ecg.ReadFile("filename");              //error pEcgData = NULL
        Length = Ecg.GetLength();
        SR = Ecg.GetSR();
        UmV = Ecg.GetUmV();
        Bits = Ecg.GetBits();

to SAVE file:
        Ecg.SaveFile("EcgFileName", pEcgData, SR, Length, Bits, Umv)        
////////////////////////////////////////////*/


// Inlines
inline int Signal::GetLength() const
{
        return Length;
}

inline int Signal::GetBits() const
{
        return Bits;
}

inline int Signal::GetUmV() const
{
        return UmV;
}

inline int Signal::GetLead() const
{
        return Lead;
}

inline int Signal::GetLeadsNum() const
{
        return (int)EcgSignals.size();
}

inline double Signal::GetSR() const
{
        return SR;
}

inline int Signal::GetH() const
{
        return hh;
}

inline int Signal::GetM() const
{
        return mm;
}

inline int Signal::GetS() const
{
        return ss;
}

inline double Signal::log2(double x) const
{
        return log(x) / log(2.0);
}

#endif Signal_h

