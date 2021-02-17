#define GCC

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include "tools.h"
// #include "test_matrix.h"
// B (PKT_SIZE,DEGREE)
// G (DEGREE,BATCH_SIZE)
#include "test_matrix.h"
using namespace std;

bool check_exit(uint8_t sample_idx[N_BATCH*MAX_DEGREE],int count, int offset, uint8_t num){
    for(int i=offset;i<count+1;i++){
        if(sample_idx[i] == num){
            return true;
        }
    }
    return false;
}

template<int N_BATCH_>
int get_B_size(uint8_t deg_list[N_BATCH_],int n_batch){
    int len = 0;
    for(int i=0;i<n_batch;i++){
        len+=deg_list[i];
    }    
    return len*BATCH_SIZE;
}

template<int N_BATCH_>
int get_A_size(uint8_t deg_list[N_BATCH_],int n_batch){
    int len = 0;
    for(int i=0;i<n_batch;i++){
        len+=deg_list[i];
    }    
    return len*PKT_SIZE;
}

template<int N_BATCH_>
int get_sampleIdx_size(uint8_t deg_list[N_BATCH_],int n_batch){
    int len = 0;
    for(int i=0;i<n_batch;i++){
        len+=deg_list[i];
    }    
    return len;
}



// int main(){
  
//     int offset_list[N_BATCH]={0};
//     for(int j=0;j<N_BATCH;j++){
//       for(int jj=0;jj<j;jj++){
//         offset_list[j] += deg_list[jj];
//       }
//     }
//     // printf("here\n");
//     uint8_t ref_output[PKT_SIZE*BATCH_SIZE*N_BATCH];
//     int golden_A[64*N_BATCH*PKT_SIZE];
//     int count = 0;
//     for(int idx=0;idx<N_BATCH;idx++){
//       int cur_deg =deg_list[idx];
//       int cur_offset = offset_list[idx];
//       for(int dd=0;dd<cur_deg;dd++){
//         int cur_sample_idx = sample_idx[cur_offset+dd];
//         for(int pk=0;pk<PKT_SIZE;pk++){
//           golden_A[count] = input_file[pk+cur_sample_idx*PKT_SIZE];
//           count++;
//         }
//       }
//     }
//     // compute golden A * matrix B
//     for(int b=0;b<N_BATCH;b++){
//       int d = deg_list[b];
//       for (int m=0; m<PKT_SIZE; m++) {
//           for (int n=0; n<BATCH_SIZE; n++) {
//               uint8_t acc = 0;
//               for (int k=0; k<d; k++) {
//                   acc ^= gf_mu_x86(golden_A[k*PKT_SIZE + m + PKT_SIZE*offset_list[b]],B[n*d + k + BATCH_SIZE*offset_list[b]]);
//               }
//               ref_output[n*PKT_SIZE + m + b*PKT_SIZE*BATCH_SIZE] = acc;
//               // printf("%d,",acc);
//           }
//       }
//     }
//     for(int i=0;i<PKT_SIZE*BATCH_SIZE*N_BATCH;i++){
//         printf("%d,",ref_output[i]);
//         if(i % 50 ==0&&i!=0){
//             printf("\n");
//         }
//     }
//   return 0;
// }

