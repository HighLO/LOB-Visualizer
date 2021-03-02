#include "../../include/Enums.h"
#include "../../include/Security.h"
#include "../../include/TimeNS.h"
#include "../../include/Windowing.h"

#include <TGraph.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>

#include <list>
#include <string>
#include <memory>
#include <algorithm>

// Maximum number of bins, limited by memory. If the required number of bins exceeds this number, automatic subsampling is applied
constexpr int MAXBINS = 10000000;

// A struct containing all the different histograms which are recorded
struct LOBPlotConfig
{
   void setup(const MetaData_t& metaData, int skip, const std::string& title, long numberOfBinsWindowHist, long numberOfMessages, TimeNS snapshotSize, int i, int yBinMargin)
   {
      index = i;

      contractID = MetaDataGetID(metaData, contract);
      if(contractID == -1) throw std::runtime_error("ID not found");

      const int lowTicks = low - yBinMargin;
      const float lowHist = lowTicks * metaData.at(contractID).PriceIncrease; // - 0.5 * metaData.at(contractID).PriceIncrease;
      const int highTicks = high + yBinMargin + 1;
      const float highHist = highTicks * metaData.at(contractID).PriceIncrease; // - 0.5 * metaData.at(contractID).PriceIncrease;

      // Initiate histograms for windowed plot
      histWindowLob = std::make_unique<TH2F>(("histWindowLob" + std::to_string(index)).c_str(), (title + ";;" + yAxisTitle).c_str(),
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second,
         high - low + yBinMargin + yBinMargin + 1, lowHist, highHist);

      histWindowTrade = std::make_unique<TH1F>(("histWindowTrade" + std::to_string(index)).c_str(), ";Time (seconds);Trade Volume",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);
      histWindowCumulTrade = std::make_unique<TH1F>(("histWindowCumulTrade" + std::to_string(index)).c_str(), ";Time (seconds);#splitline{Cumul. Trade}{    Volume}",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);
      histWindowCumulTradeBid = std::make_unique<TH1F>(("histWindowCumulTradeBid" + std::to_string(index)).c_str(), ";Time (seconds);#splitline{     Cumul. Sell}{Aggressor Volume}",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);
      histWindowCumulTradeAsk = std::make_unique<TH1F>(("histWindowCumulTradeAsk" + std::to_string(index)).c_str(), ";Time (seconds);#splitline{     Cumul. Buy}{Aggressor Volume}",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);
       histWindowPrice = std::make_unique<TH1F>(("histWindowPrice" + std::to_string(index)).c_str(), ";Time (seconds);Price (points)",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);

      histWindowTime = std::make_unique<TH1F>(("histWindowTime" + std::to_string(index)).c_str(), ";Time (seconds);#splitline{Messages}{per snapshot}",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);

      histWindowBidVolume = std::make_unique<TH1F>(("histWindowBidVolume" + std::to_string(index)).c_str(), "",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);
      histWindowAskVolume = std::make_unique<TH1F>(("histWindowAskVolume" + std::to_string(index)).c_str(), "",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);

      histWindowCancellationsBid = std::make_unique<TH1F>(("histWindowCancellationsBid" + std::to_string(index)).c_str(), ";;#splitline{Cumul. Bid Level 1}{   Cancellations}",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);
      histWindowCancellationsAsk = std::make_unique<TH1F>(("histWindowCancellationsAsk" + std::to_string(index)).c_str(), ";;#splitline{Cumul. Ask Level 1}{   Cancellations}",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);

      histWindowLevel1VolumeBid = std::make_unique<TH1F>(("histWindowLevel1VolumeBid" + std::to_string(index)).c_str(), ";;#splitline{Bid Level 1}{  Volume}",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);
      histWindowLevel1VolumeAsk = std::make_unique<TH1F>(("histWindowLevel1VolumeAsk" + std::to_string(index)).c_str(), ";;#splitline{Ask Level 1}{  Volume}",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);

      histWindowAPMBid = std::make_unique<TH1F>(("histWindowAPMBid" + std::to_string(index)).c_str(), ";;APM Bid",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);
      histWindowAPMAsk = std::make_unique<TH1F>(("histWindowAPMAsk" + std::to_string(index)).c_str(), ";;APM Ask",
         numberOfBinsWindowHist, 0, numberOfBinsWindowHist * snapshotSize / T_Second);

      spreadWindowMarker = std::make_unique<TGraph>();

      // Initiate historgrams for message Plot
      histMessageLob = std::make_unique<TH2F>(("histMessageLob" + std::to_string(index)).c_str(), (title + ";;" + yAxisTitle).c_str(),
         numberOfMessages / skip, 0, numberOfMessages,
         high - low + yBinMargin + yBinMargin + 1, lowHist, highHist);

      histMessageTrade = std::make_unique<TH1F>(("histMessageTrade" + std::to_string(index)).c_str(), ";;Trade Volume",
         numberOfMessages / skip, 0, numberOfMessages);
      histMessageCumulTrade = std::make_unique<TH1F>(("histMessageCumulTrade" + std::to_string(index)).c_str(), ";;#splitline{Cumul. Trade}{    Volume}",
         numberOfMessages / skip, 0, numberOfMessages);
      histMessageCumulTradeBid = std::make_unique<TH1F>(("histMessageCumulTradeBid" + std::to_string(index)).c_str(), ";;#splitline{     Cumul. Sell}{Aggressor Volume}",
         numberOfMessages / skip, 0, numberOfMessages);
      histMessageCumulTradeAsk = std::make_unique<TH1F>(("histMessageCumulTradeAsk" + std::to_string(index)).c_str(), ";;#splitline{     Cumul. Buy}{Aggressor Volume}",
         numberOfMessages / skip, 0, numberOfMessages);
      histMessagePrice = std::make_unique<TH1F>(("histMessagePrice" + std::to_string(index)).c_str(), ";;Price (points)",
         numberOfMessages / skip, 0, numberOfMessages);

      histMessageTime = std::make_unique<TH1F>(("histMessageTime" + std::to_string(index)).c_str(), ";Message number;Messages per snapshot",
         numberOfMessages / skip, 0, numberOfMessages);

      histMessageBidVolume = std::make_unique<TH1F>(("histMessageBidVolume" + std::to_string(index)).c_str(), "",
         numberOfMessages / skip, 0, numberOfMessages);
      histMessageAskVolume = std::make_unique<TH1F>(("histMessageAskVolume" + std::to_string(index)).c_str(), "",
         numberOfMessages / skip, 0, numberOfMessages);

      histMessageCancellationsBid = std::make_unique<TH1F>(("histMessageCancellationsBid" + std::to_string(index)).c_str(), ";;#splitline{Cumul. Bid Level 1}{   Cancellations}",
         numberOfMessages / skip, 0, numberOfMessages);
      histMessageCancellationsAsk = std::make_unique<TH1F>(("histMessageCancellationsAsk" + std::to_string(index)).c_str(), ";;#splitline{Cumul. Ask Level 1}{   Cancellations}",
         numberOfMessages / skip, 0, numberOfMessages);

      histMessageLevel1VolumeBid = std::make_unique<TH1F>(("histMessageLevel1VolumeBid" + std::to_string(index)).c_str(), ";;#splitline{Bid Level 1}{  Volume}",
         numberOfMessages / skip, 0, numberOfMessages);
      histMessageLevel1VolumeAsk = std::make_unique<TH1F>(("histMessageLevel1VolumeAsk" + std::to_string(index)).c_str(), ";;#splitline{Ask Level 1}{  Volume}",
         numberOfMessages / skip, 0, numberOfMessages);

      histMessageAPMBid = std::make_unique<TH1F>(("histMessageAPMBid" + std::to_string(index)).c_str(), ";;APM Bid",
         numberOfMessages / skip, 0, numberOfMessages);
      histMessageAPMAsk = std::make_unique<TH1F>(("histMessageAPMAsk" + std::to_string(index)).c_str(), ";;APM Ask",
         numberOfMessages / skip, 0, numberOfMessages);

      spreadMessageMarker = std::make_unique<TGraph>();
   }

