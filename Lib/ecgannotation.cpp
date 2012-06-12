

//#include "stdafx.h"
#include "signal.h"
#include "cwt.h"
#include "fwt.h"
#include "ecgdenoise.h"
#include "ecgannotation.h"
#include <iostream>


EcgAnnotation::EcgAnnotation(PANNHDR p): qrsNum(0), annNum(0), auxNum(0),
                ANN(0), qrsANN(0), AUX(0)
{
        if (p) {
                memcpy(&ahdr, p, sizeof(ANNHDR));
        } else { //defaults
                ahdr.minbpm = 40;     //min bpm
                ahdr.maxbpm = 200;    //max bpm
                ahdr.minQRS = 0.04;   //min QRS duration
                ahdr.maxQRS = 0.2;    //max QRS duration
                ahdr.minUmV = 0.2;    //min UmV of R,S peaks
                ahdr.minPQ = 0.07;    //min PQ duration
                ahdr.maxPQ = 0.20;    //max PQ duration
                ahdr.minQT = 0.21;    //min QT duration
                ahdr.maxQT = 0.48;    //max QT duration
                ahdr.pFreq = 9.0;     //cwt Hz for P wave
                ahdr.tFreq = 3.0;     //cwt Hz for T wave
        }
}

EcgAnnotation::~EcgAnnotation()
{
        if (ANN) {
                for (int i = 0; i < annNum;  i++)
                        delete[] ANN[i];
                delete[] ANN;
        }
        if (qrsANN) {
                for (int i = 0; i < qrsNum;  i++)
                        delete[] qrsANN[i];
                delete[] qrsANN;
        }
        if (AUX) {
                for (int i = 0; i < auxNum;  i++)
                        delete[] AUX[i];
                delete[] AUX;
        }
}

//-----------------------------------------------------------------------------
// 10Hz cwt trans of signal
// spectrum in cwt class
// create qrsANN array   with qrsNum records  num of heart beats = qrsNum/2
//
int** EcgAnnotation::GetQRS(const double *data, int size, double sr, wchar_t *fltdir, int stype)
{

        double *pdata = (double *)malloc(size * sizeof(double));
        for (int i = 0; i < size; i++)
        {
                pdata[i] = data[i];
                //cout << pdata[i] << endl;
        }


        if (fltdir) {
                if (Filter30hz(pdata, size, sr, fltdir, stype) == false) { //pdata filed with filterd signal
                        delete[] pdata;
                        cout << "Filter30hz == false" << endl;
                        return 0;
                }
                else
                    cout << "Filter30hz == true" << endl;
//                for (int i = 0; i < size; i++)
//                {
//                    cout << pdata[i] << '\t';
//
//                }
                cout << '\n';

                // 2 continuous passes of CWT filtering
                /*if(Filter30hz(pdata,size,sr,fltdir,qNORM) == false)   //pdata filed with filterd signal
                        return 0;
                if(stype == qNOISE)
                if(Filter30hz(pdata,size,sr,fltdir,qNORM) == false)
                        return 0;*/
        }
        else
            cout << "Error: fltdir" << endl;


        double eCycle = (60.0 / (double)ahdr.maxbpm) - ahdr.maxQRS;  //secs
        cout << "eCycle =  " << eCycle << endl;
        if (int(eCycle*sr) <= 0) {
                eCycle = 0.1;
                ahdr.maxbpm = int(60.0 / (ahdr.maxQRS + eCycle));
                cout << "ahdr.maxbpm =  " << ahdr.maxbpm << endl;
        }
        cout << "ahdr.maxbpm =  " << ahdr.maxbpm << endl;
        //////////////////////////////////////////////////////////////////////////////

        int lqNum = 0;
        vector <int> QRS;    //clean QRS detected
        int add = 0;

        while (pdata[add]) add += int(0.1 * sr);                  //skip QRS in begining
        cout << "add1 = " << add << endl;
        while (pdata[add] == 0) add++;                            //get  1st QRS
        cout << "add2 = " << add << endl;

        QRS.push_back(add - 1);
        for (int i = 0; i < QRS.size(); i++)
            cout << "QRS = " << QRS[i] << endl;
        /////////////////////////////////////////////////////////////////////////////
        for (int m = add; m < size; m++) {                  //MAX 200bpm  [0,3 sec  min cario size]
                m += int(ahdr.maxQRS * sr);                     //smpl + 0,20sec    [0,20 max QRS length]


                if (m >= size) m = size - 1;
                add = 0;

                //noise checking///////////////////////////////////////
                if (m + int(eCycle*sr) >= size) { //near end of signal
                        QRS.pop_back();
                        break;
                }
                if (IsNoise(&pdata[m], eCycle*sr)) {  //smp(0.10sec)+0,20sec in noise
                        if (lqNum != (int)QRS.size() - 1)
                                MA.push_back(QRS[QRS.size()-1]);     //push MA noise location

                        QRS.pop_back();
                        lqNum = QRS.size();

                        //Find for next possible QRS start
                        while (IsNoise(&pdata[m], eCycle*sr)) {
                                m += int(eCycle * sr);
                                if (m >= size - int(eCycle*sr)) break;   //end of signal
                        }

                        if (m >= size - int(eCycle*sr)) break;
                        else {
                                while (pdata[m] == 0) {
                                        m++;
                                        if (m >= size) break;
                                }
                                if (m >= size) break;
                                else {
                                        QRS.push_back(m - 1);
                                        continue;
                                }
                        }
                }
                ////////////////////////////////////////////////////////


                while (pdata[m-add] == 0.0) add++;               //Find back for QRS end


                if ((m - add + 1) - QRS[QRS.size()-1] > ahdr.minQRS*sr)  //QRS size > 0.04 sec
                        QRS.push_back(m - add + 2); //QRS end
                else
                        QRS.pop_back();


                m += int(eCycle * sr);                                //smpl + [0,20+0,10]sec    200bpm MAX
                if (size - m < int(sr / 2)) break;



                while (pdata[m] == 0 && (size - m) >= int(sr / 2))   //Find nearest QRS
                        m++;

                if (size - m < int(sr / 2)) break;  //end of data

                QRS.push_back(m - 1);  //QRS begin
        }
        /////////////////////////////////////////////////////////////////////////////
        delete[] pdata;



        qrsNum = QRS.size() / 2;
        cout << "qrsNum =  " << qrsNum << endl;

        if (qrsNum > 0)                              //         46: ?
        {                                           //          1: N    -1: nodata in **AUX
                qrsANN = new int*[2*qrsNum];                    // [samps] [type] [?aux data]
                for (int i = 0; i < 2*qrsNum; i++)
                        qrsANN[i] = new int[3];

                for (int i = 0; i < 2*qrsNum; i++) {
                        qrsANN[i][0] = QRS[i];                                     //samp

                        if (i % 2 == 0)
                                qrsANN[i][1] = 1;                                        //type N
                        else {
                                qrsANN[i][1] = 40;                                      //type QRS)
                                //if( (qrsANN[i][0]-qrsANN[i-1][0]) >= int(sr*0.12) || (qrsANN[i][0]-qrsANN[i-1][0]) <= int(sr*0.03) )
                                // qrsANN[i-1][1] = 46;                                  //N or ? beat  (0.03?-0.12secs)
                        }

                        qrsANN[i][2] = -1;                                         //no aux data
                }

                return qrsANN;
        }
        else
        {
            cout << "Error: qrsNum <= 0" << endl;
            return 0;
        }
}

