#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#include <exception>
#include <boost/timer/timer.hpp>
#include <boost/filesystem.hpp>

#include "procx.h"

using namespace boost::filesystem;
using namespace boost::program_options;


const string VERSION="post_proc v1.0 \n Copyright Ishanu Chattopadhyay 2016";
const string EMPTY_ARG_MESSAGE="Exiting. Type -h or --help for usage";

int main(int argc,char* argv[])
{
  string patt="";
  string path_="./";
  set<string> ss;
  int count_=1;
  string ofile="";
  
  options_description infor( "Program information");
  infor.add_options()
    ("help,h", "print help message.")
    ("version,V", "print version number");

  options_description usg( "Usage");
  usg.add_options()
    ("pathtodir,P",value<string>(), "path to directory [./]")
    ("pattern,p",value<string>(), "pattern match []")
    ("nummatch,n",value<int>(), "max number of lines chosen [-1]");


  options_description outputopt( "Output options");
  outputopt.add_options()
    ("output,o",value< string>(), "output file [output.txt]");

  options_description desc("Post proc. sel. ");
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

  if (vm.count("output"))
    ofile=vm["output"].as<string>();
  if (vm.count("pathtodir"))
    path_=vm["pathtodir"].as<string>();
  if (vm.count("pattern"))
    patt=vm["pattern"].as<string>();
  if (vm.count("nummatch"))
    count_=vm["nummatch"].as<int>();

  
  path pth(path_.c_str());
  for (auto i = directory_iterator(pth); i != directory_iterator(); i++)
    if (!is_directory(i->path())) //we eliminate directories
      {
	string str_= i->path().filename().string();
	std::size_t found = str_.find(patt);
	if (found!=std::string::npos)
	  ss.insert(str_);
      }
    else
      continue;

  _selection_ SL(path_,count_);
  set<string> ss__;
  
  for(set<string>::iterator itr=ss.begin();
      itr!=ss.end();
      ++itr)
    SL.get(*itr, ss__);
  
  if(ofile=="")
    cout << ss__ ;
  else
    {
      std::ofstream OUT(ofile.c_str());
      OUT << ss__;
      OUT.close();
    }
  
  return 0;
}
