
// Struct to contain the parameters of a single sub plot
struct PlotData
{
   PlotData(float h, std::string d)
   {
      height = h;
      dataLeft = d;
   }

   float height;

   std::string legendLocation = "upper left";

   std::string dataLeft;
   std::string legendLeft;
   bool isLOB = false;
   std::string dataSpreadMaker;
   bool drawDots = false;
   std::string dataDots;
   std::string legendDots;
   std::string legendEventLines;

   bool overlay = false;
   std::string dataRight;
   std::string titleRight;

   bool addSnapshotPoints = false;
   std::string legendSnapshotPoints;

   std::string yAxisTitle;

   bool forceYAxis = false;
   float tickSize;
   int padding = 2;
};

// Struct to contain the parameters of the full plot
struct GeneralData
{
   std::string type;
   std::string xAxisTitle;

   bool drawEventLines = false;
   std::string dataEventLines;
};

// Main function to call to output plot to screen and save to file
// Parameters:
//    fileNameIn: the path to the root file generate by the GenerateLiveLOBPlot
//    fileNameOut: the path the output png file
//    generalData: a object with all general parameters, as defined above
//    plotData: a list of objecs, each element in the list contains the parameters of a subplot, as defined above
void drawLOB(const std::string& fileNameIn, 
   const std::string& fileNameOut, 
   const GeneralData& generalData, 
   const std::vector<PlotData>& plotData)
{
   // Read all the data from the ROOT file
   TFile *file = TFile::Open(fileNameIn.c_str());

   std::vector<double>* verticalLines = nullptr;
   std::vector<std::string>* verticalLinesTitle = nullptr;
   if(generalData.type.find("window") == 0 || generalData.type.find("time") == 0 || generalData.type.find("sec") == 0)
   {
      file->GetObject("verticalLinesWindow", verticalLines);
      file->GetObject("verticalLinesTitle", verticalLinesTitle);
   }
   if(generalData.type.find("mes") == 0)
   {
      file->GetObject("verticalLinesMessage", verticalLines);
      file->GetObject("verticalLinesTitle", verticalLinesTitle);
   }

   std::vector<double>* eventLines = nullptr;
   if(generalData.drawEventLines)
   {
      file->GetObject(generalData.dataEventLines.c_str(), eventLines);
   }

   // Generate the canvas
   gStyle->SetOptStat(0);
   gStyle->SetTitleY(1.05);
   gStyle->SetTitleFontSize(0.15);

   auto c = new TCanvas("c", "c", 1280, 720);
   c->SetFillStyle(4000);
   c->SetFrameFillStyle(4000);  
   c->Draw();

   float totalHeight = 0;
   for(const auto& plot : plotData)
   {
      totalHeight += plot.height;
   }

   // Calculate the height of each panel based on the total number of subplots and relative size
   std::vector<TPad*> pads;
   std::vector<TLegend*> legends;
   float currentHeight = 0.1;
   for(int i = 0; i < plotData.size(); i++)
   {
      c->cd();

      auto padName = std::string("pad") + std::to_string(i);
      pads.push_back(new TPad(padName.c_str(), padName.c_str(), 0.0, 0.0, 1.0, 1.0));

      std::cout << "Pad " << i << " top " << currentHeight << " bottom " << 1.0 - currentHeight - (plotData.at(i).height / totalHeight) * 0.8 << "\n";
      pads.back()->SetTopMargin(currentHeight);
      pads.back()->SetLeftMargin(0.1);
      pads.back()->SetRightMargin(0.1);
      pads.back()->SetBottomMargin(1.0 - currentHeight - (plotData.at(i).height / totalHeight) * 0.8);
      pads.back()->SetFrameFillStyle(4000);
      pads.back()->SetFillStyle(4000);
      pads.back()->Draw();

      pads.back()->cd();

      if(plotData.at(i).legendLocation == "upper left")
      {
         legends.push_back(new TLegend(0.1, 1 - (currentHeight + (plotData.at(i).height / totalHeight) * 0.8 * 0.4), 0.3, 1 - currentHeight));
      }
      else
      {
         legends.push_back(new TLegend(0.77, 1 - (currentHeight + (plotData.at(i).height / totalHeight) * 0.8 * 0.4), 0.9, 1 - currentHeight));
      }

      legends.back()->SetBorderSize(0);
      legends.back()->SetFillStyle(0);

      currentHeight += (plotData.at(i).height / totalHeight) * 0.8;
   }

   // Fill in the subplots
   for(int i = 0; i < plotData.size(); i++)
   {
      pads[i]->cd();

      TAxis *h2ax = nullptr;

      if(plotData.at(i).isLOB)
      {
         pads[i]->SetGrid(0, 1);

         TH2* hist = nullptr;
         file->GetObject(plotData.at(i).dataLeft.c_str(), hist);

         TGaxis::SetExponentOffset(-0.04, -0.04, "y");

         hist->SetFillColorAlpha(0, 0);
         hist->SetStats(false);
         hist->SetMinimum(0.0);
         hist->Draw("COLZ");

         pads[i]->Update();

         h2ax = hist->GetXaxis();
         if(i != plotData.size() - 1)
         {
            hist->GetXaxis()->SetLabelSize(0);
            hist->GetXaxis()->SetLabelOffset(999);
            hist->GetXaxis()->SetTickLength(0.0);
            hist->GetXaxis()->SetNdivisions(505);
         }

         hist->GetYaxis()->SetTitleFont(43);
         hist->GetYaxis()->SetTitleSize(15);
         hist->GetYaxis()->CenterTitle();
         hist->GetYaxis()->SetTitleOffset(1.5);
         hist->GetYaxis()->SetLabelFont(43);
         hist->GetYaxis()->SetLabelSize(12);
         hist->GetYaxis()->SetNdivisions(-hist->GetNbinsY());
         hist->GetYaxis()->SetMaxDigits(4);
         hist->GetYaxis()->CenterLabels("M");
         if(plotData.at(i).yAxisTitle != "")
         {
            hist->GetYaxis()->SetTitle(plotData.at(i).yAxisTitle.c_str());
         }
   
         hist->GetZaxis()->SetTitle("Volume in LOB level");
         hist->GetZaxis()->SetTitleFont(43);
         hist->GetZaxis()->SetTitleSize(15);
         hist->GetZaxis()->CenterTitle();
         hist->GetZaxis()->SetTitleOffset(1);
         hist->GetZaxis()->SetLabelFont(43);
         hist->GetZaxis()->SetLabelSize(12);

         pads[i]->Update();

         TGraph* spreadMarker = nullptr;
         file->GetObject(plotData.at(i).dataSpreadMaker.c_str(), spreadMarker);

         spreadMarker->SetLineWidth(1);
         spreadMarker->SetLineColor(kRed);
         spreadMarker->SetFillColorAlpha(0, 0);
         spreadMarker->Draw("L SAME");

         pads[i]->Update();

         if(verticalLines)
         {
            for(int i = 0; i < verticalLines->size(); i++)
            {
               TLine *l = new TLine(verticalLines->at(i), hist->GetYaxis()->GetXmin(), verticalLines->at(i), hist->GetYaxis()->GetXmax());
               l->SetLineColor(kRed + 1);
               l->Draw();

               TText *t = new TText(verticalLines->at(i) - 0.004 * (hist->GetXaxis()->GetXmax() - hist->GetXaxis()->GetXmin()), hist->GetYaxis()->GetXmin() + 0.01 * (hist->GetYaxis()->GetXmax() - hist->GetYaxis()->GetXmin()), verticalLinesTitle->at(i).c_str());
               t->SetTextAlign(11);
               t->SetTextColor(kRed + 2);
               t->SetTextFont(43);
               t->SetTextSize(18);
               t->SetTextAngle(90);
               t->Draw();
            }
         }

         pads[i]->Update();
      }
      else
      {
         TH1* histLeft = nullptr;
         file->GetObject(plotData.at(i).dataLeft.c_str(), histLeft);

         if(!histLeft)
         {
            std::cout << "Hist not found: " << plotData.at(i).dataLeft << "\n";
         }

         if(plotData.at(i).forceYAxis)
         {
            float min = histLeft->GetMinimum();
            float max = histLeft->GetMaximum();
            int bins = std::round((max - min) / plotData.at(i).tickSize) + 2 * plotData.at(i).padding;

            auto background = new TH2F("Background", "",
               histLeft->GetNbinsX(), 0, histLeft->GetNbinsX(),
               bins, min - plotData.at(i).padding * plotData.at(i).tickSize, max + plotData.at(i).padding * plotData.at(i).tickSize);
            
            background->Draw("COLZ SAME");

            background->GetXaxis()->SetLabelSize(0);
            background->GetXaxis()->SetLabelOffset(999);
            background->GetXaxis()->SetTickLength(0.0);
            background->GetXaxis()->SetNdivisions(505);

            background->GetYaxis()->SetTitle(histLeft->GetYaxis()->GetTitle());
            background->GetYaxis()->SetTitleFont(43);
            background->GetYaxis()->SetTitleSize(15);
            background->GetYaxis()->CenterTitle();
            background->GetYaxis()->SetTitleOffset(1.5);
            background->GetYaxis()->SetLabelFont(43);
            background->GetYaxis()->SetLabelSize(12);
            background->GetYaxis()->SetNdivisions(-background->GetNbinsY());
            background->GetYaxis()->SetMaxDigits(4);
         }
         else
         {
            histLeft->SetFillColorAlpha(0, 0);
            histLeft->SetStats(false);
            histLeft->Draw("HIST AXIS");
         }

         pads[i]->Update();

         // gray background lines
         if(generalData.drawEventLines)
         {
            TLine* l;
            for(auto el : *eventLines)
            {
               l = new TLine(el, pads[i]->GetUymin(), el, pads[i]->GetUymax());
               l->SetLineColor(kGray + 1);
               l->Draw();
            }

            if(plotData.at(i).legendEventLines != "")
            {
               auto entry = legends[i]->AddEntry(l, plotData.at(i).legendEventLines.c_str(), "l");
               entry->SetTextFont(43);
               entry->SetTextSize(16);
            }

            pads[i]->Update();
         }

         // green snapshot lines
         if(plotData.at(i).addSnapshotPoints)
         {
            std::vector<double>* snapshotPoints;
            file->GetObject("messagePlotSnapshotPoints", snapshotPoints);

            TLine* l;
            for(int j = 1; j < snapshotPoints->size() - 1; j++) // Skip first and last
            {
               l = new TLine(snapshotPoints->at(j), pads[i]->GetUymin(), snapshotPoints->at(j), pads[i]->GetUymax());
               l->SetLineColor(kGreen + 2);
               l->Draw();
            }

            if(plotData.at(i).legendSnapshotPoints != "")
            {
               auto entry = legends[i]->AddEntry(l, plotData.at(i).legendSnapshotPoints.c_str(), "l");
               entry->SetTextFont(43);
               entry->SetTextSize(16);
            }

            pads[i]->Update();
         }

         // red foreground lines
         if(verticalLines)
         {
            for(auto vl : *verticalLines)
            {
               TLine *l = new TLine(vl, pads[i]->GetUymin(), vl, pads[i]->GetUymax());
               l->SetLineColor(kRed + 1);
               l->Draw();
            }

            pads[i]->Update();
         }

         histLeft->SetFillColorAlpha(0, 0);
         histLeft->SetStats(false);
         histLeft->Draw("HIST SAME");

         pads[i]->Update();

         h2ax = histLeft->GetXaxis();
         if(i != plotData.size() - 1)
         {
            histLeft->GetXaxis()->SetLabelSize(0);
            histLeft->GetXaxis()->SetLabelOffset(999);
            histLeft->GetXaxis()->SetTickLength(0.0);
            histLeft->GetXaxis()->SetNdivisions(505);
         }

         histLeft->GetYaxis()->SetLabelFont(43);
         histLeft->GetYaxis()->SetLabelSize(12);
         histLeft->GetYaxis()->SetNdivisions(505);
         histLeft->GetYaxis()->SetTitleFont(43);
         histLeft->GetYaxis()->SetTitleSize(15);
         histLeft->GetYaxis()->CenterTitle();
         histLeft->GetYaxis()->SetTitleOffset(1.5);
         if(plotData.at(i).yAxisTitle != "")
         {
            histLeft->GetYaxis()->SetTitle(plotData.at(i).yAxisTitle.c_str());
         }

         if(plotData.at(i).legendLeft != "")
         {
            auto entry = legends[i]->AddEntry(histLeft, plotData.at(i).legendLeft.c_str(), "l");
            entry->SetTextFont(43);
            entry->SetTextSize(16);
         }

         pads[i]->Update();

         if(plotData.at(i).drawDots)
         {
            std::vector<double>* dots = nullptr;
            file->GetObject(plotData.at(i).dataDots.c_str(), dots);
            
            TMarker *m;
            for(auto x : *dots)
            {
               double y = histLeft->GetBinContent(h2ax->FindBin(x));
               m = new TMarker(x, y, 4);
               m->SetMarkerSize(2);
               m->SetMarkerColor(kGreen + 2);
               m->Draw();
            }

            if(plotData.at(i).legendDots != "")
            {
               auto entry = legends[i]->AddEntry(m, plotData.at(i).legendDots.c_str(), "p");
               entry->SetTextFont(43);
               entry->SetTextSize(16);
            }

            pads[i]->Update();
         }

         if(plotData.at(i).overlay)
         {
            TH1* histRight = nullptr;
            file->GetObject(plotData.at(i).dataRight.c_str(), histRight);

            if(!histRight)
            {
               std::cout << "Hist not found: " << plotData.at(i).dataRight << "\n";
            }

            float max = histRight->GetMaximum() * 1.1;
            float scale = pads[i]->GetUymax() / max;

            histRight->Scale(scale);
            histRight->SetLineColor(kGreen + 2);
            histRight->SetFillColorAlpha(0, 0);
            histRight->SetStats(false);
            histRight->Draw("HIST SAME");

            pads[i]->Update();

            TGaxis *rightYAxis = new TGaxis(pads[i]->GetUxmax(), pads[i]->GetUymin(), pads[i]->GetUxmax(), pads[i]->GetUymax(), // Axis placement
                  0, max, // Axis limits
                  505,    // Number of ticks. 500 = 5 small ticks, 5 = 5 large ticks
                  "+L");  // +: Ticks on positive side  L: Labels left-adjusted
            rightYAxis->SetTitle(plotData.at(i).titleRight.c_str());
            rightYAxis->SetLabelFont(43);
            rightYAxis->SetLabelSize(12);
            rightYAxis->SetLabelOffset(0.02);
            rightYAxis->SetNdivisions(505);
            rightYAxis->SetTitleFont(43);
            rightYAxis->SetTitleSize(15);
            rightYAxis->CenterTitle();
            rightYAxis->SetTitleOffset(2.0);
            rightYAxis->Draw();

            pads[i]->Update();
         }
      }

      // For the last sub plot, add the x axis
      if(i == plotData.size() - 1)
      {
         h2ax->SetLabelFont(43);
         h2ax->SetLabelSize(12);
         h2ax->SetTitleFont(43);
         h2ax->SetTitleSize(18);
         h2ax->SetNdivisions(505);
         h2ax->CenterTitle();
         h2ax->SetTitleOffset(1.5);
         h2ax->SetTitle(generalData.xAxisTitle.c_str());
         h2ax->CenterTitle(true);

         pads[i]->Update();
      }

      legends[i]->Draw();

      pads[i]->Update();
   }

   c->SaveAs(fileNameOut.c_str());
}