bool EcgAnnotation::Filter30hz(double *data, int size, double sr, wchar_t *fltdir, int stype) const
{
        CWT cwt;
        ///////////CWT 10Hz transform//////////////////////////////////////////
        cwt.InitCWT(size, CWT::GAUS1, 0, sr);        //gauss1 wavelet 6-index
        double *pspec = cwt.CwtTrans(data, 13);     //10-13 Hz transform?
        for (int i = 0; i < size; i++)
                data[i] = pspec[i];
        cwt.CloseCWT();

        //debug
        //ToTxt(L"10hz.txt",data,size);

        wchar_t flt[_MAX_PATH] = L"";
        wcscpy(flt, fltdir);

        switch (stype) {
        case qNORM:
                wcscat(flt, L"\\inter1.flt");
                break;

        case qNOISE:
                for (int i = 0; i < size; i++)  //ridges
                        data[i] *= (fabs(data[i]) / 2.0);
                wcscat(flt, L"\\bior13.flt");
                break;
        }

        FWT fwt;
        ////////////FWT 0-30Hz removal//////////////////////////////////////////
        if (fwt.InitFWT(flt, data, size) == false)
                return false;
        else
            cout << "InitFWT = true" << endl;

        int J = ceil(log2(sr / 23.0)) - 2;
        cout << "J = " << J << endl;
        //trans///////////////////////////////////////////////////
        fwt.FwtTrans(J);

        int *Jnumbs = fwt.GetJnumbs(J, size);
        for (int i = 0; i < J; i++)
            cout << "Jnumbs = " << Jnumbs[i] << endl;
        int hinum, lonum;
        fwt.HiLoNumbs(J, size, hinum, lonum);
        double *lo = fwt.GetFwtSpectrum();
        double *hi = fwt.GetFwtSpectrum() + (size - hinum);


        int window;   //2.0sec window
        for (int j = J; j > 0; j--) {
                window = (2.0 * sr) / pow(2.0, (double)j);    //2.0sec interval

                Denoise(hi, Jnumbs[J-j], window, 0, false);  //hard,MINIMAX denoise [30-...Hz]
                hi += Jnumbs[J-j];
        }
        for (int i = 0; i < lonum; i++)               //remove [0-30Hz]
                lo[i] = 0.0;

        //synth/////////////////////////////
        fwt.FwtSynth(J);

        for (int i = 0; i < fwt.GetLoBandSize(); i++)
                data[i] = lo[i];
        for (int i = size - (size - fwt.GetLoBandSize()); i < size; i++)
                data[i] = 0.0;

        fwt.CloseFWT();

        //debug
        //ToTxt(L"10hz(intr1).txt",data,size);
        return true;
}


////////////////////////////////////////////////////////////////////////////////
// Find ectopic beats in HRV data
void EcgAnnotation::GetEctopics(int **ann, int qrsnum, double sr) const
{
        if (qrsnum < 3)
                return;

        vector<double> RRs;
        for (int n = 0; n < qrsnum - 1; n++)
                RRs.push_back((double)(ann[n*2+2][0] - ann[n*2][0]) / sr); //qrsNum-1 rr's
        RRs.push_back(RRs[RRs.size()-1]);

        //  [RR1  RR2  RR3]   RR2 beat classification
        double rr1, rr2, rr3;
        for (int n = -2; n < (int)RRs.size() - 2; n++) {
                if (n == -2) {
                        rr1 = RRs[1];  //2
                        rr2 = RRs[0];  //1
                        rr3 = RRs[0];  //0
                } else if (n == -1) {
                        rr1 = RRs[1];
                        rr2 = RRs[0];
                        rr3 = RRs[1];
                } else {
                        rr1 = RRs[n];
                        rr2 = RRs[n+1];
                        rr3 = RRs[n+2];
                }

                if (60.0 / rr1 < ahdr.minbpm || 60.0 / rr1 > ahdr.maxbpm) //if RR's within 40-200bpm
                        continue;
                if (60.0 / rr2 < ahdr.minbpm || 60.0 / rr2 > ahdr.maxbpm) //if RR's within 40-200bpm
                        continue;
                if (60.0 / rr3 < ahdr.minbpm || 60.0 / rr3 > ahdr.maxbpm) //if RR's within 40-200bpm
                        continue;

                if (1.15*rr2 < rr1 && 1.15*rr2 < rr3) {
                        ann[n*2+4][1] = 46;
                        continue;
                }
                if (fabs(rr1 - rr2) < 0.3 && rr1 < 0.8 && rr2 < 0.8 && rr3 > 2.4*(rr1 + rr2)) {
                        ann[n*2+4][1] = 46;
                        continue;
                }
                if (fabs(rr1 - rr2) < 0.3 && rr1 < 0.8 && rr2 < 0.8 && rr3 > 2.4*(rr2 + rr3)) {
                        ann[n*2+4][1] = 46;
                        continue;
                }
        }
}