   void save(TFile& file)
   {
      histWindowLob->SetMaximum(maxVolume);
      histMessageLob->SetMaximum(maxVolume);

      TNamed contractName(("contractName" + std::to_string(index)).c_str(), contract);
      file.WriteObject(&contractName, contractName.GetName());

      file.WriteObject(histWindowLob.get(), histWindowLob->GetName());

      file.WriteObject(histWindowTrade.get(), histWindowTrade->GetName());
      file.WriteObject(histWindowCumulTrade.get(), histWindowCumulTrade->GetName());
      file.WriteObject(histWindowCumulTradeBid.get(), histWindowCumulTradeBid->GetName());
      file.WriteObject(histWindowCumulTradeAsk.get(), histWindowCumulTradeAsk->GetName());
      file.WriteObject(histWindowPrice.get(), histWindowPrice->GetName());

      file.WriteObject(histWindowTime.get(), histWindowTime->GetName());

      file.WriteObject(histWindowBidVolume.get(), histWindowBidVolume->GetName());
      file.WriteObject(histWindowAskVolume.get(), histWindowAskVolume->GetName());

      file.WriteObject(histWindowCancellationsBid.get(), histWindowCancellationsBid->GetName());
      file.WriteObject(histWindowCancellationsAsk.get(), histWindowCancellationsAsk->GetName());

      file.WriteObject(histWindowLevel1VolumeBid.get(), histWindowLevel1VolumeBid->GetName());
      file.WriteObject(histWindowLevel1VolumeAsk.get(), histWindowLevel1VolumeAsk->GetName());

      file.WriteObject(histWindowAPMBid.get(), histWindowAPMBid->GetName());
      file.WriteObject(histWindowAPMAsk.get(), histWindowAPMAsk->GetName());

      file.WriteObject(spreadWindowMarker.get(), ("spreadWindowMarker" + std::to_string(index)).c_str());

      file.WriteObject(&windowTrades, "windowTrades");


      file.WriteObject(histMessageLob.get(), histMessageLob->GetName());
      
      file.WriteObject(histMessageTrade.get(), histMessageTrade->GetName());
      file.WriteObject(histMessageCumulTrade.get(), histMessageCumulTrade->GetName());
      file.WriteObject(histMessageCumulTradeBid.get(), histMessageCumulTradeBid->GetName());
      file.WriteObject(histMessageCumulTradeAsk.get(), histMessageCumulTradeAsk->GetName());
      file.WriteObject(histMessagePrice.get(), histMessagePrice->GetName());

      file.WriteObject(histMessageTime.get(), histMessageTime->GetName());

      file.WriteObject(histMessageBidVolume.get(), histMessageBidVolume->GetName());
      file.WriteObject(histMessageAskVolume.get(), histMessageAskVolume->GetName());

      file.WriteObject(histMessageCancellationsBid.get(), histMessageCancellationsBid->GetName());
      file.WriteObject(histMessageCancellationsAsk.get(), histMessageCancellationsAsk->GetName());

      file.WriteObject(histMessageLevel1VolumeBid.get(), histMessageLevel1VolumeBid->GetName());
      file.WriteObject(histMessageLevel1VolumeAsk.get(), histMessageLevel1VolumeAsk->GetName());

      file.WriteObject(histMessageAPMBid.get(), histMessageAPMBid->GetName());
      file.WriteObject(histMessageAPMAsk.get(), histMessageAPMAsk->GetName());

      file.WriteObject(spreadMessageMarker.get(), ("spreadMessageMarker" + std::to_string(index)).c_str());

      file.WriteObject(&messageTrades, "messageTrades");

      histWindowLob.reset();

      histWindowTrade.reset();
      histWindowCumulTrade.reset();
      histWindowCumulTradeBid.reset();
      histWindowCumulTradeAsk.reset();

      histWindowTime.reset();

      histWindowBidVolume.reset();
      histWindowAskVolume.reset();

      spreadWindowMarker.reset();


      histMessageLob.reset();

      histMessageTrade.reset();
      histMessageCumulTrade.reset();
      histMessageCumulTradeBid.reset();
      histMessageCumulTradeAsk.reset();

      histMessageTime.reset();

      histMessageBidVolume.reset();
      histMessageAskVolume.reset();

      spreadMessageMarker.reset();
   }

