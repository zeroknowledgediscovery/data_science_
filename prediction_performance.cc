#include<iostream>
#include<fstream>
#include<set>
#include<map>
#include<vector>
#include<string>
#include<sstream>
#include<stdlib.h>
#include<cmath>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#include <exception>
#include <boost/timer/timer.hpp>
#include <sys/resource.h>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::program_options;
using namespace boost::filesystem;

const string VERSION="PREDICTION_PERFORMANCE v0.5 \n Copyright Ishanu Chattopadhyay 2017";
const string EMPTY_ARG_MESSAGE="Exiting. Type -h or --help for usage";
bool TIMER=false;
//#############################################

typedef vector< vector<double> > matrix_type__;
typedef map<string,map<double,double> > perf_record__;
//#############################################
//##############################################

ostream& operator << (ostream &out, map<double,double> s)
{
  for (map<double,double>::iterator itr=s.begin();
       itr != s.end();
       ++itr)
    out << itr->first << " " << itr->second << endl;
  return out;
}
//##############################################
//##############################################


ostream& operator << (ostream &out,
		      perf_record__ s)
{
  vector<string> ns;
  vector<double> tvec;
  
  for(perf_record__::iterator itr=s.begin();
      itr != s.end();
      ++itr)
    ns.push_back(itr->first);
  
  for(map<double,double>::iterator itr=s[ns[0]].begin();
      itr!=s[ns[0]].end();
      ++itr)
    tvec.push_back(itr->first);

  out<< "TOL ";
  for(unsigned int j=0;j<ns.size();++j)
    out << ns[j] << " ";
  out<< endl;
  
  for(unsigned int i=0;i<tvec.size();++i)
    {
      out << tvec[i] << " ";
      for(unsigned int j=0;j<ns.size();++j)
	out << s[ns[j]][tvec[i]] << " ";
      out << endl;
    }
  
  return out;
}


//#############################################
//##############################################
void getfilenames(string path_,
		  string patt_,
		  set<path>& fnames)
{
  path p(current_path());
  p = system_complete(path_.c_str());

  if (!exists(p))
    {
      cout << "\nNot found: " << p << endl;
      return;
    }
  if (is_directory(p))
    {
      cout << "In directory: " << p << endl;
      directory_iterator end_iter;
      for (directory_iterator dir_itr(p);
	   dir_itr != end_iter;
	   ++dir_itr)
	try
	  {
	    if ((is_regular_file(dir_itr->status()))
		&&(dir_itr->path().filename().
		   string().find(patt_)!=string::npos))
	      fnames.insert(p/dir_itr->path().filename());
	  }
	catch (const exception & ex)
	  {
	    cout << dir_itr->path().filename()
		 << " " << ex.what() << endl;
	  }
    }
  else
    cout << "Target not a directory" << endl;
  return;
};

//#############################################
//##############################################

void readinfiles(set<path> namepath,
		 matrix_type__& H)
{
  for(set<path>::iterator itr=namepath.begin();
      itr!=namepath.end();
      ++itr)
    {
      std::ifstream IN(itr->c_str());
      string line;
      while(getline(IN,line))
	{
	  stringstream ss(line);
	  double tmp;
	  vector<double> tmpvec;
	  while(ss>>tmp)
	    tmpvec.push_back(tmp);
	  H.push_back(tmpvec);
	}
      IN.close();
    }
  return;
};

//#############################################
//##############################################
// POS NEG

perf_record__& getmetrics(double TOL__,
			  vector<matrix_type__>& H,
			  perf_record__& R)
{
  for(unsigned int i=00;i<100;++i)
    {
      double TOL_=TOL__*(i/100.0);
      int TP=0,TN=0,FP=0,FN=0;
      for(unsigned int i=0;i<H[0].size();++i)
	{
	  TP+=(H[0][i][0]-H[0][i][1] <= -TOL_);
	  FN+=(H[0][i][0]-H[0][i][1] > TOL_);
	}
      for(unsigned int i=0;i<H[1].size();++i)
	{
	  TN+=(H[1][i][0]-H[1][i][1] > TOL_);
	  FP+=(H[1][i][0]-H[1][i][1] <= -TOL_);
	}
      double AC=(TP+TN)/(TP+FP+TN+FN+0.0);
      double FD=FP/(FP+TP+0.0);
      double RC=TP/(TP+FN+0.0);
      double PD=(TP+FP+TN+FN)/(H[0].size()+H[1].size()+0.0);
      double TPR=TP/(TP+FN+0.0);
      double TNR=TN/(TN+FP+0.0);
      double PC=TP/(TP+FP+0.0);
      double FPR=FP/(TN+FP+0.0);
      R["TP"][TOL_]=TP;
      R["FP"][TOL_]=FP;
      R["TN"][TOL_]=TN;
      R["FN"][TOL_]=FN;
      R["AC"][TOL_]=AC;
      R["FD"][TOL_]=FD;
      R["RC"][TOL_]=RC;
      R["PD"][TOL_]=PD;
      R["TPR"][TOL_]=TPR;
      R["TNR"][TOL_]=TNR;
      R["PC"][TOL_]=PC;
      R["FPR"][TOL_]=FPR;
    }
  return R;
};

