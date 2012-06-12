

#ifndef FWT_h
#define FWT_h

typedef struct _fwthdr {
        char hdr[4];
        unsigned int size;
        float sr;
        unsigned char bits;
        unsigned char lead;
        unsigned short umv;

        char wlet[8];
        unsigned short J;

        char rsrv[14];
} FWTHDR, *PFWTHDR;

class Signal;

class FWT : public Signal
{
public:
        FWT();
        //FWT(const FWT& fwt);
        ~FWT();

// Operators
        //const FWT& operator=(const FWT& fwt);

// Operations
        bool InitFWT(const wchar_t* fltname, const double* data, int size);
        void CloseFWT();

        void FwtTrans(int scales);                      //wavelet transform
        void FwtSynth(int scales);                      //wavelet synthesis

        bool FwtSaveFile(const wchar_t *name, const double *hipass, const double *lopass, PFWTHDR hdr);
        bool FwtReadFile(const wchar_t *name, const char *appdir = 0);

// Access
        inline double* GetFwtSpectrum() const;        
        inline int GetLoBandSize() const;        
        inline int GetJ() const;                
        int* GetJnumbs(int j, int size);
        void HiLoNumbs(int j, int size, int &hinum, int &lonum) const;

// Inquiry

protected:
private:
        FWT(const FWT& fwt);
        const FWT& operator=(const FWT& fwt);

        //filters inits
        FILE* filter;
        double* LoadFilter(int &L, int &Z) const;
        void HiLoTrans();
        void HiLoSynth();

        PFWTHDR phdr;
        char FilterName[_MAX_PATH];

        double *tH, *tG;     //analysis filters
        double *H, *G;       //synth filters
        int thL, tgL, hL, gL;     //filters lenghts
        int thZ, tgZ, hZ, gZ;     //filter centers

        int J;                //scales
        int *Jnumbs;          //hi values per scale
        int SignalSize;       //signal size
        int LoBandSize;       //divided signal size

        //spectra
        double *pFwtSpectrum;              //buffer with fwt spectra
        double *pTmpSpectrum;              //temporary
        double *pHiData, *pLoData;         //pointers to pTmpSpectrum
        int HiNum, LoNum;                       //number of lo and hi coeffs


};

// Inlines
inline double* FWT::GetFwtSpectrum() const
{
        return pFwtSpectrum;
}

inline int FWT::GetLoBandSize() const
{
        return LoBandSize;
}

int FWT::GetJ() const
{
        return J;
}

#endif FWT_h

