#pragma once
#include "../PluginProcessor.h"
#include <memory>
class PresetBar:public juce::Component{
public:
 explicit PresetBar(SkellNecromancerAudioProcessor&p):processor(p) {
    name.setText(processor.uiState.getProperty("preset","INIT").toString(),juce::dontSendNotification);
    name.setEditable(true,true,false);name.setColour(juce::Label::textColourId,Theme::bone);
    name.setColour(juce::Label::backgroundColourId,Theme::black);name.setFont(Theme::mono(13));
    name.setColour(juce::Label::outlineColourId,Theme::border);
    name.onTextChange=[this]{processor.uiState.setProperty("preset",name.getText(),nullptr);};addAndMakeVisible(name);
    for(auto*b:{&drop,&undo,&redo,&save,&abA,&abB,&copy})addAndMakeVisible(*b);
    drop.setButtonText("V");undo.setButtonText("UNDO");redo.setButtonText("REDO");save.setButtonText("SAVE");
    abA.setButtonText("A");abB.setButtonText("B");copy.setButtonText("A>B");
    drop.onClick=[this]{openPresetMenu();};
    undo.onClick=[this]{processor.undoManager.undo();};redo.onClick=[this]{processor.undoManager.redo();};
    save.onClick=[this]{juce::String error;if(!processor.writeUserPreset(name.getText(),error))juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Preset save",error);};
    abA.onClick=[this]{processor.selectAB(0);};abB.onClick=[this]{processor.selectAB(1);};
    copy.onClick=[this]{processor.copyAB(processor.selectedSlot,1-processor.selectedSlot);};
    // This panel is a plain Component, not a SettableTooltipClient.
 }
 void resized()override {
  name.setBounds(0,0,233,27);drop.setBounds(234,0,25,27);
  undo.setBounds(267,0,46,27);redo.setBounds(316,0,46,27);save.setBounds(365,0,48,27);
  abA.setBounds(418,0,23,27);abB.setBounds(444,0,23,27);copy.setBounds(470,0,50,27);
 }
 void paint(juce::Graphics&g)override{if(processor.presetDirty.load()){g.setColour(Theme::green);g.fillEllipse(226.f,8.f,6.f,6.f);}}
 void sync(){repaint();juce::String p=processor.uiState.getProperty("preset","INIT").toString();if(!name.isBeingEdited()&&name.getText()!=p)name.setText(p,juce::dontSendNotification);
   abA.setToggleState(processor.selectedSlot==0,juce::dontSendNotification);abB.setToggleState(processor.selectedSlot==1,juce::dontSendNotification);}
private:
 void openPresetMenu(){
  juce::PopupMenu m;int i=1;for(auto& p:processor.factoryPresetNames())m.addItem(i++,p);
  m.addSeparator();m.addSectionHeader("USER PRESETS");
  auto dir=processor.presetDirectory();auto files=dir.findChildFiles(juce::File::findFiles,false,"*.xml");
  for(int n=0;n<files.size();++n)m.addItem(100+n,files[n].getFileNameWithoutExtension());
  m.addSeparator();m.addItem(1000,"Import preset XML...");m.addItem(1001,"Export preset XML...");
  auto safe=juce::Component::SafePointer<PresetBar>(this);
  m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&drop),[safe,files](int choice)mutable{
   if(safe==nullptr||choice==0)return;
   if(choice>=1&&choice<=safe->processor.factoryPresetNames().size())safe->processor.applyFactoryPreset(safe->processor.factoryPresetNames()[choice-1]);
   else if(choice>=100&&choice<100+files.size()){juce::String err;safe->processor.loadUserPreset(files[choice-100],err);}
   else if(choice==1000)safe->chooseFile(false);
   else if(choice==1001)safe->chooseFile(true);
  });
 }
 void chooseFile(bool exportIt){
  chooser=std::make_unique<juce::FileChooser>(exportIt?"Export Necromancer preset":"Import Necromancer preset",processor.presetDirectory().getChildFile(name.getText()+".xml"),"*.xml");
  auto safe=juce::Component::SafePointer<PresetBar>(this);
  chooser->launchAsync(exportIt?juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles:
                      juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
   [safe,exportIt](const juce::FileChooser& fc){if(safe==nullptr)return;const auto f=fc.getResult();if(f==juce::File())return;
    juce::String error;
    if(exportIt){
       juce::XmlElement xml("NECRO_PRESET");
       xml.setAttribute("name",safe->name.getText());
       if(auto state=safe->processor.apvts.copyState().createXml()) xml.addChildElement(state.release());
       if(!xml.writeTo(f))error="Could not write XML";
    }else safe->processor.loadUserPreset(f,error);
    if(error.isNotEmpty())juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Preset",error);
   });
 }
 SkellNecromancerAudioProcessor&processor;juce::Label name;
 juce::TextButton drop,undo,redo,save,abA,abB,copy;
 std::unique_ptr<juce::FileChooser>chooser;
};
