/** @file histo_dump.cc
    @brief main file for histo_dump, which writes pulses to root histos
    @author jalbert
*/

#include "Reader.hh"
#include "ConfigHandler.hh"
#include "BaselineFinder.hh"
#include "CommandSwitchFunctions.hh"
#include "EventHandler.hh"
#include "HistoWriter.hh"
#include "ConvertData.hh"

#include <cstdlib>
#include <string>


void SetOutputFile(HistoWriter* writer, const char* inputfile){
  if( writer->GetFilename() == writer->GetDefaultFilename() ){
     //set the filename to be the input filename + .root
    std::string fname(inputfile);
    //remove a possible trailing slash in case of a directory argument
    while(fname.length()>0 && fname[fname.length()-1] =='/' ){
      fname.resize(fname.length()-1);
    }
    //remove leading directories
    while(fname.find('/') != std::string::npos){
      fname = fname.substr(fname.find('/')+1);
    }
    //remove filename suffix
    fname = fname.substr(0, fname.find('.'));
    //append the root filename
    fname.append(".root");
    //make sure we haven't made an empty string here!
    if(fname!=".root")
      writer->SetFilename(fname);
  }
}

int ProcessOneFile(const char* filename, int max_event=-1, int min_event=0)
{
  Message(INFO)<<"\n***************************************\n"
	       <<"  Processing File "<<filename
	       <<"\n***************************************\n";
  EventHandler* modules = EventHandler::GetInstance();
  Reader reader(filename);
  
  if(modules->Initialize()){
    Message(ERROR)<<"Unable to initialize all modules.\n";
    return 1;
  }
  
  //read through the file and process all events
  time_t start_time = time(0);
  int evtnum = min_event;
  while(reader.IsOk() && !reader.eof()){
    if(max_event > 0 && evtnum >= max_event) 
      break;
    Message(DEBUG)<<"*************Event "<<evtnum<<"**************\n";
    if(evtnum%5000 == 0)
      Message(INFO)<<"Processing event "<<evtnum<<std::endl;
    
    RawEventPtr raw = reader.GetEventWithIndex(evtnum++);
    if(!raw){
      Message(ERROR)<<"Problem encountered reading event "<<evtnum<<std::endl;
    }
    modules->Process(raw);
    
  }
  //finish up
  modules->Finalize();
  Message(INFO)<<"Processed "<<evtnum - min_event<<" events in "
	       <<time(0) - start_time<<" seconds. \n";
  return 0;
}

int main(int argc, char** argv)
{
  // by default, let's not subtract the baseline
  bool histo_subtract_baseline = false;
  int max_event=-1, min_event = 0;
  ConfigHandler* config = ConfigHandler::GetInstance();
  config->SetProgramUsageString("histo_dump [<options>] <rawdata> [<rawdata2>...]");
  config->AddCommandSwitch(' ',"max","last event to process",
			   CommandSwitch::DefaultRead<int>(max_event),
			   "event");
  config->AddCommandSwitch(' ',"min","first event to process",
			   CommandSwitch::DefaultRead<int>(min_event),
			   "event");
  config->AddCommandSwitch(' ',"subtract_baseline","Select if we subtract baseline",
			   CommandSwitch::SetValue<bool>(histo_subtract_baseline, true));
  
  EventHandler* modules = EventHandler::GetInstance();
  //modules->AddCommonModules();
  modules->AddModule<ConvertData>();
  modules->AddModule<BaselineFinder>();
  HistoWriter* writer = modules->AddModule<HistoWriter>();
  
  if(config->ProcessCommandLine(argc, argv))
    return -1;

  if(argc < 2){
    Message(ERROR)<<"Incorrect number of arguments: "<<argc<<std::endl;
    config->PrintSwitches(true);
  }
  
  for(int i = 1; i<argc; i++){
    if(i > 1)
      writer->SetFilename(writer->GetDefaultFilename());
      writer->SetSubtractBaseline(histo_subtract_baseline);
    SetOutputFile(writer, argv[i] );
    if(ProcessOneFile(argv[i], max_event, min_event)){
      Message(ERROR)<<"Error processing file "<<argv[i]<<"; aborting.\n";
      return 1;
    }
  }
  return 0;
}
