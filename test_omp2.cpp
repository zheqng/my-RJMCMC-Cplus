#include <string.h>
#include "mix_lib.h"
#include <iostream>
#include <fstream>
#include "armadillo"
#include <ctime>



/*Definitions                */
#define NArgs 17
#define StrLen 40
#define ArgExt  ".arg"
#define ResExt  ".res"
#define StatExt  ".sts"


//
//using namespace std;
//using namespace arma;
/*Global variables                            */
/*Seeds for the kiss generator (see alea)     */

/*Files                                      */
Generator g;
char DataFile[StrLen], StatFile[StrLen];
ofstream parameterFP,zFP, StatFP,testfile;
/*Prior hyperparameters                    */
int Kmax;
/*Sampler settings                        */
int NOut, SubSamp, NIt;

/*Fixed k move                             */
double PFixed;
/*Birth and death                          */
double PBirth, PDeath, PFixed_or_BD;
/*Split and merge                          */
double PSplit, PMerge;
int Curve_num,THREAD_NUM;
int Nm;
double w_eps,v0_eps,sigmav2_eps;

/* Function prototypes							*/
struct STATS {
        string split_or_merge,acc_or_rej,simu_acc_or_simu_rej,delete_empty_component;
        double sm_prob,simu_prob;
        void initialize(){
                split_or_merge = "NULL";
                acc_or_rej = "NULL";
                simu_acc_or_simu_rej = "NULL";
                delete_empty_component = "NULL";
                sm_prob = 0.0;
                simu_prob = 0.0;
        }
        void print(){
                cout <<  split_or_merge
                     <<" "<<acc_or_rej<<" "<<sm_prob<<" "<<
                        simu_acc_or_simu_rej<<" "<<simu_prob<<" "<<delete_empty_component<<endl;
        }
        void write_file(ofstream & myfile){
                myfile<<  split_or_merge
                      <<" "<<acc_or_rej<<" "<<sm_prob<<" "<<
                        simu_acc_or_simu_rej<<" "<<simu_prob<<" "<<delete_empty_component<<endl;
        }
};
void read_parameters(int argc, char** argv, curve Data[] );
// void write_data(pq_point & theta, double logl,STATS & stats,vec & z,int in);
// void RJMH_birth_or_death(curve Data[], pq_point & m,double* logl,vec & z,STATS & stats);
// void RJMH_split_or_merge(curve Data[], pq_point & m, double* logl,STATS & stats);
// double Simulated_Annealing( double *logl, double *logl_old,pq_point & theta, pq_point & theta_old,int in,STATS & stats);

int main(int argc, char** argv) {
        curve Data[MaxM];
        pq_point theta(1);
        double ran;
        double logl;
        int in;
        vec z;
        STATS stats;

        stats.initialize();
        time_t t_start,t_end;
        ofstream TimeFP("time.txt");
        double DiffTime;
        t_start = time(NULL);

        read_parameters(argc, argv, Data);
        // omp_set_nested(10);
        // omp_set_max_active_levels(4);
        draw_initial_model(Data, theta, &logl);
        cout<<"Difftime:"<<DiffTime<<"log likelihood:"<<logl<<endl;

        return(0);
}


/************************************************************************/
/* Read parameters (including seeds for the random generator) and data  */
/************************************************************************/
void read_parameters(int argc, char** argv, curve Data[] ) {
        testfile.open("generator.res");
        char argfile[StrLen];

        strcpy(StatFile, argv[1]);
        strcpy(DataFile, argv[1]);
        strcpy(argfile, argv[1]);

        strcat(argfile, ArgExt);
        ifstream argfp(argfile);
        argfp>>g.i>>g.j>>g.k>>NOut>>THREAD_NUM>>SubSamp>>Kmax>>PFixed>>PBirth>>PDeath>>PSplit;
        argfp.close();
        /* Compute frequently used quantities					*/
        NIt = NOut*SubSamp;
        PMerge = 1.0 - (PFixed + PBirth + PDeath + PSplit);
        PFixed_or_BD = PFixed + PBirth + PDeath;

        /* Read data								*/
        strcat(DataFile,".dat");
        mat AAA;
        AAA.load(DataFile);
        Curve_num =   AAA.n_rows/2;
        Nm = AAA.n_cols;
        #pragma omp parallel for
        for(int m=0; m<Curve_num; m++) {
                Data[m].X =  AAA.row(2*m).t();
                Data[m].Y = AAA.row(2*m+1).t();
                // cout<<"read data"<<omp_get_thread_num()<<endl;
        }

        w_eps = 5e-6;
        v0_eps = 0.5;
        sigmav2_eps = 0.005;
        printf( "Data has %d curves. Running %d x %d iterations of the sampler...\n", Curve_num, NOut, SubSamp);

}
