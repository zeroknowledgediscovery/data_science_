#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <stdlib.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_histogram.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_cdf.h>
#include <cmath>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#include <exception>


using namespace std;
using namespace boost::program_options;

const string VERSION="\nCONFIDENCE BND FOR TS v0.131415 \nCopyright Ishanu Chattopadhyay 2018 UChicago";
const string EMPTY_ARG_MESSAGE="Exiting. Type -h or --help for usage";


vector<option> ignore_numbers(vector<string>& args)
{
  vector<option> result;
  int pos = 0;
  while(!args.empty())
    {
      const auto& arg = args[0];
      double num;
      if(boost::conversion::try_lexical_convert(arg, num))
	{
	  result.push_back(option());
	  option& opt = result.back();

	  opt.position_key = pos++;
	  opt.value.push_back(arg);
	  opt.original_tokens.push_back(arg);

	  args.erase(args.begin());
	}
      else
	break;
    }

  return result;
}
//-------------------------------------
ostream& operator << (ostream &out, vector <double> &s)
{
  for (unsigned int i=0; i<s.size(); i++)
    out << s[i] << " ";
  return out;
}
//-----------------------------------------

int main(int argc,char* argv[])
{
  string datafile="",outfile="";
  double alpha=0.9;
  unsigned int N=1000;
  unsigned int n=10;
  bool VERBOSE=false;

  options_description infor( "Program information");
  infor.add_options()
    ("help,h", "print help message.")
    ("version,V", "print version number");

  options_description usg( "Usage");
  usg.add_options()
    ("datafile,f",value<string>(&datafile), "datafile []")
    ("numeach,N",value<unsigned int>(&N), "number of histogram draws [1000]")
    ("nubins,n",value<unsigned int>(&n), "number of histogram bins [10]")
    ("alpha,a",value<double>(&alpha), "alpha [.9]");
  
  options_description outputopt( "Output options");
  outputopt.add_options()
    ("outfile,o",value<string>(&outfile), " []")
    ("verbose,v",value< bool >(&VERBOSE), "verbosity  [off] ");

  options_description desc( "\n### CNFBD ### (ishanu chattopadhyay 2018)");
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
	    .extra_style_parser(&ignore_numbers)
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

  double zscore=gsl_cdf_ugaussian_Pinv (alpha);

  //data series is in columns,
  // with each row is the set of samples at each time point
  

  string line;
  ifstream IN(datafile.c_str());
  map<unsigned int,vector<double>> H;
  unsigned int count=0;
  
  while(getline(IN,line))
    {
      stringstream ss(line);
      double token;
      vector<double> dvec;
      while(ss>>token)
	dvec.push_back(token);
      H[count++]=dvec;
    }
  IN.close();

  
  gsl_rng_env_setup ();
  const gsl_rng_type * T;
  gsl_rng * r;
  T = gsl_rng_default;
  r = gsl_rng_alloc (T);
  
  for(map<unsigned int,vector<double>>::iterator itr=H.begin();
      itr!=H.end();
      ++itr)
    {
      vector<double> data=itr->second;
      gsl_sort (&data[0], 1, data.size());

      gsl_histogram * h = gsl_histogram_alloc (n);
      gsl_histogram_set_ranges_uniform (h, data[0], data.back());
      for(unsigned int i=0;i<data.size();++i)
        gsl_histogram_increment (h, data[i]);
      

      gsl_histogram_pdf * p = gsl_histogram_pdf_alloc (h->n);
    
      gsl_histogram_pdf_init (p, h);

      for(unsigned int i=0;i<N;++i)
	data.push_back(gsl_histogram_pdf_sample (p,gsl_rng_uniform (r)));

      gsl_histogram_pdf_free(p);
      gsl_histogram_free(h);
      
      gsl_sort (&data[0], 1, data.size());
      /*      double uq = gsl_stats_quantile_from_sorted_data (&data[0], 
						       1, data.size(),
						       alpha);
      double lq = gsl_stats_quantile_from_sorted_data (&data[0], 
						       1, data.size(),
						       1-alpha);
      */
      double mean=gsl_stats_mean(&data[0],1,data.size());
      double std=gsl_stats_sd(&data[0],1,data.size());
      double clt=zscore*(std/sqrt(data.size()+0.0));

      if(outfile=="")
	cout << mean-clt << " "
	     << mean << " " <<  mean +clt
	     <<  endl;
      else
	{
	  ofstream out(outfile.c_str());
	  out << mean-clt << " "
	      << mean << " " <<  mean +clt
	     <<  endl;
	  out.close();
	}
    }
  return 0;
}
