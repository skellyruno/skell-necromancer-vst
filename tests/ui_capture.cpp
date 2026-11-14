// Reference UI capture. Renders the REAL JUCE editor (no mockup) at the
// three review sizes and checks whether runes change pixel positions in time.
#include <JuceHeader.h>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

static ReanimateDial* findDial(juce::Component& root) {
 if(auto* d=dynamic_cast<ReanimateDial*>(&root))return d;
 for(int i=0;i<root.getNumChildComponents();++i)
  if(auto* child=root.getChildComponent(i))
   if(auto* found=findDial(*child))return found;
 return nullptr;
}

static void findMeters(juce::Component& root,std::vector<PeakMeter*>& meters) {
 if(auto* m=dynamic_cast<PeakMeter*>(&root))meters.push_back(m);
 for(int i=0;i<root.getNumChildComponents();++i)
  if(auto* child=root.getChildComponent(i))findMeters(*child,meters);
}

static bool saveImage(const juce::Image& image,const juce::File& path) {
 if(image.isNull())return false;
 juce::FileOutputStream stream(path);
 if(!stream.openedOk())return false;
 juce::PNGImageFormat png;
 return png.writeImageToStream(image,stream);
}

int main(int argc,char** argv) {
 juce::ScopedJuceInitialiser_GUI init;
 const auto outputDir=argc>1?juce::File::getCurrentWorkingDirectory().getChildFile(argv[1]):
                                juce::File::getCurrentWorkingDirectory().getChildFile("ui-captures");
 if(outputDir.createDirectory().failed()){std::cerr<<"Could not create output directory\n";return 1;}
 SkellNecromancerAudioProcessor processor;
 processor.setPlayConfigDetails(2,2,48000.,512);
 processor.prepareToPlay(48000.,512);
 auto editor=std::make_unique<SkellNecromancerAudioProcessorEditor>(processor);
 editor->setVisible(true);
 // VISUAL TEST SIGNAL, NOT PROCESSED AUDIO: match the reference meter levels
 // while leaving the real processor, meter math and production defaults intact.
 processor.inputL.store(-.4f);processor.inputR.store(-.7f);
 processor.outputL.store(-.1f);processor.outputR.store(-.3f);
 std::vector<PeakMeter*>meters;
 findMeters(*editor,meters);
 if(meters.size()!=2){std::cerr<<"Expected both real meter components\\n";return 8;}
 for(auto* meter:meters)meter->tick();
 std::cout<<"UI test levels: IN -0.4/-0.7 dB, OUT -0.1/-0.3 dB (synthetic preview only)\\n";

 auto snapshot=[&](int width,int height,const juce::String& name) {
  editor->setSize(width,height);
  const auto img=editor->createComponentSnapshot(editor->getLocalBounds());
  if(img.getWidth()!=width||img.getHeight()!=height||
     !saveImage(img,outputDir.getChildFile(name))){
   std::cerr<<"Failed capture "<<name<<" ("<<img.getWidth()<<"x"<<img.getHeight()<<")\n";
   return juce::Image();
  }
  std::cout<<"Captured "<<name<<"\n";
  return img;
 };
 if(snapshot(810,428,"ui_810.png").isNull())return 2;
 const auto full=snapshot(1157,612,"ui_1157.png");
 if(full.isNull())return 3;
 // The approved black skull engraving must be visible against real silver
 // metal in the central disk; also catches accidental solid-silver overlays.
 int centerDark=0,centerSilver=0;
 for(int y=-10;y<=10;++y)for(int x=-10;x<=10;++x){
  if(x*x+y*y>=100)continue;
  const auto p=full.getPixelAt(578+x,319+y);
  const int luma=((int)p.getRed()+(int)p.getGreen()+(int)p.getBlue())/3;
  if(luma<80)++centerDark;
  if(luma>130)++centerSilver;
 }
 std::cout<<"Center cap dark/silver detail: "<<centerDark<<" / "<<centerSilver<<" pixels\\n";
 if(centerDark<95||centerSilver<12){
  std::cerr<<"Approved engraved center cap was hidden or blank\\n";return 11;
 }
 // Detect real lit LED pixels, not just value updates. Reference checks use
 // synthetic levels ONLY in this non-shipping capture program.
 auto litCount=[&](int x0){
  int total=0;
  for(int y=105;y<235;++y)
   for(int x=x0;x<x0+41;++x){
    const auto p=full.getPixelAt(x,y);
    if(p.getRed()>125 && p.getBlue()>125 && p.getGreen()>85)++total;
   }
  return total;
 };
 const int inLit=litCount(100),outLit=litCount(1016);
 std::cout<<"Active LED pixels IN="<<inLit<<" OUT="<<outLit<<"\\n";
 if(inLit<45||outLit<45){std::cerr<<"Active stereo meters are not legible in the captured UI\\n";return 10;}
 if(snapshot(1736,918,"ui_1736.png").isNull())return 4;

 editor->setSize(1157,612);
 auto* dial=findDial(*editor);
 if(dial==nullptr){std::cerr<<"REANIMATE dial not found\n";return 5;}
 dial->setActive(true);
 dial->setLive(0.f); // movement must persist even during silence
 dial->advance();
 const auto before=editor->createComponentSnapshot(editor->getLocalBounds());
 juce::Thread::sleep(125);
 dial->advance();
 const auto after=editor->createComponentSnapshot(editor->getLocalBounds());
 if(!saveImage(before,outputDir.getChildFile("rune_before.png"))||
    !saveImage(after,outputDir.getChildFile("rune_after.png")))return 6;
 int changed=0,considered=0;
 for(int y=125;y<514;y+=2)for(int x=383;x<774;x+=2){
  const float dx=(float)x-578.f,dy=(float)y-319.f,r2=dx*dx+dy*dy;
  if(r2<148.f*148.f||r2>195.f*195.f)continue;
  ++considered;
  const auto a=before.getPixelAt(x,y),b=after.getPixelAt(x,y);
  const int diff=std::abs((int)a.getRed()-(int)b.getRed())+
                 std::abs((int)a.getGreen()-(int)b.getGreen())+
                 std::abs((int)a.getBlue()-(int)b.getBlue());
  if(diff>24)++changed;
 }
 std::cout<<"Moving rune pixels: "<<changed<<" / "<<considered<<" annulus probes\n";
 if(changed<80){std::cerr<<"Rune artwork did not visibly move\n";return 7;}
 // Measure screenshot rendering work separately from audio/DSP processing.
 // Informational only: snapshot time includes allocation/OS rasterization and
 // is NOT a host CPU benchmark.
 double totalMs=0.,maximumMs=0.;
 for(int n=0;n<6;++n){
  dial->advance();
  const double t0=juce::Time::getMillisecondCounterHiRes();
  auto image=editor->createComponentSnapshot(editor->getLocalBounds());
  const double elapsed=juce::Time::getMillisecondCounterHiRes()-t0;
  if(image.isNull())return 9;
  totalMs+=elapsed;maximumMs=juce::jmax(maximumMs,elapsed);
 }
 std::cout<<"UI screenshot render mean "<<totalMs/6.<<" ms, worst "<<maximumMs
          <<" ms (headless capture; NOT host CPU)\\n";
 processor.releaseResources();
 return 0;
}
