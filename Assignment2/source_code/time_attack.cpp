#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cerrno>
#include <iomanip> 
using namespace std;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <wait.h>
#include <unistd.h>

#include <inttypes.h>
#include <vector>
#include <math.h>



class Letter {
      public: int index;//represents the letter value
      public: uint64_t sum;
      public: uint64_t square_sum;
      public: double variance;
      public: double mean;
	  public: double std_dev;
      public: Letter(int index){
              this->index=index;
              this->sum = 0;
              this->square_sum= 0;
              this->variance=0;
              this->mean=0;
			  this->std_dev=0;
      } 
      
};


static __inline__ uint64_t rdtsc()
{
   uint32_t hi, lo;
   __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
   return ((uint64_t)lo)|(((uint64_t)hi)<<32);
}

bool check_password(string input){
   return (input=="passwordabc");
}

int main(int argc, char* argv[])
{	
	string pw;
    if (argc <  2){ 
        pw ="";
    }
	else pw=argv[1];
	int base=97;//base ascii for a
			
	//initialize
	vector<Letter> letters;
	bool res;
	for(int i=0;i<26;i++){
		letters.push_back(i);  
	}
	
	random_shuffle ( letters.begin(), letters.end() );
	
	uint64_t start;
	uint64_t end;
	uint64_t duration;
	bool next_letter_found=false;
	bool pw_found=false;
    double max=0;
	int maxindex=0;
	double next_conf_val=0;
	int j=0;//iteration count
	while(!next_letter_found){
		
		random_shuffle ( letters.begin(), letters.end() ); //shuffle between each iteration
		for(int i=0;i<26;i++){
			char letter=letters[i].index+base;
			//cout<<"letter:"<<letter<<endl;
			
			 
			start=rdtsc();
			res=check_password(pw+letter);
			end=rdtsc();
			 
			if(res){
				cout <<"Password verified!\n The password is "<<pw+letter<<endl;
				next_letter_found=true;
				//pw_found=true;
				break;
			} 
			 
			duration =(end-start);
			if (j>1000){ 
				letters[i].sum+=duration;
				letters[i].square_sum+=duration*duration;
				letters[i].mean=letters[i].sum/(double)j;
				letters[i].variance=(1/((double)j-1))*(letters[i].square_sum-(j*(letters[i].mean)*(letters[i].mean)));
				letters[i].std_dev=sqrt(letters[i].variance/(double)j);
				if ((double)letters[i].mean>max){
					max =(double)letters[i].mean;
					maxindex=letters[i].index;
				}
				else if (letters[i].mean+letters[i].std_dev>next_conf_val){
					next_conf_val=letters[i].mean+1.96*letters[i].std_dev;
				}
				//check if confidence interval of highest mean overlaps with next highest confidence interval
				if (letters[maxindex].mean-1.96*(letters[maxindex].std_dev)>next_conf_val){
					for(int k=0;k<26;k++){
						//cout<<pw+letter<<" "<<
						cout<<pw+char(letters[k].index+base)<<" mean: "<<letters[k].mean<<" variance:"<<letters[k].variance<<" avg std_dev: "<<letters[k].std_dev<<endl;
					}
					cout<<"\n\n95% confident the next letter is: "<<(char)(letters[maxindex].index+base)<<endl;
					cout<<"The password so far is:"<<pw+(char)(letters[maxindex].index+base)<<endl<<endl;
					/*pw=pw+(char)(letters[maxindex].index+base);
					for(int i=0; i<26;i++){
					    letters[i].sum=0; 
						letters[i].square_sum= 0;
						letters[i].variance=0;
						letters[i].mean=0;
						letters[i].std_dev=0; 
						 
					}
					j=0;
					max=0;
					maxindex=0;
					next_conf_val=0;*/
					//pw_found=true;
					next_letter_found=true;
					break;
				}
			}	
			//if (j%100000==0)cout<<pw+letter<<" "<<char(letters[i].index+base)<<" mean: "<<letters[i].mean<<" variance:"<<letters[i].variance<<" std_dev: "<<letters[i].std_dev<<endl;

		}
		//if(j%100000==0){
		//cout<<(char)(maxindex+base)<<":"<<letters[maxindex].mean-1.96*(letters[maxindex].std_dev);
		//cout<<""
		//}
		j++;

	}



    return 0;
}
