#pragma once
#include "../Theme.h"
#include "BinaryData.h"
class MetalKnob:public juce::Slider{
public:
 explicit MetalKnob(juce::String text):caption(std::move(text)) {
  setSliderStyle(juce::Slider::RotaryVerticalDrag);setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
  setMouseCursor(juce::MouseCursor::PointingHandCursor);
  setDoubleClickReturnValue(true,0.0);setPopupMenuEnabled(true);setPopupDisplayEnabled(true,false,nullptr);
  small=juce::ImageCache::getFromMemory(BinaryData::knob_cap_small_png,BinaryData::knob_cap_small_pngSize);
 }
 void setDefault(double d){setDoubleClickReturnValue(true,d);}
 void mouseEnter(const juce::MouseEvent&e)override{juce::Slider::mouseEnter(e);repaint();}
 void mouseExit(const juce::MouseEvent&e)override{juce::Slider::mouseExit(e);repaint();}
 // A plain click (no drag) on the value readout opens an inline numeric editor.
 void mouseUp(const juce::MouseEvent&e)override{
  juce::Slider::mouseUp(e);
  if(!e.mouseWasDraggedSinceMouseDown()&&!e.mods.isPopupMenu()&&valueArea().contains(e.getMouseDownPosition()))beginEdit();
 }
 void paint(juce::Graphics&g)override{
  const auto b=getLocalBounds().toFloat();
  const float cx=b.getCentreX();
  const float r=radius(),cy=r+7;
  g.setColour(juce::Colours::black.withAlpha(.34f));g.fillEllipse(cx-r-7,cy-r-5,2*(r+7),2*(r+7));
  g.setColour(juce::Colour(0xffa4a8a9).withAlpha(.19f));
  g.drawEllipse(cx-r-5.f,cy-r-5.f,2.f*(r+5.f),2.f*(r+5.f),.9f);
  // Fine machined rim highlight; cap artwork remains the original asset.
  juce::Path specular;
  specular.addCentredArc(cx,cy,r+3.f,r+3.f,0.f,4.0f,5.7f,true);
  g.setColour(juce::Colours::white.withAlpha(.17f));
  g.strokePath(specular,juce::PathStrokeType(.9f));
  const int lit=(int)std::round(25.f*(float)valueToProportionOfLength(getValue()));
  for(int k=0;k<=25;++k){float angle=juce::MathConstants<float>::pi*(.75f+1.5f*k/25.f);
    auto rr=r+5;auto xx=cx+std::cos(angle)*rr,yy=cy+std::sin(angle)*rr;
    const auto on=k<lit;
    g.setColour(on?(k>=lit-2?Theme::greenLight.withAlpha(.78f):Theme::bone.withAlpha(.63f))
                  :Theme::bone.withAlpha(.23f));
    g.drawLine(xx,yy,cx+std::cos(angle)*(rr+3.f),cy+std::sin(angle)*(rr+3.f),1.f);
  }
  if(!small.isNull()){juce::Graphics::ScopedSaveState save(g);g.setOpacity(1.f); // drawImage takes its alpha from the current colour
      const float a=(float)(valueToProportionOfLength(getValue())*4.712389-2.35619);
      g.addTransform(juce::AffineTransform::rotation(a,cx,cy));g.drawImage(small,juce::Rectangle<float>(cx-r,cy-r,r*2,r*2));}
  else{g.setColour(Theme::border);g.fillEllipse(cx-r,cy-r,2*r,2*r);}
  if(isMouseOverOrDragging()){g.setColour(Theme::greenLight.withAlpha(.46f));g.drawEllipse(cx-r-2,cy-r-2,2*r+4,2*r+4,1.f);}
  // Slightly larger high-contrast readout holds up better at the 70% editor size.
  g.setFont(Theme::mono(16));g.setColour(Theme::bone);
  g.drawFittedText(juce::String(getValue(),1)+(caption=="MIX"||caption=="WIDTH"?" %":" dB"),
     valueArea(),juce::Justification::centred,1);
  g.setFont(Theme::mono(12).withExtraKerningFactor(.08f));g.setColour(Theme::bone.withAlpha(.88f));
  // The caption starts *below* the numeric readout at compact sizes.
  g.drawFittedText(caption,juce::Rectangle<int>(0,valueArea().getBottom()+3,getWidth(),17),
                   juce::Justification::centred,1);
 }
private:
 float radius()const{return std::min((float)getWidth(),(float)getHeight()-31.f)*.39f;}
 juce::Rectangle<int> valueArea()const{return {0,(int)(radius()*2+16),getWidth(),18};}
 void beginEdit(){
  if(editor!=nullptr)return;
  editor=std::make_unique<juce::TextEditor>();
  editor->setJustification(juce::Justification::centred);editor->setFont(Theme::mono(15));
  editor->setInputRestrictions(8,"0123456789.-+");
  editor->setColour(juce::TextEditor::backgroundColourId,Theme::black);
  editor->setColour(juce::TextEditor::textColourId,Theme::bone);
  editor->setColour(juce::TextEditor::outlineColourId,Theme::green);
  editor->setColour(juce::TextEditor::focusedOutlineColourId,Theme::green);
  editor->setText(juce::String(getValue(),1),false);
  addAndMakeVisible(*editor);editor->setBounds(valueArea().reduced(6,0));
  editor->onReturnKey=[this]{setValue(editor->getText().getDoubleValue(),juce::sendNotificationSync);endEdit();};
  editor->onEscapeKey=[this]{endEdit();};
  editor->onFocusLost=[this]{endEdit();};
  editor->grabKeyboardFocus();editor->selectAll();
 }
 // Deferred: the editor must not be destroyed from inside its own callback.
 void endEdit(){juce::MessageManager::callAsync([sp=juce::Component::SafePointer<MetalKnob>(this)]{if(sp!=nullptr)sp->editor.reset();});}
 std::unique_ptr<juce::TextEditor> editor;
 juce::String caption;juce::Image small;
};
