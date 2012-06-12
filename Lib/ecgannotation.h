

#ifndef Annotation_h
#define Annotation_h

#define qNORM 0
#define qNOISE 1

typedef struct _annrec {
        unsigned int pos;    //offset
        unsigned int type;   //beat type
        unsigned int aux;    //index to aux array
} ANNREC, *PANNREC;

typedef struct _annhdr {
        int minbpm;
        int maxbpm;
        double minUmV;  //min R,S amplitude
        double minQRS;  //min QRS duration
        double maxQRS;  //max QRS duration
        double minPQ;
        double maxPQ;
        double minQT;
        double maxQT;
        double pFreq;   //p wave freq for CWT
        double tFreq;   //t wave freq for CWT
} ANNHDR, *PANNHDR;

class Signal;

class EcgAnnotation : public Signal
{
public:
        EcgAnnotation(PANNHDR p = 0);
        //EcgAnnotation(const EcgAnnotation& annotation);
        ~EcgAnnotation();

// Operators
        //const EcgAnnotation& operator=(const EcgAnnotation& annotation);

// Operations
        int** GetQRS(const double *data, int size, double sr, wchar_t *fltdir = 0, int stype = qNORM);        //get RR's classification
        void GetEctopics(int **ann, int qrsnum, double sr) const;                                                   //classify ectopic beats
        int** GetPTU(const double *data, int length, double sr, wchar_t *fltdir, int **ann, int qrsnum);
        
        void AddAnnotationOffset(int add);    //add if annotated within fromX-toX
        bool SaveAnnotation(const wchar_t *name, int **ann, int nums);
        //bool  ReadANN(wchar_t *);

        bool GetRRseq(int **ann, int nums, double sr, vector<double> *RR, vector<int> *RRpos) const;
        bool SaveRRseq(wchar_t *name, int **ann, int nums, double sr, int length);
        bool SaveRRnseq(wchar_t *name, int **ann, int nums, double sr, int length);
        bool SaveQTseq(const wchar_t *name, int **ann, int annsize, double sr, int length);
        bool SavePQseq(const wchar_t *name, int **ann, int annsize, double sr, int length);
        bool SavePPseq(const wchar_t *name, int **ann, int annsize, double sr, int length);

// Access
        inline int GetQrsNumber() const;        
        inline int GetEcgAnnotationSize() const;        
        inline int** GetEcgAnnotation() const;        
        inline int** GetQrsAnnotation() const;        
        inline char** GetAuxData() const;
        inline PANNHDR GetAnnotationHeader();        

// Inquiry

protected:
private:
        EcgAnnotation(const EcgAnnotation& annotation);
        const EcgAnnotation& operator=(const EcgAnnotation& annotation);

        inline bool IsNoise(const double *data, int window) const;        //check for noise in window len
        bool Filter30hz(double *data, int size, double sr, wchar_t *fltdir, int stype) const;    //0-30Hz removal

        inline void FindRS(const double *data, int size, int &R, int &S, double err = 0.0) const;  //find RS or QR
        inline int Findr(const double *data, int size, double err = 0.0) const;  //find small r in PQ-S
        inline int Findq(const double *data, int size, double err = 0.0) const;  //find small q in PQ-R
        inline int Finds(const double *data, int size, double err = 0.0) const;  //find small s in R-Jpnt
        inline int FindTmax(const double *data, int size) const;


        ANNHDR ahdr;                    //annotation ECG params

        int **ANN, annNum;              //annotation after getPTU (RR's + PT)
        int **qrsANN, qrsNum;           //annotation after getQRS (RR's+MAnoise)  +ECT after getECT()
        vector <int> MA;                //MA noise
        int auxNum;
        char **AUX;                     //auxiliary ECG annotation data


};

/*                      0           1             2
        int **ANN [x][samples][annotation type][aux data index]

        wchar_t *dir = "c:\\dir_for_fwt_filters\\filters";
        double *sig;   //signal massive
        double SR;     //sampling rate of the signal
        int size;   //size of the signal

        EcgAnnotation ann;

        int **qrsAnn;  //qrs annotation massive
        qrsAnn = ann.GetQRS(sig,size,SR,dir);       //get QRS complexes
        //qrsAnn = ann->GetQRS(psig,size,SR,umess,qNOISE);    //get QRS complexes if signal is quite noisy

        int **ANN; //QRS + PT annotation
        if(qrsAnn) {     
                ann.GetEctopics(qrsAnn,ann->GetQrsNumber(),SR);     //label Ectopic beats
                ANN = ann.GetPTU(sig,size,SR,dir,qrsAnn,ann.GetQrsNumber());   //find P,T waves
        }
*/



