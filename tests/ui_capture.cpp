// Reference UI capture. Renders the REAL JUCE editor (no mockup) at the
// three review sizes and checks whether runes change pixel positions in time.
#include <JuceHeader.h>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include <cmath>
#include <iostream>
#include <memory>

static ReanimateDial* findDial(juce::Component& root) {
 if(auto* d=dynamic_cast<ReanimateDial*>(&root))return d;
 for(int i=0;i<root.getNumChildComponents();++i)
  if(auto* child=root.getChildComponent(i))
   if(auto* found=findDial(*child))return found;
 return nullptr;
}

static bool saveImage(const juce::Image& image,const juce::File& path) {
 if(image.isNull())return false;
 juce::FileOutputStream stream(path);
 if(!stream.openedOk())return false;
 juce::PNGImageFormat png;
 return png.writeImageToStream(image,stream);
}

int main(int argc,char** argv) {
 juce::ScopedJuceInitialiser_GUI juce;
 const auto outputDir=argc>1?juce::File(argv[1]):
                                juce::File::getCurrentWorkingDirectory().getChildFile("ui-captures");
 if(outputDir.createDirectory().failed()){std::cerr<<"Could not create output directory\n";return 1;}
 SkellNecromancerAudioProcessor processor;
 processor.setPlayConfigDetails(2,2,48000.,512);
 processor.prepareToPlay(48000.,512);
 auto editor=std::make_unique<SkellNecromancerAudioProcessorEditor>(processor);
 editor->setVisible(true);

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
 if(snapshot(1157,612,"ui_1157.png").isNull())return 3;
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
 processor.releaseResources();
 return 0;
}
