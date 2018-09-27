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

using namespace std;
using namespace boost::program_options;

const string VERSION="MODEL_CLOUD v0.5 \n Copyright Ishanu Chattopadhyay 2017\n 1. Reads in H.txt and E.txt \n 2. Calculates the mean coordiante \n 3. Lists the points which are closest to the mean \n X. Objective is to identify the model cloud with its centroid";
const string EMPTY_ARG_MESSAGE="Exiting. Type -h or --help for usage";
bool TIMER=false;

//---------------------------------
//-------------------------------------------

typedef map<double,set<unsigned int> > matrix_type__;

//-------------------------------------------
//-------------------------------------------

void  read_matrix(string filename,
		 vector<vector<double> >& M,
		 bool DIAGNOSTICS=false);
namespace UTIL__
{
  void exception(string message,bool ext=false);
  double norm(vector<double>, vector<double>);
  string fconstx(string PATH,unsigned int WIDTH,unsigned int index);
}
//-------------------------------------------
//-------------------------------------------

ostream& operator << (ostream &out, vector <double> &s)
{
  for (unsigned int i=0; i<s.size(); i++)
    out << s[i] << " ";
  out << endl;
  return out;
}
//-------------------------------------------
//-------------------------------------------

ostream& operator << (ostream &out, set <unsigned int> &s)
{
  for (set <unsigned int>::iterator itr=s.begin();
       itr!=s.end();
       ++itr)
    out << *itr << " ";
  return out;
}
//-------------------------------------------
//-------------------------------------------

string UTIL__::fconstx(string PATH,
		   unsigned int WIDTH,
		   unsigned  int index)
{
  string s_index=boost::lexical_cast<string>(index);
  while(s_index.length()<WIDTH)
    s_index="0"+s_index;
  return PATH+s_index;
};


//-------------------------------------------
//-------------------------------------------

double UTIL__::norm(vector<double> v1,
		  vector<double> v2)
{
  double s=0.0;
  if(v1.size()==v2.size())
    for(unsigned int i=0;i<v1.size();++i)
      s+=(v1[i]-v2[i])*(v1[i]-v2[i]);
  
  return sqrt(s);  
}
//-------------------------------------------
//------------------------------------------

void UTIL__::exception(string message,bool ext)
{
  cout << message <<endl;
  if(ext)
    exit(0);
};

//-------------------------------------------
//-------------------------------------------

void read_matrix(string filename,
		 vector<vector<double> >& M,
		 bool DIAGNOSTICS)
{
  M.clear();

  ifstream IN(filename.c_str());
  if(!IN.is_open())
    UTIL__::exception("CANNOT READ FILE",true);
    
  string line;
  while(getline(IN,line))
    {
      stringstream ss(line);
      double tmp;
      vector<double> Mrow;
      while(ss>>tmp)
	Mrow.push_back(tmp);
      if(!Mrow.empty())
	  M.push_back(Mrow);
    }
  IN.close();
  
  if(DIAGNOSTICS)
    {
      cout<< "# of rows: " << M.size() << endl;
      cout << "# of cols: ";
      for(unsigned int i=0;i<M.size();++i)
	cout<< i+1 << " " << M[i].size() << " : " ;
      cout << endl;
    }

  if(!M.empty())
    if(M.back().size()!=M[0].size())
      UTIL__::exception("COL LENGTH MISMATCH",false);
  
  return;
}
//-------------------------------------------
//-------------------------------------------