//annotation codes types
/*
   skip=59
   num=60
   sub=61
   chn=62
   aux=63

char anncodes [51][10] =  {"notQRS", "N",       "LBBB",    "RBBB",     "ABERR", "PVC",
                           "FUSION", "NPC",     "APC",     "SVPB",     "VESC",  "NESC",
                           "PACE",   "UNKNOWN", "NOISE",   "q",        "ARFCT", "Q",
                           "STCH",   "TCH",     "SYSTOLE", "DIASTOLE", "NOTE",  "MEASURE",
                           "P",      "BBB",     "PACESP",  "T",        "RTM",   "U",
                           "LEARN",  "FLWAV",   "VFON",    "VFOFF",    "AESC",  "SVESC",
                           "LINK",   "NAPC",    "PFUSE",   "(",        ")",     "RONT",

    //user defined beats//
                           "(p",     "p)",      "(t",      "t)",       "ECT",
                           "r",      "R",       "s",       "S"};

               /*
                  [16] - ARFCT
                  [15] - q
                  [17] - Q
                  [24] - P
                  [27] - T
                  [39, 40] - '(' QRS ')'  PQ, J point
                  42 - (p Pwave onset
                  43 - p) Pwave offset
                  44 - (t Twave onset
                  45 - t) Twave offset
                  46 - ect Ectopic of any origin beat
                  47 - r
                  48 - R
                  49 - s
                  50 - S
                                               */

// Inlines
inline int EcgAnnotation::GetQrsNumber() const
{
        return qrsNum;
}

inline int EcgAnnotation::GetEcgAnnotationSize() const
{
        return annNum;
}

inline int** EcgAnnotation::GetEcgAnnotation() const
{
        return ANN;
}

inline int** EcgAnnotation::GetQrsAnnotation() const
{
        return qrsANN;
}

inline char** EcgAnnotation::GetAuxData() const
{
        return AUX;
}

inline PANNHDR EcgAnnotation::GetAnnotationHeader()
{
        return &ahdr;
}

inline bool EcgAnnotation::IsNoise(const double *data, int window) const
{
        for (int i = 0; i < window; i++)
                if (data[i]) return true;

        return false;
}

inline void EcgAnnotation::FindRS(const double *data, int size, int &R, int &S, double err) const  //find RS or QR
{
        double min, max;
        MinMax(data, size, min, max);

        R = -1;
        S = -1;
        if (!(max < 0.0 || max == data[0] || max == data[size-1] || max < err)) { //(fabs(max-data[0])<err && fabs(max-data[size-1])<err) ))
                for (int i = 1; i < size - 1; i++)
                        if (data[i] == max) {
                                R = i;
                                break;
                        }
        }
        if (!(min > 0.0 || min == data[0] || min == data[size-1] || -min < err)) { //(fabs(min-data[0])<err && fabs(min-data[size-1])<err) ))
                for (int i = 1; i < size - 1; i++)
                        if (data[i] == min) {
                                S = i;
                                break;
                        }
        }
}

int EcgAnnotation::FindTmax(const double *data, int size) const  //find T max/min peak position
{
        double min, max;
        MinMax(data, size, min, max);

        int tmin = -1, tmax = -1;
        for (int i = 0; i < size; i++) {
                if (data[i] == max) {
                        tmax = i;
                        break;
                }
        }
        for (int i = 0; i < size; i++) {
                if (data[i] == min) {
                        tmin = i;
                        break;
                }
        }

        //max closest to the center
        if (tmin == -1 || tmax == -1) //??no max min found
                return -1;
        else {
                if (abs(tmax - (size / 2)) < abs(tmin - (size / 2)))
                        return tmax;
                else
                        return tmin;
        }
}

inline int EcgAnnotation::Findr(const double *data, int size, double err) const //find small r in PQ-S
{
        double tmp, min, max;
        MinMax(data, size, min, max);

        if (max < 0.0 || max == data[0] || max == data[size-1] || fabs(max - data[0]) < err) return -1;
        else tmp = max;

        for (int i = 1; i < size - 1; i++) {
                if (data[i] == tmp)
                        return i;
        }
        return -1;
}
inline int EcgAnnotation::Findq(const double *data, int size, double err) const //find small q in PQ-R
{
        double tmp, min, max;
        MinMax(data, size, min, max);

        if (min > 0.0 || min == data[0] || min == data[size-1] || fabs(min - data[0]) < err) return -1;
        else tmp = min;

        for (int i = 1; i < size - 1; i++) {
                if (data[i] == tmp)
                        return i;
        }
        return -1;
}
inline int EcgAnnotation::Finds(const double *data, int size, double err) const  //find small s in R-Jpnt
{
        double tmp, min, max;
        MinMax(data, size, min, max);

        if (min > 0.0 || min == data[0] || min == data[size-1] || fabs(min - data[size-1]) < err) return -1;
        else tmp = min;

        for (int i = 1; i < size - 1; i++) {
                if (data[i] == tmp)
                        return i;
        }
        return -1;
}

#endif Annotation_h