int main(){
    char buf[1000] = {0};
    int n = 0;
    srand(SEED-1);
    ofstream myfile;
    myfile.open ("./include/test_matrix.h");

    // generate file
    n = sprintf(buf,"#include \"config.h\"\n\nstatic uint8_t input_file[%d] = {\n\t\t",FILE_SIZE);
    myfile << buf;
    // print arrays
    // A (PKT_SIZE,DEGREE)
    uint8_t input_file[FILE_SIZE];
    for(int j=0;j<FILE_SIZE;j++){
        
        uint8_t num = rand() % 256;
        input_file[j] = num;
        n = sprintf(buf,"%d,",num);
        myfile << buf;
        if(j % 50 ==0){
            myfile << "\n\t\t";
        }
    }
    myfile << "\n\t}; // FILE_SIZE\n";
    
    // degree
    uint8_t deg_list[N_BATCH];
    n = sprintf(buf,"\n\nstatic uint8_t deg_list[%d] = {",N_BATCH);
    myfile << buf;
    for(int j=0;j<N_BATCH;j++){
        // int num = rand() % 64;
        int num = 16;
        while(num % TS != 0 || num ==0){
            num = rand() % 64;
        }
        n = sprintf(buf,"%d,",num);
        myfile << buf;
        deg_list[j] = num;
    }
    myfile << "}; // N_BATCH\n";

    // generate sample idx
    uint8_t sample_idx[N_BATCH*MAX_DEGREE]={0};
    int offset_list[N_BATCH]={0};
    for(int j=0;j<N_BATCH;j++){
      for(int jj=0;jj<j;jj++){
        offset_list[j] += deg_list[jj];
      }
    }

    n = sprintf(buf,"\n\nstatic uint8_t sample_idx[%d] = {\n\t\t",get_sampleIdx_size<N_BATCH>(deg_list,N_BATCH));
    myfile << buf;
    int count=0;
    for(int j=0;j<N_BATCH;j++){
        uint8_t cur_deg = deg_list[j];
        int cur_offset = offset_list[j];
        for(int d=0;d<cur_deg;d++){
            uint8_t num = rand() % PKT_NUM;
            while(check_exit(sample_idx,count,cur_offset,num)){
                num = rand() % PKT_NUM;
            }
            sample_idx[count] = num;
            
            n = sprintf(buf,"%d,",num);
            myfile << buf;
            count++;
        }
        myfile << "\n\t\t";
    }
    myfile << "}; // DEGREE*N_BATCH\n";

   
    // B (DEGREE,BATCH_SIZE)
    uint8_t B[MAX_DEGREE*BATCH_SIZE];
    count = 0;
    srand(SEED);
    n = sprintf(buf,"\n\nstatic uint8_t B[%d] = {\n\t\t",get_B_size<N_BATCH>(deg_list,N_BATCH));
    myfile << buf;
    for(int j=0;j<N_BATCH;j++){
        uint8_t cur_deg = deg_list[j];
        for(int d=0;d<cur_deg;d++){
            for(int b=0;b<BATCH_SIZE;b++){
                int num = rand() % 256;
                B[count] = num;
                n = sprintf(buf,"%d,",num);
                myfile << buf;
            }
            myfile << "\n\t\t";
        }
    }
    myfile << "\n\t}; // (DEGREE,BATCH_SIZE)\n";


    // for(int j=0;j<DEGREE;j++){
    //     myfile << "\n\t\t{";
    //     for(int i=0;i<BATCH_SIZE;i++){
    //         int num = rand() % 256;
    //         G[j][i] = num;
    //         n = sprintf(buf,"%d,",num);
    //         myfile << buf;
    //     }
    //     myfile << "},";
    // }
    // myfile << "\n\t};\n";
    
   


    // // multiply them
    // uint8_t B_[PKT_SIZE*DEGREE];
    // uint8_t G_[DEGREE*BATCH_SIZE];
    // uint8_t res[PKT_SIZE][BATCH_SIZE];
    // uint8_t res_[PKT_SIZE*BATCH_SIZE];
    // matrix_flatten<PKT_SIZE,DEGREE>(B,B_);
    // matrix_flatten<DEGREE,BATCH_SIZE>(G,G_);
    // matrix_multi(B_,G_,res_,PKT_SIZE,DEGREE,DEGREE);
    // matrix_reform<PKT_SIZE,BATCH_SIZE>(res_,res);

    // myfile << "\n\nstatic uint8_t res[PKT_SIZE][BATCH_SIZE] = {";
    // for(int j=0;j<PKT_SIZE;j++){
    //     myfile << "\n\t\t{";
    //     for(int i=0;i<BATCH_SIZE;i++){
    //         n = sprintf(buf,"%d,",res[j][i]);
    //         myfile << buf;
    //     }
    //     myfile << "},";
    // }
    // myfile << "\n\t};\n";

    


    myfile.close();
    return 0;


}