///////////////////////////////////////////////////////////////////////////////
//out united annotation with QRS PT////////////////////////////////////////////
// **ann [PQ,JP] pairs
int** EcgAnnotation::GetPTU(const double *data, int length, double sr, wchar_t *fltdir, int **ann, int qrsnum)
{
        int size, annPos;
        int T1 = -1, T = -1, T2 = -1, Twaves = 0;
        int P1 = -1, P = -1, P2 = -1, Pwaves = 0;

        CWT cwt;
        vector <int> Pwave;
        vector <int> Twave;                             //Twave [ ( , T , ) ]
        double *pspec;                             //      [ 0 , 0 , 0 ]  0 no Twave
        double min, max;                           //min,max for gaussian1 wave, center is zero crossing

        double rr;   //rr interval

        bool sign;
        bool twave, pwave;


        int add = 0;//int(sr*0.04);  //prevent imprecise QRS end detection

        int maNum = 0;
        for (int n = 0; n < qrsnum - 1; n++) {
                annPos = ann[n*2+1][0];                //i
                size = ann[n*2+2][0] - ann[n*2+1][0];  //i   size of  (QRS) <----> (QRS)

                bool maNs = false;
                for (int i = maNum; i < (int)MA.size(); i++) {
                        if (MA[i] > ann[n*2+1][0] && MA[i] < ann[n*2+2][0]) {
                                Pwave.push_back(0);
                                Pwave.push_back(0);
                                Pwave.push_back(0);
                                Twave.push_back(0);
                                Twave.push_back(0);
                                Twave.push_back(0);
                                maNum++;
                                maNs = true;
                                break;
                        }
                }
                if (maNs) continue;


                rr = (double)(ann[n*2+2][0] - ann[n*2][0]) / sr;
                if (60.0 / rr < ahdr.minbpm || 60.0 / rr > ahdr.maxbpm - 20) { //check if normal RR interval (40bpm - 190bpm)
                        Pwave.push_back(0);
                        Pwave.push_back(0);
                        Pwave.push_back(0);
                        Twave.push_back(0);
                        Twave.push_back(0);
                        Twave.push_back(0);
                        continue;
                }


                ///////////////search for TWAVE///////////////////////////////////////////////////////////

                if (sr*ahdr.maxQT - (ann[n*2+1][0] - ann[n*2+0][0]) > size - add)
                        size = size - add;
                else
                        size = sr * ahdr.maxQT - (ann[n*2+1][0] - ann[n*2+0][0]) - add;


                //double avg = Mean(data+annPos+add,size);         //avrg extension on boundaries
                //double lvl,rvl;
                //lvl = data[annPos+add];
                //rvl = data[annPos+add+size-1];
                cwt.InitCWT(size, CWT::GAUS1, 0, sr);                           //6-Gauss1 wlet
                pspec = cwt.CwtTrans(data + annPos + add, ahdr.tFreq);//,false,lvl,rvl);   //3Hz transform  pspec = size-2*add

                //cwt.ToTxt(L"debugS.txt",data+annPos+add,size);    //T wave
                //cwt.ToTxt(L"debugC.txt",pspec,size);               //T wave spectrum

                MinMax(pspec, size, min, max);
                for (int i = 0; i < size; i++) {
                        if (pspec[i] == min) T1 = i + annPos + add;
                        if (pspec[i] == max) T2 = i + annPos + add;
                }
                if (T1 > T2)swap(T1, T2);

                //additional constraints on [T1 T T2] duration, symmetry, QT interval
                twave = false;
                if ((pspec[T1-annPos-add] < 0 && pspec[T2-annPos-add] > 0) || (pspec[T1-annPos-add] > 0 && pspec[T2-annPos-add] < 0))
                        twave = true;
                if (twave) {
                        if ((double)(T2 - T1) >= 0.09*sr) { // && (double)(T2-T1)<=0.24*sr)   //check for T wave duration
                                twave = true;                  //QT interval = .4*sqrt(RR)
                                if ((double)(T2 - ann[n*2+0][0]) >= ahdr.minQT*sr && (double)(T2 - ann[n*2+0][0]) <= ahdr.maxQT*sr)
                                        twave = true;
                                else
                                        twave = false;
                        } else
                                twave = false;
                }

                if (twave) {
                        if (pspec[T1-annPos-add] > 0) sign = true;
                        else sign = false;

                        for (int i = T1 - annPos - add; i < T2 - annPos - add; i++) {
                                if (sign) {
                                        if (pspec[i] > 0) continue;
                                } else {
                                        if (pspec[i] < 0) continue;
                                }

                                T = i + annPos + add;
                                break;
                        }

                        //check for T wave symetry//////////////////////////
                        double ratio;
                        if (T2 - T < T - T1) ratio = (double)(T2 - T) / (double)(T - T1);
                        else ratio = (double)(T - T1) / (double)(T2 - T);
                        ////////////////////////////////////////////////////

                        if (ratio < 0.4) { //not a T wave
                                Twave.push_back(0);
                                Twave.push_back(0);
                                Twave.push_back(0);
                                twave = false;
                        } else {
                                //adjust center of T wave
                                //smooth it with gaussian, Find max ?
                                //cwt.ToTxt(L"debugS.txt",data+annPos+add,size);
                                int Tcntr = FindTmax(data + T1, T2 - T1);
                                if (Tcntr != -1) {
                                        Tcntr += T1;
                                        if (abs((Tcntr - T1) - ((T2 - T1) / 2)) < abs((T - T1) - ((T2 - T1) / 2)))  //which is close to center 0-cross or T max
                                                T = Tcntr;
                                }

                                Twaves++;
                                Twave.push_back(T1);
                                Twave.push_back(T);
                                Twave.push_back(T2);
                        }
                } else { //no T wave???    empty (QRS) <-------> (QRS)
                        Twave.push_back(0);
                        Twave.push_back(0);
                        Twave.push_back(0);
                }
                T = -1;
                ///////////////search for TWAVE///////////////////////////////////////////////////////////
                cwt.CloseCWT();





                ///////////////search for PWAVE///////////////////////////////////////////////////////////

                size = ann[n*2+2][0] - ann[n*2+1][0];  //n   size of  (QRS) <----> (QRS)

                if (sr*ahdr.maxPQ < size)
                        size = sr * ahdr.maxPQ;

                if (twave) {
                        if (T2 > ann[n*2+2][0] - size - int(0.04*sr))   // pwave wnd far from Twave at least on 0.02sec
                                size -= T2 - (ann[n*2+2][0] - size - int(0.04 * sr));
                }
                int size23 = (ann[n*2+2][0] - ann[n*2+1][0]) - size;

                //size -= 0.02*sr;   //impresize QRS begin detection
                if (size <= 0.03*sr) {
                        Pwave.push_back(0);
                        Pwave.push_back(0);
                        Pwave.push_back(0);
                        continue;
                }


                //avg = Mean(data+annPos+size23,size);                     //avrg extension on boundaries
                //lvl = data[annPos+size23];
                //rvl = data[annPos+size23+size-1];
                cwt.InitCWT(size, CWT::GAUS1, 0, sr);                                        //6-Gauss1 wlet
                pspec = cwt.CwtTrans(data + annPos + size23, ahdr.pFreq);//,false,lvl,rvl);    //9Hz transform  pspec = size-2/3size

                //cwt.ToTxt(L"debugS.txt",data+annPos+size23,size);
                //cwt.ToTxt(L"debugC.txt",pspec,size);

                MinMax(pspec, size, min, max);
                for (int i = 0; i < size; i++) {
                        if (pspec[i] == min) P1 = i + annPos + size23;
                        if (pspec[i] == max) P2 = i + annPos + size23;
                }
                if (P1 > P2) swap(P1, P2);

                //additional constraints on [P1 P P2] duration, symmetry, PQ interval
                pwave = false;
                if ((pspec[P1-annPos-size23] < 0 && pspec[P2-annPos-size23] > 0) || (pspec[P1-annPos-size23] > 0 && pspec[P2-annPos-size23] < 0))
                        pwave = true;
                if (pwave) {
                        if ((double)(P2 - P1) >= 0.03*sr && (double)(P2 - P1) <= 0.15*sr) { //check for P wave duration  9Hz0.03 5Hz0.05
                                pwave = true;                //PQ interval = [0.07 - 0.12,0.20]
                                if ((double)(ann[n*2+2][0] - P1) >= ahdr.minPQ*sr && (double)(ann[n*2+2][0] - P1) <= ahdr.maxPQ*sr)
                                        pwave = true;
                                else
                                        pwave = false;
                        } else
                                pwave = false;
                }

                if (pwave) {
                        if (pspec[P1-annPos-size23] > 0) sign = true;
                        else sign = false;

                        for (int i = P1 - annPos - size23; i < P2 - annPos - size23; i++) {
                                if (sign) {
                                        if (pspec[i] > 0) continue;
                                } else {
                                        if (pspec[i] < 0) continue;
                                }

                                P = i + annPos + size23;
                                break;
                        }

                        //check for T wave symetry//////////////////////////
                        double ratio;
                        if (P2 - P < P - P1) ratio = (double)(P2 - P) / (double)(P - P1);
                        else ratio = (double)(P - P1) / (double)(P2 - P);
                        ////////////////////////////////////////////////////

                        if (ratio < 0.4f) { //not a P wave
                                Pwave.push_back(0);
                                Pwave.push_back(0);
                                Pwave.push_back(0);
                        } else {
                                Pwaves++;
                                Pwave.push_back(P1);
                                Pwave.push_back(P);
                                Pwave.push_back(P2);
                        }
                } else {
                        Pwave.push_back(0);
                        Pwave.push_back(0);
                        Pwave.push_back(0);
                }
                P1 = -1;
                P = -1;
                P2 = -1;
                ///////////////search for PWAVE///////////////////////////////////////////////////////////
                cwt.CloseCWT();

        }






        /////////////////get q,r,s peaks//////////////////////////////////////////////////////////
        // on a denoised signal

        int peaksnum = 0;
        int Q, R, S;
        vector <int> qrsPeaks;          //q,r,s peaks [ q , r , s ]
                                        //            [ 0,  R , 0 ]  zero if not defined
        vector <char> qrsTypes;         //[q,r,s] or [_,R,s], etc...


        for (int n = 0; n < qrsnum; n++) { //fill with zeros
                for (int i = 0; i < 3; i++) {
                        qrsPeaks.push_back(0);
                        qrsTypes.push_back(' ');
                }
        }


        double *buff = (double *)malloc(length * sizeof(double));
        for (int i = 0; i < length; i++)
                buff[i] = data[i];

        EcgDenoise enoise;
        enoise.InitDenoise(fltdir, buff, length, sr);
        if (enoise.LFDenoise()) {
                //ToTxt(L"f.txt",buff,length);
                double *pbuff;
                for (int n = 0; n < qrsnum; n++) {
                        annPos = ann[n*2][0];   //PQ
                        size = ann[n*2+1][0] - ann[n*2][0] + 1; //PQ-Jpnt, including Jpnt

                        pbuff = &buff[annPos];

                        Q = -1;
                        FindRS(pbuff, size, R, S, ahdr.minUmV);
                        if (R != -1) R += annPos;
                        if (S != -1) S += annPos;

                        if (R != -1 && S != -1) {  // Rpeak > 0mV Speak < 0mV
                                if (S < R) { //check for S
                                        if (buff[R] > -buff[S]) {
                                                Q = S;
                                                S = -1;

                                                size = ann[n*2+1][0] - R + 1;  //including Jpnt
                                                pbuff = &buff[R];
                                                S = Finds(pbuff, size, 0.05);
                                                if (S != -1) S += R;
                                        }
                                } else {   //check for Q
                                        size = R - annPos + 1;  //including R peak
                                        Q = Findq(pbuff, size, 0.05);
                                        if (Q != -1) Q += annPos;
                                }
                        }
                        //else if only S
                        else if (S != -1) { //Find small r if only S detected  in rS large T lead
                                size = S - annPos + 1; //including S peak
                                pbuff = &buff[annPos];
                                R = Findr(pbuff, size, 0.05);
                                if (R != -1) R += annPos;
                        }
                        //else if only R
                        else if (R != -1) { //only R Find small q,s
                                size = R - annPos + 1; //including R peak
                                Q = Findq(pbuff, size, 0.05);
                                if (Q != -1) Q += annPos;
                                size = ann[n*2+1][0] - R + 1;  //including Jpnt
                                pbuff = &buff[R];
                                S = Finds(pbuff, size, 0.05);
                                if (S != -1) S += R;
                        }


                        //put peaks to qrsPeaks vector
                        if (R == -1 && S == -1) { //no peaks
                                ann[n*2][1] = 16;   //ARTEFACT
                                //remove P,T
                                if (n != 0) {
                                        if (Pwave[3*(n-1)]) {
                                                Pwaves--;
                                                Pwave[3*(n-1)] = 0;
                                                Pwave[3*(n-1)+1] = 0;
                                                Pwave[3*(n-1)+2] = 0;
                                        }
                                }
                                if (n != qrsnum - 1) {
                                        if (Twave[3*n]) {
                                                Twaves--;
                                                Twave[3*n] = 0;
                                                Twave[3*n+1] = 0;
                                                Twave[3*n+2] = 0;
                                        }
                                }
                        }
                        if (Q != -1) {
                                peaksnum++;
                                qrsPeaks[n*3] = Q;
                                if (fabs(buff[Q]) > 0.5)
                                        qrsTypes[n*3] = 17; //'Q';
                                else
                                        qrsTypes[n*3] = 15; //'q';
                        }
                        if (R != -1) {
                                peaksnum++;
                                qrsPeaks[n*3+1] = R;
                                if (fabs(buff[R]) > 0.5)
                                        qrsTypes[n*3+1] = 48; //'R';
                                else
                                        qrsTypes[n*3+1] = 47; //'r';
                        }
                        if (S != -1) {
                                peaksnum++;
                                qrsPeaks[n*3+2] = S;
                                if (fabs(buff[S]) > 0.5)
                                        qrsTypes[n*3+2] = 50; //'S';
                                else
                                        qrsTypes[n*3+2] = 49; //'s';
                        }
                }
        }

        free(buff);
        /////////////////get q,r,s peaks//////////////////////////////////////////////////////////






        ///////////////////////// complete annotation array///////////////////////////////////////
        maNum = 0;

        //Pwave vec size = Twave vec size
        annNum = Pwaves * 3 + qrsnum * 2 + peaksnum + Twaves * 3 + (int)MA.size();   //P1 P P2 [QRS] T1 T T2  noise annotation
        if (annNum > qrsnum)                        //42-(p 43-p) 24-Pwave
        {                                           //44-(t 45-t) 27-Twave
                ANN = new int*[annNum];                    // [samps] [type] [?aux data]
                for (int i = 0; i < annNum; i++)
                        ANN[i] = new int[3];

                int index = 0; //index to ANN
                int qindex = 0;  //index to qrsANN

                for (int i = 0; i < (int)Twave.size(); i += 3) {   //Twave=Pwaves=qrsPeaks size
                        //QRS complex
                        ANN[index][0] = ann[qindex][0];           //(QRS
                        ANN[index][1] = ann[qindex][1];           //
                        ANN[index++][2] = ann[qindex++][2];       //aux
                        if (qrsPeaks[i]) { //q
                                ANN[index][0] = qrsPeaks[i];
                                ANN[index][1] = qrsTypes[i];
                                ANN[index++][2] = -1;              //no aux
                        }
                        if (qrsPeaks[i+1]) { //r
                                ANN[index][0] = qrsPeaks[i+1];
                                ANN[index][1] = qrsTypes[i+1];
                                ANN[index++][2] = -1;              //no aux
                        }
                        if (qrsPeaks[i+2]) { //s
                                ANN[index][0] = qrsPeaks[i+2];
                                ANN[index][1] = qrsTypes[i+2];
                                ANN[index++][2] = -1;              //no aux
                        }
                        ANN[index][0] = ann[qindex][0];           //QRS)
                        ANN[index][1] = ann[qindex][1];           //
                        ANN[index++][2] = ann[qindex++][2];       //aux

                        //T wave
                        if (Twave[i]) {
                                ANN[index][0] = Twave[i];                //(t
                                ANN[index][1] = 44;
                                ANN[index++][2] = -1;                    //no aux
                                ANN[index][0] = Twave[i+1];              //T
                                ANN[index][1] = 27;
                                ANN[index++][2] = -1;                    //no aux
                                ANN[index][0] = Twave[i+2];              //t)
                                ANN[index][1] = 45;
                                ANN[index++][2] = -1;                    //no aux
                        }
                        //P wave
                        if (Pwave[i]) {
                                ANN[index][0] = Pwave[i];                //(t
                                ANN[index][1] = 42;
                                ANN[index++][2] = -1;                    //no aux
                                ANN[index][0] = Pwave[i+1];              //T
                                ANN[index][1] = 24;
                                ANN[index++][2] = -1;                    //no aux
                                ANN[index][0] = Pwave[i+2];              //t)
                                ANN[index][1] = 43;
                                ANN[index++][2] = -1;                    //no aux
                        }

                        if (!Twave[i] && !Pwave[i]) {          //check for MA noise
                                for (int m = maNum; m < (int)MA.size(); m++) {
                                        if (MA[m] > ann[qindex-1][0] && MA[m] < ann[qindex][0]) {
                                                ANN[index][0] = MA[m];      //Noise
                                                ANN[index][1] = 14;
                                                ANN[index++][2] = -1;       //no aux
                                                maNum++;
                                                break;
                                        }
                                }
                        }
                }

                //last QRS complex
                int ii = 3 * (qrsnum - 1);
                ANN[index][0] = ann[qindex][0];           //(QRS
                ANN[index][1] = ann[qindex][1];           //
                ANN[index++][2] = ann[qindex++][2];       //aux
                if (qrsPeaks[ii]) { //q
                        ANN[index][0] = qrsPeaks[ii];
                        ANN[index][1] = qrsTypes[ii];
                        ANN[index++][2] = -1;              //no aux
                }
                if (qrsPeaks[ii+1]) { //r
                        ANN[index][0] = qrsPeaks[ii+1];
                        ANN[index][1] = qrsTypes[ii+1];
                        ANN[index++][2] = -1;              //no aux
                }
                if (qrsPeaks[ii+2]) { //s
                        ANN[index][0] = qrsPeaks[ii+2];
                        ANN[index][1] = qrsTypes[ii+2];
                        ANN[index++][2] = -1;              //no aux
                }
                ANN[index][0] = ann[qindex][0];           //QRS)
                ANN[index][1] = ann[qindex][1];           //
                ANN[index++][2] = ann[qindex++][2];       //aux

                //check if noise after last qrs
                if (maNum < (int)MA.size()) {
                        if (MA[maNum] > ann[qindex-1][0]) {
                                ANN[index][0] = MA[maNum];      //Noise
                                ANN[index][1] = 14;
                                ANN[index++][2] = -1;
                        }
                }

                return ANN;
        } else
                return 0;
}
//-----------------------------------------------------------------------------