   void addSpreadMarkerWindow(double x, double y)
   {
      if(spreadWindowFirst)
      {
         spreadWindowFirst = false;
         spreadWindowMarker->SetPoint(spreadWindowMarkerNumber, x, y);
         spreadWindowMarkerNumber++;
         spreadWindowLast = y;
      }
      else
      {
         spreadWindowMarker->SetPoint(spreadWindowMarkerNumber, x, spreadWindowLast);
         spreadWindowMarkerNumber++;
         spreadWindowMarker->SetPoint(spreadWindowMarkerNumber, x, y);
         spreadWindowMarkerNumber++;
         spreadWindowLast = y;
      }
   }

   void addSpreadMarkerMessage(double x, double y)
   {
      if(spreadMessageFirst)
      {
         spreadMessageFirst = false;
         spreadMessageMarker->SetPoint(spreadMessageMarkerNumber, x, y);
         spreadMessageMarkerNumber++;
         spreadMessageLast = y;
      }
      else
      {
         spreadMessageMarker->SetPoint(spreadMessageMarkerNumber, x, spreadMessageLast);
         spreadMessageMarkerNumber++;
         spreadMessageMarker->SetPoint(spreadMessageMarkerNumber, x, y);
         spreadMessageMarkerNumber++;
         spreadMessageLast = y;
      }
   }

   int index = 1;
   int low = 99999;
   int high = 0;
   int maxVolume= 0 ;
   long messages = 0;
   int contractID = -1;
   double dollarValue = 0;

   std::string fileName;
   std::string contract;
   std::string yAxisTitle;

   long tradeVolumeSinceLastSnapshot = 0;
   long totalTradeVolume = 0;
   long tradeVolumeSinceLastMessage = 0;
   long bidTradeVolume = 0;
   long askTradeVolume = 0;
   long unexplainedTradeVolume = 0;
   long numberOfMessagesSinceLastSnapshot = 0;
   long numberOfMessagesSinceStart = 0;
   long bidCancellations = 0;
   long askCancellations = 0;