int main(int argc, char* argv[])
{
  string filenameE="E.txt",filenameH="H.txt",outfile="out.txt";
  bool DIAGNOSTICS=false;
  unsigned int dim=3;
  unsigned int TOP=0;
  bool VERBOSE=false;
  string PATHPREF="test/MPOS";
  unsigned int WIDTH=3;

  
  options_description infor( "Program information");
  infor.add_options()
    ("help,h", "print help message.")
    ("version,V", "print version number");

  options_description usg( "Usage");
  usg.add_options()
    ("Efile,E",value<string>(), "Coordinate file or E-file [E.txt]")
    ("Hfile,H",value<string>(), "Distance matrix or H-file [H.txt]")
    ("dim,d",value<unsigned int>(), "Number of dimensions considered [3]");


  options_description outputopt( "Output options");
  outputopt.add_options()
    ("ofile,o",value< string>(), "outfile [out.txt]")
    ("width,w",value<unsigned int>(), "Filename index width in integer [3]")
    ("pref,p",value<string>(), "Path+pref to model filename [test/MPOS_]")
    ("DIAGNOSTICS,D",value<bool>(), "turn on diagnostics [0/off]")
    ("VERBOSE,v",value<bool>(), "On-screen output [0/off]")
    ("top,t",value<unsigned int>(), "Number of elements to report [all,0]");
  
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
%%%%%%%%%%%%%%%%%%%%%           MODEL CLOUD EXTRACTOR                          \n\
   &%%%%%%%%%%%%%%%%%    copyright ishanu chattopadhyay 2017                   \n\
     &%%%%%%%%%%%%%%%%            ishanu@uchicago.edu          &%%%%%%%%%%%%%%#\n\
       %%%%%%%%%%%%%%%%                                      &%%%%%%%%%%%%%%%  \n\
         %%%%%%%%%%%%%%%%       PIPELINE SECTION II        %%%%%%%%%%%%%%%%,   \n\
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
  
  if (vm.count("Efile"))
    filenameE=vm["Efile"].as<string>();
  if (vm.count("Hfile"))
    filenameH=vm["Hfile"].as<string>();
  if (vm.count("pref"))
    PATHPREF=vm["pref"].as<string>();
  if (vm.count("width"))
   WIDTH=vm["width"].as<unsigned int>();
  if (vm.count("dim"))
   dim=vm["dim"].as<unsigned int>();
  if (vm.count("top"))
    TOP=vm["top"].as<unsigned int>();
  if (vm.count("DIAGNOSTICS"))
    DIAGNOSTICS=vm["DIAGNOSTICS"].as<bool>();
  if (vm.count("VERBOSE"))
    VERBOSE=vm["VERBOSE"].as<bool>();

  if (vm.count("ofile"))
    outfile=vm["ofile"].as<string>();

  //#############################################
  string PATH="./";

  
  vector< vector<double> > H,E;

  read_matrix(filenameE,E,DIAGNOSTICS);
  read_matrix(filenameH,H,DIAGNOSTICS);

  vector <double> Dd(H.size(),0.0);
  for(unsigned int i=0;i<H.size();++i)
    Dd[i]=H[i][i];

  //  cout << Dd ;

  //calculate mean
  vector<double> Emean;
  if(!E.empty())
    for(unsigned int i=0;i<dim;++i)
      {
	double S=0;
	for(unsigned int j=0;j<E.size();++j)
	  S+=E[j][i];
	Emean.push_back(S/(E.size()+0.0));
      }

  //compute distance from mean
  matrix_type__ Distance;
  if(!E.empty())
    for(unsigned int i=0;i<E.size();++i)
	Distance[UTIL__::norm(vector <double>
			      (E[i].begin(),
			       E[i].begin()+dim),
			      Emean)].insert(i);;
  
  if(TOP>0)
    {
      matrix_type__ Distance_tmp;
      matrix_type__::iterator itrEND = Distance.begin();
      for(unsigned int i=0;i<TOP;++i)
	itrEND++;
      
      std::copy(Distance.begin(),itrEND,
		std::inserter(Distance_tmp,
			      Distance_tmp.end()));
      Distance=Distance_tmp;
    }
  
  //pretty print
  if(VERBOSE)
    for(map<double, set<unsigned int> >::iterator itr
	  =Distance.begin();
	itr!=Distance.end();
	++itr)
      cout<< itr->first << " " << itr->second << endl;

  ofstream OUT(outfile.c_str());
  OUT << "% " << filenameE << endl;
  OUT<< "#CENTROID" << endl <<  Emean << endl ;
  OUT<< "#TOP" << endl << TOP << endl << "%(Zero based count)" << endl;
  OUT << "#LIBFILES" << endl;
  for(map<double, set<unsigned int> >::iterator itr
	=Distance.begin();
      itr!=Distance.end();
      ++itr)
    for(set<unsigned int>::iterator itr1=itr->second.begin();
	itr1!=itr->second.end();++itr1)
      OUT<< UTIL__::fconstx(PATHPREF,WIDTH,(*itr1) + 1)
	 << endl;
  OUT <<"#LIBDISTANCES" << endl;
  for(map<double, set<unsigned int> >::iterator itr
	=Distance.begin();
      itr!=Distance.end();
      ++itr)
    for(set<unsigned int>::iterator itr1=itr->second.begin();
	itr1!=itr->second.end();++itr1)
      OUT<< itr->first << " "
	 << UTIL__::fconstx(PATHPREF,WIDTH,(*itr1) + 1)
	 << endl;
  OUT.close();
  
  return 0;
}
//-------------------------------------------
//-------------------------------------------
