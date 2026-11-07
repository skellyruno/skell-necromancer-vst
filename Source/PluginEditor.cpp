#include "PluginEditor.h"
#include "BinaryData.h"
#include <cmath>
using namespace juce;
SkellNecromancerAudioProcessorEditor::Panel::Panel(){
 bg=ImageCache::getFromMemory(BinaryData::bg_brushed_png,BinaryData::bg_brushed_pngSize);
 setBufferedToImage(true);
}
void SkellNecromancerAudioProcessorEditor::Panel::paint(Graphics&g){
 g.fillAll(Theme::black);g.drawImage(bg,getLocalBounds().toFloat());
 g.setColour(Theme::panelRaised);g.fillRect(0,35,Theme::width,44);
 g.setColour(Theme::border);g.drawLine(0.f,80.f,1157.f,80.f,1.f);
 // Original Skell skull-and-wings mark, deliberately not a copy of the reference WM logo.
 Path skull;skull.addEllipse(24.f,45.f,22.f,21.f);skull.addRectangle(30.f,62.f,10.f,8.f);
 g.setColour(Theme::bone);g.strokePath(skull,PathStrokeType(1.7f));
 g.fillEllipse(29,54,3,3);g.fillEllipse(38,54,3,3);
 Path wing;wing.startNewSubPath(22,51);wing.lineTo(10,47);wing.lineTo(16,57);wing.lineTo(7,60);wing.lineTo(22,61);
 wing.startNewSubPath(47,51);wing.lineTo(60,47);wing.lineTo(54,57);wing.lineTo(63,60);wing.lineTo(47,61);
 g.strokePath(wing,PathStrokeType(2.f));
 g.setFont(Theme::display(24));g.setColour(Theme::bone);
 g.drawFittedText("S K E L L  -  N E C R O M A N C E R",300,40,510,31,Justification::centred,1);
 g.setColour(Theme::greenLight);g.setFont(Theme::mono(9));
 g.drawText("R A I S E   T H E   D E A D   M I X",423,69,292,13,Justification::centred);
 g.setFont(Theme::mono(9));g.setColour(Theme::faint.brighter(.7f));g.drawText("SKELL AUDIO",27,586,160,16,Justification::centredLeft);
 g.setFont(Theme::mono(8));g.setColour(Theme::greenDim.brighter(.8f));
 g.drawFittedText("REVIVE / TONE / LOUDNESS",440,558,264,10,Justification::centred,1);
}
SkellNecromancerAudioProcessorEditor::SkellNecromancerAudioProcessorEditor(SkellNecromancerAudioProcessor&p):
 AudioProcessorEditor(&p),processor(p),top(p),inMeter(p.inputL,p.inputR),outMeter(p.outputL,p.outputR,true),autopsy(p){
 setLookAndFeel(&look);addAndMakeVisible(panel);panel.setBounds(0,0,Theme::width,Theme::height);
 auto add=[this](Component&c){panel.addAndMakeVisible(c);};
 add(top);top.setBounds(0,0,Theme::width,36);
 add(dial);dial.setBounds(Theme::dial);attachSlider(dial,"reanimate");
 for(auto*knob:{&low,&mid,&high,&hf,&input,&output})add(*knob);
 low.setBounds(225,166,74,118);mid.setBounds(225,320,74,118);high.setBounds(845,166,74,118);hf.setBounds(845,320,74,118);
 input.setBounds(63,438,86,116);output.setBounds(991,438,86,116);
 low.setRange(-6,6,.1);low.setDefault(1.5);mid.setRange(-6,6,.1);mid.setDefault(-1);
 high.setRange(-6,6,.1);high.setDefault(2);hf.setRange(-6,6,.1);hf.setDefault(1);
 input.setRange(-12,12,.1);output.setRange(-12,12,.1);
 attachSlider(low,"low");attachSlider(mid,"mid");attachSlider(high,"high");attachSlider(hf,"hfAdjust");attachSlider(input,"input");attachSlider(output,"output");
 add(range);range.setBounds(303,351,50,24);attachChoice(range,"midRange");
 add(inMeter);add(outMeter);inMeter.setBounds(65,96,84,325);outMeter.setBounds(998,96,84,325);
 setTextButton(autoButton,"⏻ AUTO",791,47,111,26,"Analyze 12 s, then replay the loudest section for the second pass.");
 setTextButton(onButton,"⏻ ON",917,47,100,26,"Toggle just the REVIVE engine; EQ, trim and limiter remain active.",true);
 setTextButton(bypassButton,"⏻ BYPASS",1030,47,116,26,"Latency-aligned, optional gain-matched bypass.",true);
 setTextButton(limiterButton,"⏻ LIMITER",820,584,105,24,"True-peak detection limiter; configure the ceiling in Settings.",true);
 setTextButton(deltaButton,"Δ LISTEN",784,547,94,25,"Listen to level-matched difference between processed and dry signal.",true);
 setTextButton(matchButton,"GAIN MATCH",886,547,109,25,"Loudness-compensated compare; uses a smoothed RMS estimate.",true);
 attachButton(onButton,"reanimateOn");attachButton(bypassButton,"bypass");attachButton(limiterButton,"limiterOn");
 attachButton(deltaButton,"deltaListen");attachButton(matchButton,"levelMatch");
 autoButton.onClick=[this]{processor.requestAuto();};
 mode.addItemList({"VOCAL","MASTER","BUS"},1);add(mode);mode.setBounds(510,84,117,20);attachChoice(mode,"mode");
 mode.setTooltip("Select vocal-specific de-ess, mastering weight/width, or bus/transient voicing.");
 add(mix);mix.setSliderStyle(Slider::LinearHorizontal);mix.setTextBoxStyle(Slider::NoTextBox,false,0,0);
 mix.setColour(Slider::trackColourId,Theme::greenDim);mix.setColour(Slider::thumbColourId,Theme::green);
 mix.setBounds(510,542,138,15);mix.setRange(0,100,.1);attachSlider(mix,"mix");
 add(mixValue);mixValue.setBounds(648,536,92,22);mixValue.setFont(Theme::mono(10));mixValue.setColour(Label::textColourId,Theme::bone);
 add(push);push.setBounds(942,587,103,18);push.setRange(0,24,.1);attachSlider(push,"push");
 add(pushValue);pushValue.setBounds(1053,583,90,22);pushValue.setColour(Label::textColourId,Theme::bone);pushValue.setFont(Theme::mono(13));
 add(autopsy);autopsy.setBounds(257,580,235,31);
 add(lufs);lufs.setBounds(501,583,244,25);lufs.setFont(Theme::mono(10));lufs.setColour(Label::textColourId,Theme::greenLight);
 add(gr);gr.setBounds(749,583,69,23);gr.setFont(Theme::mono(10));gr.setColour(Label::textColourId,Theme::bone);
 add(status);status.setBounds(10,546,325,24);status.setFont(Theme::mono(10));status.setColour(Label::textColourId,Theme::greenLight);
 add(hints);hints.setBounds(0,0,Theme::width,Theme::height);hints.toFront(false);
 top.settings.onClick=[this]{openSettings();};top.help.onClick=[this]{showTour();};
 hintsOn=(bool)processor.uiState.getProperty("hints",true);
 if(hintsOn)tooltips=std::make_unique<TooltipWindow>(this,500);
 setResizable(true,true);setResizeLimits(868,459,1736,918);
 if(auto* c=getConstrainer())c->setFixedAspectRatio((double)Theme::width/Theme::height);
 auto size=processor.uiState.getProperty("window","M").toString();changeSize(size=="S"?0:size=="L"?2:size=="XL"?3:1);
 // Align the moving rune, dial and meters with the display refresh.
 vblank=std::make_unique<VBlankAttachment>(this,[this](double){
     dial.setLive(processor.liveLevel.load());dial.advance();
     auto hold=(int)processor.uiState.getProperty("meterHold",1000);inMeter.setHoldMilliseconds(hold);outMeter.setHoldMilliseconds(hold);inMeter.tick();outMeter.tick();
 });
 startTimerHz(10);
 if(!(bool)processor.uiState.getProperty("tourSeen",false)){
     processor.uiState.setProperty("tourSeen",true,nullptr);showTour();
 }
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
 static const double factors[]={.75,1.,1.25,1.5};const int ix=juce::jlimit(0,3,index);
 processor.uiState.setProperty("window",StringArray{"S","M","L","XL"}[ix],nullptr);
 setSize((int)std::round(Theme::width*factors[ix]),(int)std::round(Theme::height*factors[ix]));
}
void SkellNecromancerAudioProcessorEditor::showTour(){
 if(tourStep>=4){tourStep=-1;hints.setVisible(false);return;}
 tourStep=(tourStep+1)%5;hints.showTour(tourStep);
}
void SkellNecromancerAudioProcessorEditor::timerCallback(){
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
 menu.addSectionHeader("WINDOW SIZE");for(int i=0;i<4;++i)menu.addItem(10+i,StringArray{"S 75%","M 100%","L 125%","XL 150%"}[i]);
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