   bool spreadWindowFirst = true;
   double spreadWindowLast = 0.0;
   long spreadWindowMarkerNumber = 0;

   bool spreadMessageFirst = true;
   double spreadMessageLast = 0.0;
   long spreadMessageMarkerNumber = 0;

   std::unique_ptr<TH2F> histWindowLob;

   std::unique_ptr<TH1F> histWindowTrade;
   std::unique_ptr<TH1F> histWindowCumulTrade;
   std::unique_ptr<TH1F> histWindowCumulTradeBid;
   std::unique_ptr<TH1F> histWindowCumulTradeAsk;
   std::unique_ptr<TH1F> histWindowPrice;

   std::unique_ptr<TH1F> histWindowTime;

   std::unique_ptr<TH1F> histWindowBidVolume;
   std::unique_ptr<TH1F> histWindowAskVolume;

   std::unique_ptr<TH1F> histWindowCancellationsBid;
   std::unique_ptr<TH1F> histWindowCancellationsAsk;

   std::unique_ptr<TH1F> histWindowLevel1VolumeBid;
   std::unique_ptr<TH1F> histWindowLevel1VolumeAsk;

   std::unique_ptr<TH1F> histWindowAPMBid;
   std::unique_ptr<TH1F> histWindowAPMAsk;

   std::unique_ptr<TGraph> spreadWindowMarker;

   std::vector<double> windowTrades;


   std::unique_ptr<TH2F> histMessageLob;

   std::unique_ptr<TH1F> histMessageTrade;
   std::unique_ptr<TH1F> histMessageCumulTrade;
   std::unique_ptr<TH1F> histMessageCumulTradeBid;
   std::unique_ptr<TH1F> histMessageCumulTradeAsk;
   std::unique_ptr<TH1F> histMessagePrice;

   std::unique_ptr<TH1F> histMessageTime;

   std::unique_ptr<TH1F> histMessageBidVolume;
   std::unique_ptr<TH1F> histMessageAskVolume;

   std::unique_ptr<TH1F> histMessageCancellationsBid;
   std::unique_ptr<TH1F> histMessageCancellationsAsk;

   std::unique_ptr<TH1F> histMessageLevel1VolumeBid;
   std::unique_ptr<TH1F> histMessageLevel1VolumeAsk;

   std::unique_ptr<TH1F> histMessageAPMBid;
   std::unique_ptr<TH1F> histMessageAPMAsk;

   std::unique_ptr<TGraph> spreadMessageMarker;

   std::vector<double> messageTrades;
};

// Function to calculate some of the required parameters, runs before the main loop. Should not be called by user.
void getPeriodStats(std::vector<LOBPlotConfig>& configs, TimeNS beginTime, TimeNS endTime, const std::string &rootPath, bool cutMissing)
{
   std::set<std::string> fileNames;
   std::set<int> ids;

   for(auto& config : configs)
   {
      fileNames.insert(config.fileName);
   }

   std::vector<std::unique_ptr<TFile>> files;
   Windower<> windower;
   MetaData_t metaData;

   for(auto& fileName : fileNames)
   {
      std::string filePath = rootPath + "/" + fileName;
      files.push_back(std::make_unique<TFile>(filePath.c_str()));
      if (!files.back()) throw std::invalid_argument("Could not open " + filePath);

      ReadMetaData(*files.back(), metaData);
      windower.addTree(files.back().get(), "Messages");
   }

   for(auto& config : configs)
   {
      config.contractID = MetaDataGetID(metaData, config.contract);
      if(config.contractID == -1) throw std::runtime_error("ID not found");
      ids.insert(config.contractID);
   }

   windower.setIdFilter(ids);
   windower.setDefaultStateInitializerAndUpdater(&metaData);

   if(cutMissing)
   {
      for(auto& config : configs)
      {
         std::swap(config.low, config.high);
      }
   }

   windower.setForEachRow([&](int id, TimeNS time, const MRow& row, const Security& security)
   {
      if (beginTime <= time && time <= endTime)
      {
         if(row.messageKind >= (char)MessageKind::BidNew
            && row.messageKind <= (char)MessageKind::AskDelete)
         {
            for(auto& config : configs)
            {
               if(config.contractID == id)
               {
                  config.messages++;

                  int localLow = security.getBook(BookSide::BidConsolidated)->back().price;
                  int localHigh = security.getBook(BookSide::AskConsolidated)->back().price;

                  if(!cutMissing)
                  {
                     if(localLow != 0 && localLow < config.low)
                     {
                        config.low = localLow;
                     }

                     if(localHigh != 0 && localHigh > config.high)
                     {
                        config.high = localHigh;
                     }
                  }
                  else
                  {
                     if(localLow != 0 && localLow > config.low)
                     {
                        config.low = localLow;
                     }

                     if(localHigh != 0 && localHigh < config.high)
                     {
                        config.high = localHigh;
                     }
                  }

                  for(auto level : *security.getBook(BookSide::BidConsolidated))
                  {
                     if(level.volume > config.maxVolume)
                     {
                        config.maxVolume = level.volume;
                     }
                  }
                  for(auto level : *security.getBook(BookSide::AskConsolidated))
                  {
                     if(level.volume > config.maxVolume)
                     {
                        config.maxVolume = level.volume;
                     }
                  }
               }
            }
         }
      }
   });

   windower.run();
}

