/** @file HistoWriter.hh
    @brief Addition to AsciiWriter module to simply write root histos to a root file
    @author josh albert
    @ingroup modules
*/

#ifndef HISTO_WRITER_h
#define HISTO_WRITER_h

#include "ChannelModule.hh"
#include <fstream>
#include "TH1.h"
#include "TFile.h"
#include "TTree.h"

/** @class HistoWriter 
    @brief module which dumps the waveform of a channel to a root histogram and saves the file
    @ingroup modules
*/
class HistoWriter : public ChannelModule{
public:
  HistoWriter();
  ~HistoWriter();
  
  int Initialize();
  int Finalize();
  int Process(ChannelData* chdata);
  
  static const std::string GetDefaultName(){ return "HistoWriter"; }
  
  /// Get the default name of the output text file
  std::string GetDefaultFilename(){ return "out.txt"; }
  /// Get the name of the output text file being used
  const std::string& GetFilename(){ return _filename; }
  /// Set the name of the output text file
  void SetFilename(const std::string& s){ _filename = s; }
  void SetSubtractBaseline(bool sbh = true){_subtract_baseline_histo = sbh;}
private:
  TFile * _fout;     ///< root file to write to
  TTree * _tree;    ///< root tree which will hold the histograms
  TH1S  * _histoch0;   ///< root histo which we will write repeatedly
  TH1S  * _histoch1;   ///< root histo which we will write repeatedly
  std::string _filename;   ///< name of the output file
  bool _wrote_header;      ///< has the header been written yet?
  bool _initialized_histo;      ///< has the histo been initialized yet?
  bool _subtract_baseline_histo; ///< subtract baselines from our histos
};

#endif