//#############################################
//##############################################
int main(int argc, char* argv[])
{
  string outfile="out.txt";
  vector<string> namepatt;
  string DIR="./";
  //  unsigned int TOP=0;
  double tolr=0.025;
  bool VERBOSE=false;
  string metric="";
  
  options_description infor( "Program information");
  infor.add_options()
    ("help,h", "print help message.")
    ("version,V", "print version number");

  options_description usg( "Usage");
  usg.add_options()
    ("filenamepattern,p",value<vector<string> >()->multitoken(), "Unique string in filenames []")
    ("directory,D",value<string>(), "Directory to find files  [./]")
    ("tolerance,d",value<double>(), "Tolerance factor [0.025]");


  options_description outputopt( "Output options");
  outputopt.add_options()
    ("ofile,o",value< string>(), "outfile [out.txt]")
    ("metric,m",value< string>(), "metric requested (AC FD FN FP FPR PC PD RC TN TNR TP TPR) []")
    ("VERBOSE,v",value<bool>(), "On-screen output [0/off]");
  //("top,t",value<unsigned int>(), "Number of elements to report [all,0]");
  
  options_description desc( "\n\n\
                               ,%%%%%%%%%%%%%%%%%%%                            \n\
                         (%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                     \n\
                     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%,                \n\
                  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%&             \n\
               *%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%&          \n\
             %%%%%%%%%%%%%%%%%%%%%%,          *%%%%%%%%%%%%%%%%%%%%%%%%        \n\
           %%%%%%%%%%%%%%%%%.                         /%%%%%%%%%%%%%%%%%%/     \n\
         %%%%%%%%%%%%%%%%                                 (%%%%%%%%%%%%%%%%    \n\
       %%%%%%%%%%%%%%%%                                      %%%%%%%%%%%%%%%%  \n\
     %%%%%%%%%%%%%%%%%                                         %%%%%%%%%%%%%%%.\n\
   %%%%%%%%%%%%%%%%%%                                                          \n\
%%%%%%%%%%%%%%%%%%%%%         PREDICTION PERFORMANCE                           \n\
   &%%%%%%%%%%%%%%%%%    copyright ishanu chattopadhyay 2017                   \n\
     &%%%%%%%%%%%%%%%%            ishanu@uchicago.edu          &%%%%%%%%%%%%%%#\n\
       %%%%%%%%%%%%%%%%                                      &%%%%%%%%%%%%%%%  \n\
         %%%%%%%%%%%%%%%%       PIPELINE SECTION IV        %%%%%%%%%%%%%%%%,   \n\
           %%%%%%%%%%%%%%%%%                           &%%%%%%%%%%%%%%%%%%     \n\
             %%%%%%%%%%%%%%%%%%%%%#            #%%%%%%%%%%%%%%%%%%%%%%%,       \n\
               %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%&          \n\
                  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%     %       \n\
                     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                \n\
                         %&%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%&                     \n\
                               %%%%%%%%%%%%%%%%%%%%,");
  desc.add(infor).add(usg).add(outputopt);

  positional_options_description p;
  variables_map vm;
  if (argc == 1)
    {
      cout << EMPTY_ARG_MESSAGE << endl;
      return 1;
    }
  try
    {
      store(command_line_parser(argc, argv)
	    .options(desc)
	    .run(), vm);
      notify(vm);
    } 
  catch (std::exception &e)
    {
      cout << endl << e.what() 
	   << endl << desc << endl;
      return 1;
    }
  if (vm.count("help"))    
    {
      cout << desc << endl;
      return 1;
    }
  if (vm.count("version"))
    {
      cout << VERSION << endl; 
      return 1;
    }
  
  if (vm.count("filenamepattern"))
    namepatt=vm["filenamepattern"].as<vector<string> >();
  if (vm.count("directory"))
    DIR=vm["directory"].as<string>();
  if (vm.count("tolerance"))
    tolr=vm["tolerance"].as<double>();
  /*  if (vm.count("top"))
      TOP=vm["top"].as<int>();
  */
  if (vm.count("VERBOSE"))
    VERBOSE=vm["VERBOSE"].as<bool>();

  if (vm.count("ofile"))
    outfile=vm["ofile"].as<string>();
  if (vm.count("metric"))
    metric=vm["metric"].as<string>();

  //#############################################
  
  vector<set<path> > fnames(namepatt.size());
  vector<matrix_type__> H(namepatt.size());
  
  for(unsigned int i=0;i<namepatt.size();++i)
    {
      getfilenames(DIR,namepatt[i],fnames[i]);
      readinfiles(fnames[i],H[i]);
    }
  std::ofstream OUT(outfile.c_str());
  perf_record__ R;
  OUT << getmetrics(tolr,H,R);
  OUT.close();

  cout << " xxx " << metric <<  endl;
  
  if(metric!="")
    cout << R[metric];
  else
    {
      if(VERBOSE)
	cout << R;
    }
  return 0;
}


  //#############################################
