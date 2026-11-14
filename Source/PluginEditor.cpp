#include "PluginEditor.h"
#include "BinaryData.h"
#include <cmath>
using namespace juce;
SkellNecromancerAudioProcessorEditor::Panel::Panel(){
 bg=ImageCache::getFromMemory(BinaryData::bg_brushed_png,BinaryData::bg_brushed_pngSize);
 setBufferedToImage(true);
}
void SkellNecromancerAudioProcessorEditor::Panel::paint(Graphics&g){
 g.fillAll(Theme::black);
 g.drawImage(bg,getLocalBounds().toFloat());
 // The reference has a nearly black header and footer, NOT another bright metal
 // panel. Keep the high-contrast information on the brushed central instrument.
 g.setColour(juce::Colour(0xff090b0d));g.fillRect(0,35,Theme::width,45);
 g.setColour(juce::Colour(0xff101416));g.fillRect(0,577,Theme::width,35);
 g.setColour(juce::Colour(0xff31383d));g.drawLine(0.f,80.f,1157.f,80.f,1.f);
 g.setColour(juce::Colour(0xff363c40));g.drawLine(0.f,577.f,1157.f,577.f,1.f);
 // One continuous brushed steel panel: deterministic, clipped streaks add
 // directional texture without regenerating or modifying the original assets.
 g.setColour(juce::Colours::black.withAlpha(.19f));
 g.fillRoundedRectangle(10.f,87.f,1137.f,484.f,10.f);
 {
  juce::Graphics::ScopedSaveState textureState(g);
  g.reduceClipRegion(juce::Rectangle<int>(13,90,1131,478));
  juce::ColourGradient shade(juce::Colour(0xffb2bec4).withAlpha(.034f),
                             18.f,98.f,juce::Colours::black.withAlpha(.13f),
                             1128.f,566.f,false);
  g.setGradientFill(shade);
  g.fillRect(13,90,1131,478);
  juce::Random grain(0x51E110);
  for(int i=0;i<160;++i){
   const float y=91.f+grain.nextFloat()*473.f;
   const float x=14.f+grain.nextFloat()*170.f;
   const float len=610.f+grain.nextFloat()*850.f;
   const float opacity=.026f+grain.nextFloat()*.055f;
   g.setColour((i%5==0?juce::Colours::white:juce::Colours::black).withAlpha(opacity));
   g.drawLine(x,y,juce::jmin(1143.f,x+len),y,.5f+(i%7==0?.55f:0.f));
  }
  for(int i=0;i<62;++i){
   const float y=94.f+7.5f*i;
   g.setColour(juce::Colours::black.withAlpha(i%4==0?.055f:.021f));
   g.drawLine(13.f,y,1143.f,y,.5f);
  }
 }
 // Subtle bevel and edge lighting keep the hardware depth but avoid extra boxes.
 g.setColour(juce::Colour(0xff8a9298).withAlpha(.32f));
 g.drawLine(25.f,89.5f,1132.f,89.5f,1.f);
 g.setColour(juce::Colours::black.withAlpha(.65f));
 g.drawLine(24.f,568.5f,1133.f,568.5f,1.f);
 g.setColour(Theme::border.withAlpha(.82f));
 g.drawRoundedRectangle(10.5f,87.5f,1136.f,483.f,10.f,1.2f);
 // Original Skell skull-and-wings mark, deliberately not a copy of the reference WM logo.
 {
 Graphics::ScopedSaveState logoState(g);g.addTransform(AffineTransform::translation(12.f,0.f));
 Path skull;skull.addEllipse(24.f,45.f,22.f,21.f);skull.addRectangle(30.f,62.f,10.f,8.f);
 g.setColour(Theme::bone);g.strokePath(skull,PathStrokeType(1.7f));
 g.fillEllipse(29,54,3,3);g.fillEllipse(38,54,3,3);
 Path wing;wing.startNewSubPath(22,51);wing.lineTo(10,47);wing.lineTo(16,57);wing.lineTo(7,60);wing.lineTo(22,61);
 wing.startNewSubPath(47,51);wing.lineTo(60,47);wing.lineTo(54,57);wing.lineTo(63,60);wing.lineTo(47,61);
 g.strokePath(wing,PathStrokeType(2.f));
 }
 g.setFont(Theme::display(22).withExtraKerningFactor(.41f));
 g.setColour(Theme::bone);
 g.drawFittedText("SKELL  NECROMANCER",268,41,600,33,Justification::centred,1);
 g.setFont(Theme::mono(12));g.setColour(Theme::faint.brighter(1.1f));g.drawText("SKELL AUDIO",27,588,160,16,Justification::centredLeft);
 g.setFont(Theme::mono(11));g.setColour(Theme::faint.brighter(1.15f));
}
SkellNecromancerAudioProcessorEditor::SkellNecromancerAudioProcessorEditor(SkellNecromancerAudioProcessor&p):
 AudioProcessorEditor(&p),processor(p),top(p),inMeter("IN",p.inputL,p.inputR),outMeter("OUT",p.outputL,p.outputR,true),autopsy(p){
 setLookAndFeel(&look);addAndMakeVisible(panel);panel.setBounds(0,0,Theme::width,Theme::height);
 auto add=[this](Component&c){panel.addAndMakeVisible(c);};
 add(top);top.setBounds(0,0,Theme::width,36);
 add(dial);dial.setBounds(Theme::dial);attachSlider(dial,"reanimate");
 for(auto*knob:{&low,&mid,&high,&hf,&input,&output})add(*knob);
 low.setBounds(225,166,74,118);mid.setBounds(225,320,74,118);high.setBounds(837,166,74,118);hf.setBounds(837,320,74,118);
 input.setBounds(63,446,86,116);output.setBounds(987,446,86,116);
 low.setRange(-6,6,.1);low.setDefault(1.5);mid.setRange(-6,6,.1);mid.setDefault(-1);
 high.setRange(-6,6,.1);high.setDefault(2);hf.setRange(-6,6,.1);hf.setDefault(1);
 input.setRange(-12,12,.1);output.setRange(-12,12,.1);
 attachSlider(low,"low");attachSlider(mid,"mid");attachSlider(high,"high");attachSlider(hf,"hfAdjust");attachSlider(input,"input");attachSlider(output,"output");
 add(range);range.setBounds(306,331,40,50);attachChoice(range,"midRange");
 add(inMeter);add(outMeter);inMeter.setBounds(65,96,84,325);outMeter.setBounds(987,96,84,325);
 setTextButton(autoButton,"AUTO",791,47,111,26,"Analyze 12 s, then replay the loudest section for the second pass.");
 setTextButton(onButton,"ON",917,47,100,26,"Toggle just the REVIVE engine; EQ, trim and limiter remain active.",true);
 setTextButton(bypassButton,"BYPASS",1030,47,116,26,"Latency-aligned, optional gain-matched bypass.",true);
 setTextButton(limiterButton,"LIMITER",820,584,105,24,"True-peak detection limiter; configure the ceiling in Settings.",true);
 setTextButton(deltaButton,"DELTA",366,543,104,24,"Listen to level-matched difference between processed and dry signal.",true);
 setTextButton(matchButton,"GAIN MATCH",666,543,104,24,"Loudness-compensated compare; uses a smoothed RMS estimate.",true);
 attachButton(onButton,"reanimateOn");attachButton(bypassButton,"bypass");attachButton(limiterButton,"limiterOn");
 attachButton(deltaButton,"deltaListen");attachButton(matchButton,"levelMatch");
 autoButton.onClick=[this]{processor.requestAuto();};
 mode.addItemList({"VOCAL","MASTER","BUS"},1);add(mode);mode.setBounds(510,93,117,20);attachChoice(mode,"mode");
 mode.setTooltip("Select vocal-specific de-ess, mastering weight/width, or bus/transient voicing.");
 add(mix);mix.setSliderStyle(Slider::LinearHorizontal);mix.setTextBoxStyle(Slider::NoTextBox,false,0,0);
 mix.setColour(Slider::trackColourId,Theme::greenDim);mix.setColour(Slider::thumbColourId,Theme::green);
 mix.setBounds(484,548,118,15);mix.setRange(0,100,.1);attachSlider(mix,"mix");
 add(mixValue);mixValue.setBounds(604,543,60,24);mixValue.setFont(Theme::mono(10));mixValue.setColour(Label::textColourId,Theme::bone);
 add(push);push.setBounds(942,587,103,18);push.setRange(0,24,.1);attachSlider(push,"push");
 add(pushValue);pushValue.setBounds(1053,585,90,22);pushValue.setColour(Label::textColourId,Theme::bone);pushValue.setFont(Theme::mono(13));
 add(autopsy);autopsy.setBounds(221,580,268,31);
 add(lufs);lufs.setBounds(501,595,170,14);lufs.setFont(Theme::mono(9));lufs.setColour(Label::textColourId,Theme::faint.brighter(.9f));
 add(gr);gr.setBounds(680,595,70,14);gr.setFont(Theme::mono(9));gr.setColour(Label::textColourId,Theme::faint.brighter(.9f));
 add(status);status.setBounds(501,581,310,14);status.setFont(Theme::mono(9));status.setColour(Label::textColourId,Theme::greenLight);
 add(hints);hints.setBounds(0,0,Theme::width,Theme::height);hints.toFront(false);
 top.settings.onClick=[this]{openSettings();};top.help.onClick=[this]{showTour();};
 hintsOn=(bool)processor.uiState.getProperty("hints",true);
 if(hintsOn)tooltips=std::make_unique<TooltipWindow>(this,500);
 setResizable(true,true);setResizeLimits(810,428,1736,918);
 if(auto* c=getConstrainer())c->setFixedAspectRatio((double)Theme::width/Theme::height);
 // One-time upgrade: existing saved M-size windows open at the new compact default.
 if (!(bool)processor.uiState.getProperty("compactV2", false)) {
    const auto oldSize = processor.uiState.getProperty("window", "M").toString();
    if (oldSize == "M" || oldSize.isEmpty())
      processor.uiState.setProperty("window", "S", nullptr);
    processor.uiState.setProperty("compactV2", true, nullptr);
 }
 auto size=processor.uiState.getProperty("window","S").toString();
 changeSize(size=="S"?0:size=="L"?2:size=="XL"?3:1);
 // Align the moving rune, dial and meters with the display refresh.
 vblank=std::make_unique<VBlankAttachment>(this,[this](double){
     dial.setLive(processor.liveLevel.load());dial.setActive(onButton.getToggleState()&&!bypassButton.getToggleState());dial.tickFromVBlank();
     auto hold=(int)processor.uiState.getProperty("meterHold",1000);inMeter.setHoldMilliseconds(hold);outMeter.setHoldMilliseconds(hold);inMeter.tick();outMeter.tick();
 });
 startTimerHz(10);
 // The help tour is opt-in; automatically opening it covered the title/header.
}
SkellNecromancerAudioProcessorEditor::~SkellNecromancerAudioProcessorEditor(){stopTimer();vblank.reset();setLookAndFeel(nullptr);}
void SkellNecromancerAudioProcessorEditor::attachSlider(Slider&v,const String&id){sliderAttachments.push_back(std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,id,v));}
void SkellNecromancerAudioProcessorEditor::attachButton(Button&v,const String&id){buttonAttachments.push_back(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(processor.apvts,id,v));}
void SkellNecromancerAudioProcessorEditor::attachChoice(ComboBox&v,const String&id){choiceAttachments.push_back(std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(processor.apvts,id,v));}
void SkellNecromancerAudioProcessorEditor::setTextButton(TextButton&b,String name,int x,int y,int w,int h,const String&tip,bool toggle){
 b.setButtonText(name);b.setClickingTogglesState(toggle);b.setTooltip(tip);panel.addAndMakeVisible(b);b.setBounds(x,y,w,h);
}
void SkellNecromancerAudioProcessorEditor::paint(Graphics&g){g.fillAll(Theme::black);}
void SkellNecromancerAudioProcessorEditor::resized(){
 const float s=(float)getWidth()/Theme::width;
 panel.setBounds(0,0,Theme::width,Theme::height);panel.setTransform(AffineTransform::scale(s));
}
void SkellNecromancerAudioProcessorEditor::changeSize(int index){
 static const double factors[]={.70,1.,1.25,1.5};const int ix=juce::jlimit(0,3,index);
 processor.uiState.setProperty("window",StringArray{"S","M","L","XL"}[ix],nullptr);
 setSize((int)std::round(Theme::width*factors[ix]),(int)std::round(Theme::height*factors[ix]));
}
void SkellNecromancerAudioProcessorEditor::showTour(){
 processor.uiState.setProperty("tourSeen",true,nullptr);
 if(tourStep>=4){tourStep=-1;hints.setVisible(false);return;}
 tourStep=(tourStep+1)%5;hints.showTour(tourStep);
}
void SkellNecromancerAudioProcessorEditor::timerCallback(){
 // Maintain rune state even when the host does not provide VBlank callbacks.
 dial.setLive(processor.liveLevel.load());
 dial.setActive(onButton.getToggleState()&&!bypassButton.getToggleState());
 autopsy.repaint();top.presets.sync();
 const auto wf=processor.uiState.getProperty("lufsWindow","short").toString();
 float shown=wf=="momentary"?processor.momentary.load():processor.shortTerm.load();
 lufs.setText("LUFS "+String(shown,1)+" / I "+String(processor.integrated.load(),1)+"  T "+String((float)processor.uiState.getProperty("targetLUFS",-14.f),0),dontSendNotification);
 gr.setText("GR "+String(processor.gainReduction.load(),1),dontSendNotification);
 pushValue.setText(String(push.getValue(),1)+" dB",dontSendNotification);
 mixValue.setText("MIX  "+String(mix.getValue(),0)+"%",dontSendNotification);
 status.setText(processor.autoStatus(),dontSendNotification);
}
void SkellNecromancerAudioProcessorEditor::openSettings(){
 PopupMenu menu;
 menu.addSectionHeader("WINDOW SIZE");for(int i=0;i<4;++i)menu.addItem(10+i,StringArray{"S 70%","M 100%","L 125%","XL 150%"}[i]);
 menu.addSeparator();menu.addSectionHeader("MODE");
 menu.addItem(20,"VOCAL");menu.addItem(21,"MASTER");menu.addItem(22,"BUS");
 menu.addSeparator();menu.addSectionHeader("PROCESSING");
 menu.addItem(30,"Oversampling OFF",true,(int)processor.apvts.getRawParameterValue("oversampling")->load()==0);
 menu.addItem(31,"Oversampling 2x",true,(int)processor.apvts.getRawParameterValue("oversampling")->load()==1);
 menu.addItem(32,"Oversampling 4x",true,(int)processor.apvts.getRawParameterValue("oversampling")->load()==2);
 menu.addItem(40,"Soft clip before limiter",true,(bool)processor.uiState.getProperty("softClip",false));
 menu.addItem(50,"Ceiling -0.1 dBTP");menu.addItem(51,"Ceiling -0.3 dBTP");menu.addItem(52,"Ceiling -1.0 dBTP");
 menu.addSeparator();menu.addSectionHeader("WORKFLOW");
 menu.addItem(60,"Vertical knob drag",true,processor.uiState.getProperty("drag").toString()=="vertical");
 menu.addItem(61,"Circular knob drag",true,processor.uiState.getProperty("drag").toString()=="circular");
 menu.addItem(62,"Hints",true,hintsOn);menu.addItem(63,"First-run tour");
 menu.addItem(70,"Meter hold 0.5s");menu.addItem(71,"Meter hold 1s");menu.addItem(72,"Meter hold 2s");
 menu.addItem(80,"LUFS momentary");menu.addItem(81,"LUFS short-term");
 menu.addItem(90,"Target -14 LUFS");menu.addItem(91,"Target -9 LUFS");
 PopupMenu targetMenu;
 for(int i=0;i<=38;++i)targetMenu.addItem(150+i,String(-24.f+i*.5f,1)+" LUFS");
 menu.addSubMenu("Custom LUFS target",targetMenu);
 auto safe=Component::SafePointer<SkellNecromancerAudioProcessorEditor>(this);
 menu.showMenuAsync(PopupMenu::Options().withTargetComponent(&top.settings),[safe](int id){
  if(safe==nullptr||!id)return;auto& e=*safe;auto& p=e.processor;
  const auto setParam=[&](const char*name,float v){if(auto*a=p.apvts.getParameter(name)){a->beginChangeGesture();a->setValueNotifyingHost(a->convertTo0to1(v));a->endChangeGesture();}};
  if(id>=10&&id<=13)e.changeSize(id-10);
  else if(id>=20&&id<=22)setParam("mode",(float)(id-20));
  else if(id>=30&&id<=32)setParam("oversampling",(float)(id-30));
  else if(id==40)p.setSoftClip(!(bool)p.uiState.getProperty("softClip",false));
  else if(id>=50&&id<=52)setParam("ceiling",(float)(id-50));
  else if(id==60||id==61){const bool circular=id==61;p.uiState.setProperty("drag",circular?"circular":"vertical",nullptr);
     e.dial.setSliderStyle(circular?Slider::Rotary:Slider::RotaryVerticalDrag);
     for(auto*knob:{&e.low,&e.mid,&e.high,&e.hf,&e.input,&e.output})knob->setSliderStyle(circular?Slider::Rotary:Slider::RotaryVerticalDrag);
  }
  else if(id==62){e.hintsOn=!e.hintsOn;p.uiState.setProperty("hints",e.hintsOn,nullptr);if(e.hintsOn)e.tooltips=std::make_unique<TooltipWindow>(&e,500);else e.tooltips.reset();}
  else if(id==63)e.showTour();
  else if(id>=70&&id<=72)p.uiState.setProperty("meterHold",id==70?500:id==71?1000:2000,nullptr);
  else if(id==80||id==81)p.uiState.setProperty("lufsWindow",id==80?"momentary":"short",nullptr);
  else if(id==90||id==91)p.uiState.setProperty("targetLUFS",id==90?-14.f:-9.f,nullptr);
  else if(id>=150&&id<=188)p.uiState.setProperty("targetLUFS",-24.f+(id-150)*.5f,nullptr);
 });
}
AudioProcessorEditor* SkellNecromancerAudioProcessor::createEditor(){return new SkellNecromancerAudioProcessorEditor(*this);}
