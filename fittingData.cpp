//include 
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TF1.h>
#include <iostream>
using namespace std;

//LIMIT
    #define LIML 5.05e3
    #define LIMH 6e3

//choose Fitting method
    #define BACKG_EXP

    #define SIGNAL_DOUBLE_GAUS
    //#define SIGNAL_CRUIJFF
    //#define SIGNAL_LORENTZ
    
    #define SHOULDER_GAUS
    //#define SHOULDER_ARGUS
    //#define SHOULDER_LORENTZ

    //because double Gauss and Cruijff are the same but Alpha
        #ifdef SIGNAL_DOUBLE_GAUS
        #define SIGNAL_CRUIJFF
        #endif
//Quantity of Parameters
    #define NUM_PAR_BAC 2

    #ifdef SIGNAL_LORENTZ
        #define NUM_PAR_SIG 3
    #endif
    #ifdef SIGNAL_CRUIJFF
        #define NUM_PAR_SIG 6
    #endif

    #ifdef SHOULDER_GAUS
        #define NUM_PAR_SHO 3
    #endif
    #ifdef SHOULDER_LORENTZ
        #define NUM_PAR_SHO 3
    #endif
    #ifdef SHOULDER_ARGUS
        #define NUM_PAR_SHO 0 //TODO
    #endif
    #define NUM_PAR NUM_PAR_BAC+NUM_PAR_SIG+NUM_PAR_SHO


//Define fitting function
    //BACKGROUND_FUNCTION
        Double_t BackGround(Double_t *x, Double_t *par){   
            return TMath::Exp(par[0] * x[0] + par[1]);
        }
    //SHOULDER FUNCTION
        Double_t Shoulder(Double_t *x, Double_t *par){
            #ifdef SHOULDER_GAUS
            return par[0] * exp(-0.5 * pow((x[0] - par[1]) / par[2], 2));
            #endif

            #ifdef SHOULDER_LORENTZ
            return par[0]* par[1]*par[1] / TMath::Max(1.e-10, (x[0]-par[2])*(x[0]-par[2])+ par[1]*par[1]);
            #endif 

            #ifdef SHOULDER_ARGUS
            return 0 //TODO
            #endif
        }
    //Signal function
        Double_t Signal(Double_t *x, Double_t *par){

            #ifdef SIGNAL_LORENTZ
            return par[0]* par[1]*par[1] / TMath::Max(1.e-10, (x[0]-par[2])*(x[0]-par[2])+ par[1]*par[1]);
            #endif 

            #ifdef SIGNAL_CRUIJFF
                if ( x[0]<= par[0])
                    return par[5]*TMath::Exp(-(pow(x[0]-par[0],2)/(2*(pow(par[1],2)+par[2]*(pow(x[0]-par[0],2))))));
                else
                    return par[5]*TMath::Exp(-(pow(x[0]-par[0],2)/(2*(pow(par[3],2)+par[4]*(pow(x[0]-par[0],2))))));
            #endif
        }
    //Combine Signal and High Mass Background Function
        Double_t Combine(Double_t *x, Double_t *par){
            return Signal(x, par) + Shoulder(x,&par[NUM_PAR_SIG]) + BackGround(x, &par[NUM_PAR_SIG+NUM_PAR_SHO]);
        }
//Error Propagation functions
    Double_t ErrAPlusB(Double_t ErrA, Double_t ErrB)
    {
        Double_t Err2 = pow(ErrA,2)+pow(ErrB,2); 
        return sqrt(Err2);
    }
    Double_t ErrAMinuB(Double_t ErrA, Double_t ErrB)
    {
        return ErrAPlusB(ErrA,ErrB);
    }
    Double_t ErrAMultB(Double_t C , Double_t A, Double_t B, Double_t ErrA, Double_t ErrB)
    {
        Double_t f2 = pow(C,2);
        Double_t AErrA2 = pow(ErrA/A,2);
        Double_t BErrB2 = pow(ErrB/B,2);
        Double_t Err2 = f2*(AErrA2+BErrB2);
        return sqrt(Err2);
    }