// Main function collecting the plot data
// Parameters:
//    rootPath: the path to the input ROOT file
//    outputFileName: the path to the output ROOT file
//    beginTime: start date and time of plot
//    endTime: end date and time of plot
//    title: title of plot
//    snapshotSize: snapshot size of the plot
//    configs: a list of different configurations to be made, each element is an object as defined above
//    verticalLines: time and name of the vertical lines to be overlayed
//    cutMissing: boolean to control the min and max price of the plot
void GenerateLiveLOBPlot(const std::string &rootPath,
   const std::string& outputFileName,
   const TimeNS beginTime, const TimeNS endTime, 
   const std::string& title, 
   const TimeNS snapshotSize, 
   std::vector<LOBPlotConfig>& configs,
   std::vector<std::pair<TimeNS, std::string>> verticalLines,
   bool cutMissing)
{
   // Calculate parameters based on the configuration
   if(beginTime % snapshotSize != 0)
   {
      std::cout << "Begin time is alligned with the snapshot series, undefined behaviour!\n";
   }
   if(endTime % snapshotSize != 0)
   {
      std::cout << "End time is alligned with the snapshot series, undefined behaviour!\n";
   }

   const long numberOfBinsWindowHist = (endTime - beginTime) / snapshotSize;

   // Gather the minimum and maximum price within the specified window
   getPeriodStats(configs, beginTime, endTime, rootPath, cutMissing);

   // Continue calculating parameters 
   long numberOfMessages = 0;
   int skip = 1;
   int maxVerticalRange = 1;
   int maxVolume = 0;
   for(auto& config : configs)
   {
      numberOfMessages += config.messages;
      if(config.high - config.low > maxVerticalRange)
      {
         maxVerticalRange = config.high - config.low;
      }
      if(config.maxVolume > maxVolume)
      {
         maxVolume = config.maxVolume;
      }
   }

   int numberOfMessagesCopy = numberOfMessages;
   int verticalNumberOfBins = maxVerticalRange + 10;
   while(numberOfMessagesCopy * verticalNumberOfBins > MAXBINS)
   {
      numberOfMessagesCopy /= 10;
      skip *= 10;
   }

   std::cout << "Number of messages: " << numberOfMessages 
      << ", number of snapshots: " << numberOfBinsWindowHist 
      << ", skip interval: " << skip
      << ", number of horizontal time bins: " <<  numberOfMessages / skip
      << ", max vertical range: " << maxVerticalRange 
      << ", max volume: " << maxVolume << "\n";

   std::set<std::string> fileNames;
   std::set<int> ids;

   for(auto& config : configs)
   {
      fileNames.insert(config.fileName);
   }

   std::vector<std::unique_ptr<TFile>> files;
   Windower<> windower;
   MetaData_t metaData;

   for(auto& fileName : fileNames)
   {
      std::cout << "Unique file: " << fileName << "\n";

      std::string filePath = rootPath + "/" + fileName;
      files.push_back(std::make_unique<TFile>(filePath.c_str()));
      if (!files.back()) throw std::invalid_argument("Could not open " + filePath);

      ReadMetaData(*files.back(), metaData);
      windower.addTree(files.back().get(), "Messages");
   }

   int index = 1;
   auto titleCopy = title;
   int yBinMargin = cutMissing ? 0 : 3;
   for(auto& config : configs)
   {
      std::cout << config.contract << "=" << config.contractID
         << ", low (ticks): " << config.low
         << ", high (ticks): " << config.high
         << std::setprecision(5)
         << ", low: " << config.low * metaData[config.contractID].PriceIncrease
         << ", high: " << config.high * metaData[config.contractID].PriceIncrease
         << ", tick size: " << metaData[config.contractID].PriceIncrease
         << ", messages: " << config.messages
         << ", dollar value:" << config.dollarValue
         << "\n";

      config.setup(metaData, skip, titleCopy, numberOfBinsWindowHist, numberOfMessages, snapshotSize, index, yBinMargin);
      titleCopy = "";
      index++;
      ids.insert(config.contractID);
   }

   // Construct the histogram and other objects
   auto histMessageCumulTime = std::make_unique<TH1F>("histMessageCumulTime", ";Message number since start of plot;#splitline{Seconds since}{  start of plot}", numberOfMessages / skip, 0, numberOfMessages);
   auto histMessageTimeRatio = std::make_unique<TH1F>("histMessageTimeRatio", ";Message number since start of plot;", numberOfMessages / skip, 0, numberOfMessages);

   windower.setIdFilter(ids);
   windower.setDefaultStateInitializerAndUpdater(&metaData);

   long long currentMessageNumber = 0;
   long long currentWindowNumber = 0;
   long snapshotStartMessage = 0;

   std::vector<double> messagePlotSnapshotPoints;
   std::vector<double> verticalLinesWindow;
   std::vector<double> verticalLinesMessage;
   std::vector<std::string> verticalLinesTitle;
   int verticalLineIndex = 0;

   for(auto vl : verticalLines)
   {
      verticalLinesWindow.push_back(static_cast<double>(vl.first - beginTime) / T_Second);
      verticalLinesTitle.push_back(vl.second);
   }

   // Apply for each snapshot --> snapshot based plot
   windower.setStateWindowAction(snapshotSize, [&](TimeNS time, const std::map<int, Security>& securities)
   {
      if (beginTime <= time && time <= endTime)
      {
         messagePlotSnapshotPoints.push_back(currentMessageNumber);

         for(auto& config : configs)
         {
            auto FillLevel = [&](std::unique_ptr<TH2F>& hist, int id, BookSide side)
            {
               auto book = securities.at(id).getBook(side);
               for (auto &&level : *book)
               {
                  if (level.price > 0)
                  {
                     //hist->Fill((double) currentWindowNumber * snapshotSize / T_Second,
                     //   level.price * metaData[id].PriceIncrease,
                     //   level.volume);
                     hist->SetBinContent(currentWindowNumber + 1,
                              level.price - config.low + yBinMargin + 1,
                              level.volume);
                  }
               }
            };

            FillLevel(config.histWindowLob, config.contractID, BookSide::BidConsolidated);
            FillLevel(config.histWindowLob, config.contractID, BookSide::AskConsolidated);

            config.histWindowTrade->SetBinContent(currentWindowNumber + 1, config.tradeVolumeSinceLastSnapshot);
            config.histWindowCumulTrade->SetBinContent(currentWindowNumber + 1, config.totalTradeVolume);
            config.histWindowCumulTradeBid->SetBinContent(currentWindowNumber + 1, config.bidTradeVolume);
            config.histWindowCumulTradeAsk->SetBinContent(currentWindowNumber + 1, config.askTradeVolume);
            config.histWindowPrice->SetBinContent(currentWindowNumber + 1, securities.at(config.contractID).getPrice() * metaData[config.contractID].PriceIncrease);

            if(!cutMissing)
            {
               config.histWindowBidVolume->SetBinContent(currentWindowNumber + 1, securities.at(config.contractID).getVolume(BookSide::BidConsolidated));
               config.histWindowAskVolume->SetBinContent(currentWindowNumber + 1, securities.at(config.contractID).getVolume(BookSide::AskConsolidated));
            }
            else
            {
               config.histWindowBidVolume->SetBinContent(currentWindowNumber + 1, securities.at(config.contractID).getVolume(BookSide::BidConsolidated, config.low));
               config.histWindowAskVolume->SetBinContent(currentWindowNumber + 1, securities.at(config.contractID).getVolume(BookSide::AskConsolidated, config.high));
            }

            config.histWindowCancellationsBid->SetBinContent(currentWindowNumber + 1, config.bidCancellations);
            config.histWindowCancellationsAsk->SetBinContent(currentWindowNumber + 1, config.askCancellations);

            if(securities.at(config.contractID).getBook(BookSide::BidConsolidated)->size() >= 1)
            {
               config.histWindowLevel1VolumeBid->SetBinContent(currentWindowNumber + 1, securities.at(config.contractID).getBook(BookSide::BidConsolidated)->at(0).volume);
            }
            if(securities.at(config.contractID).getBook(BookSide::AskConsolidated)->size() >= 1)
            {
               config.histWindowLevel1VolumeAsk->SetBinContent(currentWindowNumber + 1, securities.at(config.contractID).getBook(BookSide::AskConsolidated)->at(0).volume);
            }

            bool saturated = false;
            config.histWindowAPMBid->SetBinContent(currentWindowNumber + 1, securities.at(config.contractID).getAPM(BookSide::BidConsolidated, config.dollarValue / metaData[config.contractID].PriceIncrease, saturated));
            config.histWindowAPMAsk->SetBinContent(currentWindowNumber + 1, securities.at(config.contractID).getAPM(BookSide::AskConsolidated, config.dollarValue / metaData[config.contractID].PriceIncrease, saturated));

            config.addSpreadMarkerWindow(static_cast<double>(currentWindowNumber * snapshotSize) / T_Second,
               (securities.at(config.contractID).getMidPoint(Book::Consolidated) + 0.5) * metaData[config.contractID].PriceIncrease);
         }

         for(auto& config : configs)
         {
            config.histWindowTime->SetBinContent(currentWindowNumber + 1, config.numberOfMessagesSinceLastSnapshot);

            for(long i = snapshotStartMessage; i < currentMessageNumber; i++)
            {
               if(i % skip == 0)
               {
                  config.histMessageTrade->SetBinContent(1 + i / skip, config.tradeVolumeSinceLastSnapshot);
                  config.histMessageTime->SetBinContent(1 + i / skip, config.numberOfMessagesSinceLastSnapshot);
               }
            }

            config.tradeVolumeSinceLastSnapshot = 0;
            config.numberOfMessagesSinceLastSnapshot = 0;
         }

         currentWindowNumber++;
         snapshotStartMessage = currentMessageNumber;
      }
   });

   // Apply for each row (each message) --> message based plot
   windower.setForEachRow([&](int id, TimeNS time, const MRow& row, const std::map<int, Security>& securities)
   {
      if (beginTime <= time && time <= endTime)
      {
         if(verticalLineIndex < verticalLines.size())
         {
            if(verticalLines.at(verticalLineIndex).first - time <= 0)
            {
               verticalLinesMessage.push_back(currentMessageNumber);
               verticalLineIndex++;
            }
         }
         if(row.messageKind >= (char)MessageKind::BidNew
            && row.messageKind <= (char)MessageKind::AskDelete)
         {
            for(auto& config : configs)
            {
               auto actions = securities.at(id).getLastUpdateActions();
               for(auto a : *actions)
               {
                  if(a.actionType == ActionType::DeleteAction)
                  {
                     if(a.side == Side::Bid)
                     {
                        if(a.price >= config.low && a.level == 1)
                        {
                           config.bidCancellations += a.volume;
                        }
                     }
                     else
                     {
                        if(a.price <= config.high && a.level == 1)
                        {
                           config.askCancellations += a.volume;
                        }
                     }
                  }
               }

               if(currentMessageNumber % skip == 0)
               {
                  auto FillLevel = [&](std::unique_ptr<TH2F>& hist, int id, BookSide side)
                  {
                     auto book = securities.at(id).getBook(side);
                     for (auto &&level : *book)
                     {
                        if (level.price > 0)
                        {
                           hist->SetBinContent(1 + currentMessageNumber / skip,
                              level.price - config.low + yBinMargin + 1,
                              level.volume);
                        }
                     }
                  };

                  FillLevel(config.histMessageLob, config.contractID, BookSide::BidConsolidated);
                  FillLevel(config.histMessageLob, config.contractID, BookSide::AskConsolidated);

                  config.tradeVolumeSinceLastMessage = 0;
                  config.histMessageCumulTrade->SetBinContent(1 + currentMessageNumber / skip, config.totalTradeVolume);
                  config.histMessageCumulTradeBid->SetBinContent(1 + currentMessageNumber / skip, config.bidTradeVolume);
                  config.histMessageCumulTradeAsk->SetBinContent(1 + currentMessageNumber / skip, config.askTradeVolume);
                  config.histMessagePrice->SetBinContent(1 + currentMessageNumber / skip, securities.at(config.contractID).getPrice() * metaData[config.contractID].PriceIncrease);

                  if(!cutMissing)
                  {
                     config.histMessageBidVolume->SetBinContent(1 + currentMessageNumber / skip, securities.at(config.contractID).getVolume(BookSide::BidConsolidated));
                     config.histMessageAskVolume->SetBinContent(1 + currentMessageNumber / skip, securities.at(config.contractID).getVolume(BookSide::AskConsolidated));
                  }
                  else
                  {
                     config.histMessageBidVolume->SetBinContent(1 + currentMessageNumber / skip, securities.at(config.contractID).getVolume(BookSide::BidConsolidated, config.low));
                     config.histMessageAskVolume->SetBinContent(1 + currentMessageNumber / skip, securities.at(config.contractID).getVolume(BookSide::AskConsolidated, config.high));
                  }

                  config.histMessageCancellationsBid->SetBinContent(1 + currentMessageNumber / skip, config.bidCancellations);
                  config.histMessageCancellationsAsk->SetBinContent(1 + currentMessageNumber / skip, config.askCancellations);

                  if(securities.at(config.contractID).getBook(BookSide::BidConsolidated)->size() >= 1)
                  {
                     config.histMessageLevel1VolumeBid->SetBinContent(1 + currentMessageNumber / skip, securities.at(config.contractID).getBook(BookSide::BidConsolidated)->at(0).volume);
                  }
                  if(securities.at(config.contractID).getBook(BookSide::AskConsolidated)->size() >= 1)
                  {
                     config.histMessageLevel1VolumeAsk->SetBinContent(1 + currentMessageNumber / skip, securities.at(config.contractID).getBook(BookSide::AskConsolidated)->at(0).volume);
                  }

                  bool saturated = false;
                  config.histMessageAPMBid->SetBinContent(1 + currentMessageNumber / skip, securities.at(config.contractID).getAPM(BookSide::BidConsolidated, config.dollarValue / metaData[config.contractID].PriceIncrease, saturated));
                  config.histMessageAPMAsk->SetBinContent(1 + currentMessageNumber / skip, securities.at(config.contractID).getAPM(BookSide::AskConsolidated, config.dollarValue / metaData[config.contractID].PriceIncrease, saturated));

                  config.addSpreadMarkerMessage(currentMessageNumber,
                     (securities.at(config.contractID).getMidPoint(Book::Consolidated) + 0.5) * metaData[config.contractID].PriceIncrease);
               }

               if(id == config.contractID)
               {
                  config.numberOfMessagesSinceLastSnapshot++;
                  config.numberOfMessagesSinceStart++;
               }
            }

            currentMessageNumber++;
            if(currentMessageNumber % skip == 0)
            {
               histMessageCumulTime->SetBinContent(1 + currentMessageNumber / skip, double(time-beginTime) / T_Second);
            }

            if(configs.size() == 2)
            {
               //double ratio = (configs[0].numberOfMessagesSinceStart / (double)currentMessageNumber) - (configs[1].numberOfMessagesSinceStart / (double)currentMessageNumber);
               double ratio = (configs[0].numberOfMessagesSinceStart / (double)configs[0].messages) - (configs[1].numberOfMessagesSinceStart / (double)configs[1].messages);
               histMessageTimeRatio->SetBinContent(1 + currentMessageNumber / skip, ratio * 10.0 + 1);
            }
         }
         else if (row.messageKind == static_cast<char>(MessageKind::Trade)
            && row.quoteCondition == static_cast<char>(QuoteCondition::Trade))
         {
            for(auto& config : configs)
            {
               if(id == config.contractID)
               {
                  config.totalTradeVolume += row.quantity;
                  config.tradeVolumeSinceLastMessage += row.quantity;
                  config.tradeVolumeSinceLastSnapshot += row.quantity;

                  auto bidBook = securities.at(id).getBook(BookSide::BidConsolidated);
                  auto askBook = securities.at(id).getBook(BookSide::AskConsolidated);

                  if(bidBook->size() >= 1 && bidBook->at(0).price == row.price) // Short-circuit evaluation
                  {
                     config.bidTradeVolume += row.quantity;
                  } 
                  else if(askBook->size() >= 1 && askBook->at(0).price == row.price) // Short-circuit evaluation
                  {
                     config.askTradeVolume += row.quantity;
                  }
                  else
                  {
                     config.unexplainedTradeVolume += row.quantity;
                  }

                  config.messageTrades.push_back(static_cast<double>(currentMessageNumber) / skip);
                  config.windowTrades.push_back(currentWindowNumber + 1);
               }
            }
         }
      }
   });

   // Build the plot
   windower.run();

   // Display some post building statistics
   std::cout << "Window Plot: " << currentWindowNumber << " horizontal bins required. (" << numberOfBinsWindowHist << ")\n";
   std::cout << "Message Plot: " << currentMessageNumber << " horizontal bins required. (" << numberOfMessages / skip << ")\n";

   for(auto& config : configs)
   {
      std::cout << config.index << ": Buy trades = " << config.askTradeVolume 
         << ", Sell trades = " << config.bidTradeVolume
         << ", Unmatched trades = " << config.unexplainedTradeVolume << "\n"; 
   }

   // Write everything to a ROOT file
   TFile outputFile(outputFileName.c_str(), "recreate");

   for(auto& config : configs)
   {
      config.save(outputFile);
   }

   outputFile.WriteObject(histMessageCumulTime.get(), histMessageCumulTime->GetName());
   outputFile.WriteObject(histMessageTimeRatio.get(), histMessageTimeRatio->GetName());

   outputFile.WriteObject(&messagePlotSnapshotPoints, "messagePlotSnapshotPoints"); 
   outputFile.WriteObject(&verticalLinesWindow, "verticalLinesWindow");
   outputFile.WriteObject(&verticalLinesMessage, "verticalLinesMessage");
   outputFile.WriteObject(&verticalLinesTitle, "verticalLinesTitle");

   outputFile.Write();
   outputFile.Close();
}
