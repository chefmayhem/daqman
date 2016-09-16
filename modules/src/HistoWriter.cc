#include "HistoWriter.hh"
#include "ConvertData.hh"
#include "ConfigHandler.hh"
#include "CommandSwitchFunctions.hh"
#include "intarray.hh"
HistoWriter::HistoWriter() : 
  ChannelModule(GetDefaultName(),
		"Write waveforms as histograms into a root file") 
{
  AddDependency<ConvertData>();
  RegisterParameter("filename", _filename = GetDefaultFilename(),
		    "Name of the output root file");
  ConfigHandler::GetInstance()->
    AddCommandSwitch('f',"filename", "Set output root file",
		     CommandSwitch::DefaultRead<std::string>(_filename), 
		     "file");
}

HistoWriter::~HistoWriter()
{
  
}

int HistoWriter::Initialize()
{
  _wrote_header = false;
  _initialized_histo = false; // we can't initialize until we know the data length
  if(_filename.find(".root") == std::string::npos)
    _filename.append(".root");
  Message(INFO)<<"Saving root waveforms to file "<<_filename<<std::endl;
  _fout = new TFile(_filename.c_str(), "RECREATE", "histo waveforms", 1);
//  _fout.open(_filename.c_str());
  
  if(!_fout->IsOpen()){
    Message(CRITICAL)<<"Unable to open file "<<_filename<<std::endl;
    return 1;
  }

  // And initialize the tree too!
  _tree = new TTree("ht", "Tree of histograms (for now)");
  std::cout << "The tree should be printed here....\n";
  _tree->Print();
  _tree->GetName();
  
  return 0;
}

int HistoWriter::Finalize(){
  if(_fout->IsOpen()){
    _tree->Write();
    _fout->Write();
    _fout->Close();
  }
  return 0;
}

int HistoWriter::Process(ChannelData* chdata)
{
  if(!_wrote_header){
    //assume only one channel, or all identical channels
    std::cout<<chdata->nsamps<<"\t"<<chdata->sample_rate<<"\t"
	 <<chdata->trigger_index<<std::endl;
    std::cout << "I should be writing the header!\n";
    std::cout << chdata->channel_num << "  " << chdata->channel_id << "\n";
    _wrote_header = true;
  }
  if (!_initialized_histo){
    // Actually, let's not just initialize the histo, but set the branch address too!
    _histoch0 = new TH1S("histoch0", "Ch0 waveform; time #mu s; counts", //...
      chdata->nsamps, -1* chdata->trigger_index/chdata->sample_rate, //...
      (chdata->nsamps-chdata->trigger_index)/chdata->sample_rate);
    _histoch1 = new TH1S("histoch1", "Ch1 waveform; time #mu s; counts", //...
      chdata->nsamps, -1* chdata->trigger_index/chdata->sample_rate, //...
      (chdata->nsamps-chdata->trigger_index)/chdata->sample_rate);
    _tree->Branch("histoch0", "TH1S", &_histoch0, 32000, 0);
    _tree->Branch("histoch1", "TH1S", &_histoch1, 32000, 0);

    _initialized_histo = true;
  } // end histo initialization

  // Here's the stuff we do for each waveform
  TH1S * current_histo;
  if (chdata->channel_num == 0) {
    current_histo = _histoch0;
  }
  else {
    current_histo = _histoch1;
  }
  current_histo->Reset("ICE");

  double* wave;
  if (!_subtract_baseline_histo) {
    wave = chdata->GetWaveform();
  }
  else {
    //std::cout << "bl? " << chdata->baseline.found_baseline << chdata->baseline.saturated << "\n";
    if (chdata->baseline.found_baseline == 1){
      wave = chdata->GetBaselineSubtractedWaveform();
    }
    else {
      wave = chdata->GetWaveform();
      Message(INFO) << "No baseline found :-(\n";
    }
  }
  for(int samp = 0; samp < chdata->nsamps; samp++){
    current_histo->SetBinContent(samp+1, wave[samp]);
  }

  if (chdata->channel_num != 0) { // only do after filling second channel
    _tree->Fill();
  }
  return 0;
}