//main function
void fittingData(){
//Declare fitting method
    #ifdef BACKG_EXP
        cout<<"BackGround fitting method :\tDouble Gaussian"<<endl;
    #endif
    #ifdef SIGNAL_CRUIJFF
        #ifdef SIGNAL_DOUBLE_GAUS
            cout<<"Signal fitting method :\t\tDouble Gaussian"<<endl;
        #else
            cout<<"Signal fitting method :\t\tCruijff"<<endl;
        #endif
    #endif
    #ifdef SIGNAL_LORENTZ
        cout<<"Signal fitting method :\tLorentz Peak"<<endl;
    #endif
    #ifdef SHOULDER_GAUS
        cout<<"Shoulder fitting method :\tGaussian"<<endl;
    #endif
    #ifdef SHOULDER_ARGUS
        cout<<"Shoulder fitting method :\tARGUS"<<endl;
    #endif
    #ifdef SHOULDER_LORENTZ
        cout<<"Shoulder fitting method :\tLorentz"<<endl;
    #endif

//Loading files
    TFile *f1 = new TFile("Output/DataAll.root");
    TFile *f2 = new TFile("Output/DataMagnetDown.root");
    TFile *f3 = new TFile("Output/DataMagnetUp.root");
    TFile *f4 = new TFile("Output/PhaseSpace.root");
//Define needed Data from file
    TH1F *B0Pos = (TH1F*) f1->Get("h_B_M0_Pos");
    TH1F *B0Neg = (TH1F*) f1->Get("h_B_M0_Neg");
//Set up fitting object
    //create Pos fitting obj
        TF1 *ComPos =      new TF1("ComPos",   Combine,    LIML, LIMH, NUM_PAR);
    //Configure fitting parametres
        //Background
            #ifdef BACKG_EXP
                ComPos->SetParName(NUM_PAR_SIG+NUM_PAR_SHO, "BackGr A");
                ComPos->SetParameter(NUM_PAR_SIG+NUM_PAR_SHO, -1.26e-3);
                ComPos->SetParLimits(NUM_PAR_SIG+NUM_PAR_SHO, -1,0);

                ComPos->SetParName(NUM_PAR_SIG+NUM_PAR_SHO+1, "BackGr B");
                ComPos->SetParameter(NUM_PAR_SIG+NUM_PAR_SHO+1, 10);
                ComPos->SetParLimits(NUM_PAR_SIG+NUM_PAR_SHO+1, 0,20);
            #endif
        //Signal
            #ifdef SIGNAL_LORENTZ 
                ComPos->SetParName(0,"Signal I");
                ComPos->SetParameter(0, 90);
                ComPos->SetParLimits(0,0,100);

                ComPos->SetParName(1,"Signal Gam");
                ComPos->SetParameter(1, 50);
                ComPos->SetParLimits(1,0,200);

                ComPos->SetParName(2,"Signal x0");
                ComPos->SetParameter(2, 5.28e3);
                ComPos->SetParLimits(2,5200,5400);
            #endif
            #ifdef SIGNAL_CRUIJFF 
                ComPos->SetParName(0, "Signal x0");
                ComPos->SetParameter(0, 5200);
                ComPos->SetParLimits(0,5200,5400);

                ComPos->SetParName(1, "Signal SigL");
                ComPos->SetParameter(1, 50);
                ComPos->SetParLimits(1,0,100);

                ComPos->SetParName(2, "Signal AlpL");
                ComPos->SetParameter(2, 0);
                ComPos->SetParLimits(2,0,1);
                
                ComPos->SetParName(3, "Signal SigR");
                ComPos->SetParameter(3, 50);
                ComPos->SetParLimits(3,0,100);
                
                ComPos->SetParName(4, "Signal AlpR");
                ComPos->SetParameter(4, 0);
                ComPos->SetParLimits(4,0,1);
                
                ComPos->SetParName(5, "Signal I");
                ComPos->SetParameter(5, 60);
                ComPos->SetParLimits(5,30,150);

                #ifdef SIGNAL_DOUBLE_GAUS
                ComPos->FixParameter(2,0);
                ComPos->FixParameter(4,0);
                #endif
            #endif
        //Shoulder
            #ifdef SHOULDER_LORENTZ
                ComPos->SetParName(NUM_PAR_SIG,"SHOULD I");
                ComPos->SetParameter(NUM_PAR_SIG, 20);
                ComPos->SetParLimits(NUM_PAR_SIG,0,50);

                ComPos->SetParName(NUM_PAR_SIG+1,"SHOULD Gam");
                ComPos->SetParameter(NUM_PAR_SIG+1, 100);
                ComPos->SetParLimits(NUM_PAR_SIG+1,5,100);

                ComPos->SetParName(NUM_PAR_SIG+2,"SHOULD x0");
                ComPos->SetParameter(NUM_PAR_SIG+2, 5100);
                ComPos->SetParLimits(NUM_PAR_SIG+2,4800,5100);//!THIS PARA SHOULD HAVE PHYSICS EXPLAIN
            #endif
            #ifdef SHOULDER_ARGUS
                //TODO
            #endif
            #ifdef SHOULDER_GAUS
                ComPos->SetParName(NUM_PAR_SIG,"SHOULD I");
                ComPos->SetParameter(NUM_PAR_SIG, 30);
                ComPos->SetParLimits(NUM_PAR_SIG,0,100);

                ComPos->SetParName(NUM_PAR_SIG+1,"SHOULD x0");
                ComPos->SetParameter(NUM_PAR_SIG+1, 5050);
                ComPos->SetParLimits(NUM_PAR_SIG+1,5000,5100);

                ComPos->SetParName(NUM_PAR_SIG+2,"SHOULD Sig");
                ComPos->SetParameter(NUM_PAR_SIG+2, 30);
                ComPos->SetParLimits(NUM_PAR_SIG+2,0,100);
            #endif
    //Create Neg fitting obj
        TF1 *ComNeg = ComPos;
        ComNeg->SetName("ComNeg");
//fitting Postive 
    TCanvas *c8 = new TCanvas("c8","",600,400);
    B0Pos->SetAxisRange(0,120,"Y");
    TFitResultPtr posPtr = B0Pos->Fit(ComPos,"RS");
    B0Pos->SetAxisRange(5100,5500);
    
    TF1 *BacPos      = new TF1("Bac",   BackGround, LIML, LIMH, NUM_PAR_BAC);
    TF1 *FourBodyPos = new TF1("4Body", Shoulder,   LIML, LIMH, NUM_PAR_SHO); 
    TF1 *SigPos      = new TF1("Sig",   Signal,     LIML, LIMH, NUM_PAR_SIG);
    SigPos->SetLineColor(kBlue);
    BacPos->SetLineColor(kYellow);
    FourBodyPos->SetLineColor(kGreen);

    Double_t parPos[NUM_PAR];
    Double_t *errPos;
    ComPos->GetParameters(parPos);
    errPos = (Double_t*)ComPos->GetParErrors();
    SigPos->SetParameters(&parPos[0]);
    FourBodyPos->SetParameters(&parPos[NUM_PAR_SIG]);
    BacPos->SetParameters(&parPos[NUM_PAR - NUM_PAR_BAC]);

    SigPos->Draw("same");
    BacPos->Draw("same");
    FourBodyPos->Draw("same");

    c8->SaveAs("Plots/c8_Background&SignalFitsPos.pdf");

//fitting Negtive
    TCanvas *c9 = new TCanvas("c9","",600,400);
    B0Neg->SetAxisRange(0,120,"Y");
    TFitResultPtr negPtr = B0Neg->Fit(ComNeg,"R");
    B0Neg->SetAxisRange(5100,5500);

    TF1 *BacNeg      = new TF1("Bac",   BackGround, LIML, LIMH, NUM_PAR_BAC);
    TF1 *FourBodyNeg = new TF1("4Body", Shoulder,   LIML, LIMH, NUM_PAR_SHO); 
    TF1 *SigNeg      = new TF1("Sig",   Signal,     LIML, LIMH, NUM_PAR_SIG);
    SigNeg->SetLineColor(kBlue);
    BacNeg->SetLineColor(kYellow);
    FourBodyNeg->SetLineColor(kGreen);


    Double_t parNeg[NUM_PAR];
    Double_t *errNeg;
    ComNeg->GetParameters(parNeg);
    errNeg = (Double_t*)ComNeg->GetParErrors();
    SigNeg->SetParameters(&parNeg[0]);
    FourBodyNeg->SetParameters(&parNeg[NUM_PAR_SIG]);
    BacNeg->SetParameters(&parNeg[NUM_PAR-NUM_PAR_BAC]);
    

    SigNeg->Draw("same");
    BacNeg->Draw("same");
    FourBodyNeg->Draw("same");

    c9->SaveAs("Plots/c9_Background&SignalFitsNeg.pdf");

//Error Calculating
    #ifdef SIGNAL_LORENTZ
        Double_t NPos = parPos[0]*parPos[1]*TMath::Pi();
        Double_t NNeg = parNeg[0]*parNeg[1]*TMath::Pi();
        Double_t ErrNPos = ErrAMultB(NPos,parPos[0], parPos[1],errPos[0], errPos[1]);
        Double_t ErrNNeg = ErrAMultB(NNeg,parNeg[0], parNeg[1],errNeg[0], errNeg[1]);
    #endif

    #ifdef SIGNAL_CRUIJFF
        #ifdef SIGNAL_DOUBLE_GAUS
            Double_t NPos = SigPos->Integral(LIML,LIMH);
            Double_t NNeg = SigNeg->Integral(LIML,LIMH);
            Double_t ANPos = parPos[5]*(parPos[3]+parPos[1])*sqrt(TMath::Pi()*2)/2;
            Double_t ANNeg = parNeg[5]*(parNeg[3]+parNeg[1])*sqrt(TMath::Pi()*2)/2;
            cout<<NPos<<'\t'<<ANPos<<endl;
            cout<<NNeg<<'\t'<<ANNeg<<endl;
            Double_t ErrSigPos = ErrAPlusB(errPos[1],errPos[3]);
            Double_t ErrSigNeg = ErrAPlusB(errNeg[1],errNeg[3]);
            Double_t ErrNPos = ErrAMultB(NPos, parPos[5], parPos[3]+parPos[1], errPos[5], ErrSigPos);
            Double_t ErrNNeg = ErrAMultB(NNeg, parNeg[5], parNeg[3]+parNeg[1], errNeg[5], ErrSigNeg);
        #else
        //TODO Numerical Method
        //!analytical, notgood,  Alpha tails has been ignored ,for double Gaussian it's good
        /*
            Double_t NPos = SigPos->Integral(LIML,LIMH);
            Double_t NNeg = SigNeg->Integral(LIML,LIMH);
            Double_t ErrNPos = SigPos->IntegralError(LIML,LIMH);//, parPos, posPtr->GetCovarianceMatrix()->GetMatrixArray());
            Double_t ErrNNeg = SigNeg->IntegralError(LIML,LIMH);//, parNeg, negPtr->GetCovarianceMatrix()->GetMatrixArray());*/
            Double_t NPos = SigPos->Integral(LIML,LIMH);
            Double_t NNeg = SigNeg->Integral(LIML,LIMH);
            Double_t ANPos = parPos[5]*(parPos[3]+parPos[1])*sqrt(TMath::Pi()*2)/2;
            Double_t ANNeg = parNeg[5]*(parNeg[3]+parNeg[1])*sqrt(TMath::Pi()*2)/2;
            cout<<NPos<<'\t'<<ANPos<<endl;
            cout<<NNeg<<'\t'<<ANNeg<<endl;
            Double_t ErrSigPos = ErrAPlusB(errPos[1],errPos[3]);
            Double_t ErrSigNeg = ErrAPlusB(errNeg[1],errNeg[3]);
            Double_t ErrNPos = ErrAMultB(NPos, parPos[5], parPos[3]+parPos[1], errPos[5], ErrSigPos);
            Double_t ErrNNeg = ErrAMultB(NNeg, parNeg[5], parNeg[3]+parNeg[1], errNeg[5], ErrSigNeg);
        #endif
    #endif
    Double_t Asym = (NNeg-NPos)/(NNeg+NPos);
    Double_t ErrN    = ErrAPlusB(ErrNPos, ErrNNeg);
    Double_t EffPos  = NPos/(NPos+NNeg);
    Double_t EffNeg  = NNeg/(NPos+NNeg);

    Double_t ErrEffPos = ErrAMultB(EffPos,NPos, NPos+NNeg, ErrNPos,ErrN);
    Double_t ErrEffNeg = ErrAMultB(EffNeg,NNeg, NPos+NNeg, ErrNNeg,ErrN);

    Double_t ErrA      = ErrAMinuB(ErrEffNeg, ErrEffPos);

//Output
    cout<<"Number of B+  =\t"<<NPos<<" +- "<<ErrNPos<<endl;
    cout<<"Number of B-  =\t"<<NNeg<<" +- "<<ErrNNeg<<endl;
    cout<<"Efficiency N+ =\t"<<EffPos<<" +- "<<ErrEffPos<<endl;
    cout<<"Efficiency N- =\t"<<EffNeg<<" +- "<<ErrEffNeg<<endl;
    cout<<"Effi Glb Asym =\t"<<Asym<<" +- "<<ErrA<<endl;
    ErrA = ErrAMultB(Asym, NNeg-NPos, NNeg+NPos, ErrN, ErrN);
    cout<<"Devi Glb Asym =\t"<<Asym<<" +- "<<ErrA<<endl;
    cout<<"Predicted Error =\t"<<sqrt((1-Asym*Asym)/(NPos+NNeg))<<endl;

}