void EcgAnnotation::AddAnnotationOffset(int add)
{
        for (int i = 0; i < qrsNum; i++)
                qrsANN[i][0] += add;

        for (int i = 0; i < annNum; i++)
                ANN[i][0] += add;
}

// SaveAnnotation (**aux)   aux data
bool EcgAnnotation::SaveAnnotation(const wchar_t *name, int **ann, int nums)
{
        int samps;
        unsigned short anncode = 0;
        unsigned short type;
        DWORD bytes;
        char buff[1024];

        fp = CreateFileW(name, GENERIC_WRITE | GENERIC_READ, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (fp == INVALID_HANDLE_VALUE)
                return false;

        samps = ann[0][0];
        sprintf(buff, "%c%c%c%c%c%c", 0, 0xEC, HIWORD(LOBYTE(samps)), HIWORD(HIBYTE(samps)), LOWORD(LOBYTE(samps)), LOWORD(HIBYTE(samps)));
        WriteFile(fp, buff, 6, &bytes, 0);
        type = ann[0][1];
        anncode |= (type << 10);
        sprintf(buff, "%c%c", (char)LOBYTE(anncode), (char)HIBYTE(anncode));
        WriteFile(fp, buff, 2, &bytes, 0);


        for (int i = 1; i < nums; i++) {
                samps = ann[i][0] - ann[i-1][0];
                anncode = 0;
                type = ann[i][1];


                if (samps > 1023) {
                        sprintf(buff, "%c%c%c%c%c%c", 0, 0xEC, HIWORD(LOBYTE(samps)), HIWORD(HIBYTE(samps)), LOWORD(LOBYTE(samps)), LOWORD(HIBYTE(samps)));
                        WriteFile(fp, buff, 6, &bytes, 0);
                } else
                        anncode = samps;


                anncode |= (type << 10);

                sprintf(buff, "%c%c", (char)LOBYTE(anncode), (char)HIBYTE(anncode));
                WriteFile(fp, buff, 2, &bytes, 0);
        }
        sprintf(buff, "%c%c", 0, 0);
        WriteFile(fp, buff, 2, &bytes, 0);

        CloseHandle(fp);
        return true;
}

bool EcgAnnotation::SaveQTseq(const wchar_t *name, int **ann, int annsize, double sr, int length)
{
        vector <double> QT;
        int q = 0, t = 0;


        for (int i = 0; i < annsize; i++) {
                switch (ann[i][1]) {
                case 14:            //noise
                case 15:            //q
                case 16:            //artifact
                case 17:            //Q
                case 18:            //ST change
                case 19:            //T change
                case 20:            //systole
                case 21:            //diastole
                case 22:
                case 23:
                case 24:            //P
                case 25:
                case 26:
                case 27:
                case 28:
                case 29:
                case 30:
                case 31:
                case 32:
                case 33:
                case 36:
                case 37:
                case 39:
                case 40:
                case 42:  //(p
                case 43:  //p)
                case 44:  //(t
                case 47:  //r
                case 48:  //R
                case 49:  //s
                case 50:  //S
                        continue;
                }

                if (ann[i][1] == 45) { //45 - t)
                        t = ann[i][0];
                        if (q < t)
                                QT.push_back((double)(t - q) / sr);
                } else {
                        /*if(i+1<annsize && (ann[i+1][1]==47 || ann[i+1][1]==48))  //r only
                         q = ann[i+1][0];
                        else if(i+2<annsize && (ann[i+2][1]==47 || ann[i+2][1]==48))  //q,r
                         q = ann[i+2][0];
                        else*/
                        q = ann[i][0];
                }
        }


        if (QT.size()) {
                DATAHDR hdr;
                memset(&hdr, 0, sizeof(DATAHDR));

                memcpy(hdr.hdr, "DATA", 4);
                hdr.size = QT.size();
                hdr.sr = float((double)QT.size() / ((double)length / sr));
                hdr.bits = 32;
                hdr.umv = 1;

                SaveFile(name, &QT[0], &hdr);

                return true;
        } else
                return false;
}

bool EcgAnnotation::SavePQseq(const wchar_t *name, int **ann, int annsize, double sr, int length)
{
        vector <double> PQ;
        int p = length, q = 0;


        for (int i = 0; i < annsize; i++) {
                switch (ann[i][1]) {
                case 14:            //noise
                case 15:            //q
                case 16:            //artifact
                case 17:            //Q
                case 18:            //ST change
                case 19:            //T change
                case 20:            //systole
                case 21:            //diastole
                case 22:
                case 23:
                case 24:            //P
                case 25:
                case 26:
                case 27:
                case 28:
                case 29:
                case 30:
                case 31:
                case 32:
                case 33:
                case 36:
                case 37:
                case 39:
                case 40:
                case 43:  //p)
                case 44:  //(t
                case 45:  //t)
                case 47:  //r
                case 48:  //R
                case 49:  //s
                case 50:  //S
                        continue;
                }

                if (ann[i][1] == 42)   //42 - (p
                        p = ann[i][0];
                else {
                        q = ann[i][0];
                        if (p < q) {
                                PQ.push_back((double)(q - p) / sr);
                                p = length;
                        }
                }
        }

        if (PQ.size()) {
                DATAHDR hdr;
                memset(&hdr, 0, sizeof(DATAHDR));

                memcpy(hdr.hdr, "DATA", 4);
                hdr.size = PQ.size();
                hdr.sr = float((double)PQ.size() / ((double)length / sr));
                hdr.bits = 32;
                hdr.umv = 1;

                SaveFile(name, &PQ[0], &hdr);

                return true;
        } else
                return false;
}

bool EcgAnnotation::SavePPseq(const wchar_t *name, int **ann, int annsize, double sr, int length) 
{
        vector <double> PP;
        int p1, p2;

        for (int i = 0; i < annsize; i++) {
                if (ann[i][1] == 42)      //42 - (p
                        p1 = ann[i][0];
                else if (ann[i][1] == 43) { //43 - p)
                        p2 = ann[i][0];
                        PP.push_back((double)(p2 - p1) / sr);
                }
        }

        if (PP.size()) {
                DATAHDR hdr;
                memset(&hdr, 0, sizeof(DATAHDR));

                memcpy(hdr.hdr, "DATA", 4);
                hdr.size = PP.size();
                hdr.sr = float((double)PP.size() / ((double)length / sr));
                hdr.bits = 32;
                hdr.umv = 1;

                SaveFile(name, &PP[0], &hdr);

                return true;
        } else
                return false;
}

bool EcgAnnotation::GetRRseq(int **ann, int nums, double sr, vector<double> *RR, vector<int> *RRpos) const
{
        int add = -1;
        double rr, r1, r2;

        RR->clear();
        RRpos->clear();

        //R peak or S peak annotation
        bool rrs = true;
        int rNum = 0, sNum = 0;
        for (int i = 0; i < nums; i++) {
                if (ann[i][1] == 47 || ann[i][1] == 48) rNum++;
                else if (ann[i][1] == 49 || ann[i][1] == 50) sNum++;
        }
        if (int(1.2f*(float)rNum) < sNum)
                rrs = false;  //R peaks less than S ones


        for (int i = 0; i < nums; i++) {
                switch (ann[i][1]) {
                case 0:    //non beats
                case 15:   //q
                case 17:   //Q
                case 18:   //ST change
                case 19:   //T change
                case 20:   //systole
                case 21:   //diastole
                case 22:
                case 23:
                case 24:
                case 25:
                case 26:
                case 27:
                case 28:
                case 29:
                case 30:
                case 31:
                case 32:
                case 33:
                case 36:
                case 37:
                case 40:   //')' J point
                case 42:   //(p
                case 43:   //p)
                case 44:   //(t
                case 45:   //t)
                case 47:   //r
                case 48:   //R
                case 49:   //s
                case 50:   //S
                        continue;

                case 14:   //noise
                case 16:   //artifact
                        add = -1;
                        continue;
                }

                if (add != -1) {
                        //annotation on RRs peaks
                        if (rrs) {
                                if (i + 1 < nums && (ann[i+1][1] == 47 || ann[i+1][1] == 48))  //r only
                                        r2 = ann[i+1][0];
                                else if (i + 2 < nums && (ann[i+2][1] == 47 || ann[i+2][1] == 48))  //q,r
                                        r2 = ann[i+2][0];
                                else //(ann[i][1]==N,ECT,...)  //no detected R only S
                                        r2 = ann[i][0];

                                if (add + 1 < nums && (ann[add+1][1] == 47 || ann[add+1][1] == 48))
                                        r1 = ann[add+1][0];
                                else if (add + 2 < nums && (ann[add+2][1] == 47 || ann[add+2][1] == 48))
                                        r1 = ann[add+2][0];
                                else //(ann[add][1]==N,ECT,...) //no detected R only S
                                        r1 = ann[add][0];
                        }
                        //annotation on S peaks
                        else {
                                if (i + 1 < nums && (ann[i+1][1] == 40))  //N)
                                        r2 = ann[i][0];
                                else if (i + 1 < nums && (ann[i+1][1] == 49 || ann[i+1][1] == 50))  //Sr
                                        r2 = ann[i+1][0];
                                else if (i + 2 < nums && (ann[i+2][1] == 49 || ann[i+2][1] == 50))  //rS
                                        r2 = ann[i+2][0];
                                else if (i + 3 < nums && (ann[i+3][1] == 49 || ann[i+3][1] == 50))  //errQ rS
                                        r2 = ann[i+3][0];
                                else if (i + 1 < nums && (ann[i+1][1] == 47 || ann[i+1][1] == 48))  //no S
                                        r2 = ann[i+1][0];
                                else if (i + 2 < nums && (ann[i+2][1] == 47 || ann[i+2][1] == 48))  //no S
                                        r2 = ann[i+2][0];

                                if (add + 1 < nums && (ann[add+1][1] == 40))  //N)
                                        r1 = ann[add][0];
                                else if (add + 1 < nums && (ann[add+1][1] == 49 || ann[add+1][1] == 50))
                                        r1 = ann[add+1][0];
                                else if (add + 2 < nums && (ann[add+2][1] == 49 || ann[add+2][1] == 50))
                                        r1 = ann[add+2][0];
                                else if (add + 3 < nums && (ann[add+3][1] == 49 || ann[add+3][1] == 50))
                                        r1 = ann[add+3][0];
                                else if (add + 1 < nums && (ann[add+1][1] == 47 || ann[add+1][1] == 48))  //no S
                                        r1 = ann[add+1][0];
                                else if (add + 2 < nums && (ann[add+2][1] == 47 || ann[add+2][1] == 48))  //no S
                                        r1 = ann[add+2][0];
                        }

                        rr = 60.0 / ((r2 - r1) / sr);
                        if (rr >= ahdr.minbpm && rr <= ahdr.maxbpm) {
                                RR->push_back(rr);         //in bpm
                                RRpos->push_back(r1);
                        }
                }
                add = i;
        }

        if (RR->size())
                return true;
        else
                return false;
}

bool EcgAnnotation::SaveRRseq(wchar_t *name, int **ann, int nums, double sr, int length) 
{
        vector <double> RR;
        int add = -1;
        double rr, r1, r2;

        //R peak or S peak annotation////////////////////////////
        bool rrs = true;
        int rNum = 0, sNum = 0;
        for (int i = 0; i < nums; i++) {
                if (ann[i][1] == 47 || ann[i][1] == 48) rNum++;
                else if (ann[i][1] == 49 || ann[i][1] == 50) sNum++;
        }
        if (int(1.1f*(float)rNum) < sNum) {
                rrs = false;  //R peaks less than S ones
                wcscat(name, L"_SS.dat");
        } else
                wcscat(name, L"_RR.dat");

        ////////////////////////////////////////////////////////


        for (int i = 0; i < nums; i++) {
                switch (ann[i][1]) {
                case 0:    //non beats
                case 15:   //q
                case 17:   //Q
                case 18:   //ST change
                case 19:   //T change
                case 20:   //systole
                case 21:   //diastole
                case 22:
                case 23:
                case 24:
                case 25:
                case 26:
                case 27:
                case 28:
                case 29:
                case 30:
                case 31:
                case 32:
                case 33:
                case 36:
                case 37:
                case 40:   //')' J point
                case 42:   //(p
                case 43:   //p)
                case 44:   //(t
                case 45:   //t)
                case 47:   //r
                case 48:   //R
                case 49:   //s
                case 50:   //S
                        continue;

                case 14:   //noise
                case 16:   //artifact
                        add = -1;
                        continue;
                }

                if (add != -1) {
                        //annotation on RRs peaks
                        if (rrs) {
                                if (i + 1 < nums && (ann[i+1][1] == 47 || ann[i+1][1] == 48))  //r only
                                        r2 = ann[i+1][0];
                                else if (i + 2 < nums && (ann[i+2][1] == 47 || ann[i+2][1] == 48))  //q,r
                                        r2 = ann[i+2][0];
                                else //(ann[i][1]==N,ECT,...)  //no detected R only S
                                        r2 = ann[i][0];

                                if (add + 1 < nums && (ann[add+1][1] == 47 || ann[add+1][1] == 48))
                                        r1 = ann[add+1][0];
                                else if (add + 2 < nums && (ann[add+2][1] == 47 || ann[add+2][1] == 48))
                                        r1 = ann[add+2][0];
                                else //(ann[add][1]==N,ECT,...) //no detected R only S
                                        r1 = ann[add][0];
                        }
                        //annotation on S peaks
                        else {
                                if (i + 1 < nums && (ann[i+1][1] == 40))  //N)
                                        r2 = ann[i][0];
                                else if (i + 1 < nums && (ann[i+1][1] == 49 || ann[i+1][1] == 50))  //Sr
                                        r2 = ann[i+1][0];
                                else if (i + 2 < nums && (ann[i+2][1] == 49 || ann[i+2][1] == 50))  //rS
                                        r2 = ann[i+2][0];
                                else if (i + 3 < nums && (ann[i+3][1] == 49 || ann[i+3][1] == 50))  //errQ rS
                                        r2 = ann[i+3][0];
                                else if (i + 1 < nums && (ann[i+1][1] == 47 || ann[i+1][1] == 48))  //no S
                                        r2 = ann[i+1][0];
                                else if (i + 2 < nums && (ann[i+2][1] == 47 || ann[i+2][1] == 48))  //no S
                                        r2 = ann[i+2][0];

                                if (add + 1 < nums && (ann[add+1][1] == 40))  //N)
                                        r1 = ann[add][0];
                                else if (add + 1 < nums && (ann[add+1][1] == 49 || ann[add+1][1] == 50))
                                        r1 = ann[add+1][0];
                                else if (add + 2 < nums && (ann[add+2][1] == 49 || ann[add+2][1] == 50))
                                        r1 = ann[add+2][0];
                                else if (add + 3 < nums && (ann[add+3][1] == 49 || ann[add+3][1] == 50))
                                        r1 = ann[add+3][0];
                                else if (add + 1 < nums && (ann[add+1][1] == 47 || ann[add+1][1] == 48))  //no S
                                        r1 = ann[add+1][0];
                                else if (add + 2 < nums && (ann[add+2][1] == 47 || ann[add+2][1] == 48))  //no S
                                        r1 = ann[add+2][0];
                        }

                        rr = 60.0 / ((r2 - r1) / sr);
                        if (rr >= ahdr.minbpm && rr <= ahdr.maxbpm)
                                RR.push_back(rr);         //in bpm
                }
                add = i;
        }

        if (RR.size()) {
                DATAHDR hdr;
                memset(&hdr, 0, sizeof(DATAHDR));

                memcpy(hdr.hdr, "DATA", 4);
                hdr.size = RR.size();
                hdr.sr = float((double)RR.size() / ((double)length / sr));
                hdr.bits = 32;
                hdr.umv = 1;

                SaveFile(name, &RR[0], &hdr);
                return true;
        } else
                return false;
}

bool EcgAnnotation::SaveRRnseq(wchar_t *name, int **ann, int nums, double sr, int length)
{
        vector <double> RR;
        int add = -1;
        double rr, r1, r2;

        //R peak or S peak annotation////////////////////////////
        bool rrs = true;
        int rNum = 0, sNum = 0;
        for (int i = 0; i < nums; i++) {
                if (ann[i][1] == 47 || ann[i][1] == 48) rNum++;
                else if (ann[i][1] == 49 || ann[i][1] == 50) sNum++;
        }
        if (int(1.1f*(float)rNum) < sNum) {
                rrs = false;  //R peaks less than S ones
                wcscat(name, L"_SSn.dat");
        } else
                wcscat(name, L"_RRn.dat");
        ////////////////////////////////////////////////////////


        for (int i = 0; i < nums; i++) {
                switch (ann[i][1]) {
                case 1:             //N
                        if (add == -1) {
                                add = i;
                                continue;
                        }
                        break;

                case 0:     //ectopic beats
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:    //noise
                case 16:    //artefact
                case 34:
                case 35:
                case 38:
                case 46:
                        add = -1;   //reset counter
                        continue;

                default:     //other types
                        continue;
                }

                //annotation on RRs peaks
                if (rrs) {
                        if (i + 1 < nums && (ann[i+1][1] == 47 || ann[i+1][1] == 48))  //r only
                                r2 = ann[i+1][0];
                        else if (i + 2 < nums && (ann[i+2][1] == 47 || ann[i+2][1] == 48))  //q,r
                                r2 = ann[i+2][0];
                        else //(ann[i][1]==N,ECT,...)  //no detected R only S
                                r2 = ann[i][0];

                        if (add + 1 < nums && (ann[add+1][1] == 47 || ann[add+1][1] == 48))
                                r1 = ann[add+1][0];
                        else if (add + 2 < nums && (ann[add+2][1] == 47 || ann[add+2][1] == 48))
                                r1 = ann[add+2][0];
                        else //(ann[add][1]==N,ECT,...) //no detected R only S
                                r1 = ann[add][0];
                }
                //annotation on S peaks
                else {
                        if (i + 1 < nums && (ann[i+1][1] == 40))  //N)
                                r2 = ann[i][0];
                        else if (i + 1 < nums && (ann[i+1][1] == 49 || ann[i+1][1] == 50))  //Sr
                                r2 = ann[i+1][0];
                        else if (i + 2 < nums && (ann[i+2][1] == 49 || ann[i+2][1] == 50))  //rS
                                r2 = ann[i+2][0];
                        else if (i + 3 < nums && (ann[i+3][1] == 49 || ann[i+3][1] == 50))  //errQ rS
                                r2 = ann[i+3][0];
                        else if (i + 1 < nums && (ann[i+1][1] == 47 || ann[i+1][1] == 48))  //no S
                                r2 = ann[i+1][0];
                        else if (i + 2 < nums && (ann[i+2][1] == 47 || ann[i+2][1] == 48))  //no S
                                r2 = ann[i+2][0];

                        if (add + 1 < nums && (ann[add+1][1] == 40))  //N)
                                r1 = ann[add][0];
                        else if (add + 1 < nums && (ann[add+1][1] == 49 || ann[add+1][1] == 50))
                                r1 = ann[add+1][0];
                        else if (add + 2 < nums && (ann[add+2][1] == 49 || ann[add+2][1] == 50))
                                r1 = ann[add+2][0];
                        else if (add + 3 < nums && (ann[add+3][1] == 49 || ann[add+3][1] == 50))
                                r1 = ann[add+3][0];
                        else if (add + 1 < nums && (ann[add+1][1] == 47 || ann[add+1][1] == 48))  //no S
                                r1 = ann[add+1][0];
                        else if (add + 2 < nums && (ann[add+2][1] == 47 || ann[add+2][1] == 48))  //no S
                                r1 = ann[add+2][0];
                }

                rr = 60.0 / ((r2 - r1) / sr);
                if (rr >= ahdr.minbpm && rr <= ahdr.maxbpm)
                        RR.push_back(rr);         //in bpm

                add = i;
        }


        if (RR.size()) {
                DATAHDR hdr;
                memset(&hdr, 0, sizeof(DATAHDR));

                memcpy(hdr.hdr, "DATA", 4);
                hdr.size = RR.size();
                hdr.sr = float((double)RR.size() / ((double)length / sr));
                hdr.bits = 32;
                hdr.umv = 1;

                SaveFile(name, &RR[0], &hdr);
                return true;
        } else
                return false;
}

