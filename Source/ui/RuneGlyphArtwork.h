#pragma once
#include <JuceHeader.h>
#include <initializer_list>
#include <utility>
#include "RuneGlyphCatalog.h"

// Twenty-four newly authored Egyptian-inspired LINE ART motifs.
// No text fonts, emoji fallback, licensed glyph library or runtime downloads.
// Paths are rasterised into unused cells of the existing atlas ONCE when the
// editor opens. The original 40 atlas cells remain pixel-for-pixel untouched.
namespace skell::necromancer::runes::glyphs {
using Pt = juce::Point<float>;
inline juce::Path motifPath(int id) {
 juce::Path p;
 auto line=[&](float x,float y,float xx,float yy){p.startNewSubPath(x,y);p.lineTo(xx,yy);};
 auto poly=[&](std::initializer_list<Pt> pts,bool close=false){
  auto it=pts.begin();if(it==pts.end())return;
  p.startNewSubPath(*it++);for(;it!=pts.end();++it)p.lineTo(*it);
  if(close)p.closeSubPath();
 };
 auto oval=[&](float x,float y,float w,float h){p.addEllipse(x,y,w,h);};
 auto q=[&](float x,float y,float cx,float cy,float ex,float ey){
  p.startNewSubPath(x,y);p.quadraticTo(cx,cy,ex,ey);
 };
 switch(id){
  case 0: // ankh — the loop of life
   oval(12,3,8,10);line(16,13,16,29);line(7,18,25,18);break;
  case 1: // eye with eyebrow, pupil, trailing tear
   p.startNewSubPath(3,16);p.quadraticTo(16,5,29,16);
   p.quadraticTo(16,26,3,16);oval(13,12,6,8);
   q(21,20,24,25,27,26);q(6,12,15,3,25,11);break;
  case 2: // scarab and winged carapace
   oval(12,10,8,13);oval(14,6,4,4);
   q(13,13,7,7,4,13);q(19,13,25,7,28,13);
   line(16,11,16,22);line(7,16,4,21);line(25,16,28,21);
   line(10,23,5,27);line(22,23,27,27);break;
  case 3: // ibis long-beaked profile
   q(9,27,10,13,18,8);q(18,8,23,5,24,10);
   q(24,10,25,13,29,15);line(29,15,17,17);
   line(17,17,11,27);oval(20,9,2,2);break;
  case 4: // rearing cobra with hood
   q(13,29,7,22,13,16);q(13,16,18,12,13,8);
   q(13,8,10,3,16,4);q(16,4,22,2,23,8);
   q(23,8,25,14,18,16);line(18,16,20,29);
   line(12,8,7,13);line(23,8,28,12);break;
  case 5: // thin vertical cartouche
   p.addRoundedRectangle(9,3,14,26,6.5f);
   line(16,7,16,25);line(11,29,21,29);break;
  case 6: // hooked was-staff with divided base
   line(14,8,14,27);q(14,8,13,3,19,5);
   line(14,27,9,29);line(14,27,19,29);
   line(12,14,17,14);break;
  case 7: // bundle of reeds
   for(int k=0;k<3;++k){float x=10.f+6.f*k;line(x,28,x,7);
    q(x,7,x-4,5,x-3,3);line(x,12,x+3,9);}
   line(5,28,27,28);break;
  case 8: // feather of Ma'at
   q(8,25,2,10,20,4);q(20,4,27,17,8,25);
   line(8,28,20,4);for(int k=0;k<4;++k)
   line(11.f+2*k,22.f-4*k,6.f+3*k,18.f-4*k);break;
  case 9: // sun disc with short radiants
   oval(11,11,10,10);
   for(int i=0;i<8;++i){float a=6.2831853f*i/8.f;
    line(16+8*std::cos(a),16+8*std::sin(a),
         16+12*std::cos(a),16+12*std::sin(a));}break;
  case 10: // flowering lotus
   line(16,20,16,29);q(16,21,5,16,8,10);
   q(16,21,27,16,24,10);q(16,22,8,12,16,5);
   q(16,5,24,12,16,22);line(9,26,23,26);break;
  case 11: // spread falcon wing and tail feathers
   poly({{4,17},{12,8},{20,10},{28,5}});
   poly({{4,17},{15,22},{23,19},{28,5}});
   line(9,19,6,26);line(14,22,13,28);line(20,20,23,26);break;
  case 12: // djed-style stability pillar
   line(12,29,12,13);line(20,29,20,13);
   for(int i=0;i<4;++i)line(7.f+i*.7f,7.f+i*4.f,25.f-i*.7f,7.f+i*4.f);
   line(7,29,25,29);break;
  case 13: // obelisk
   poly({{12,29},{12,10},{16,3},{20,10},{20,29}},true);
   line(8,29,24,29);line(14,11,18,11);break;
  case 14: // papyrus stalk and crown
   line(16,14,16,29);line(16,13,7,6);line(16,13,11,3);
   line(16,13,16,2);line(16,13,21,3);line(16,13,25,6);
   q(7,6,16,17,25,6);break;
  case 15: // ritual spiral
   p.startNewSubPath(25,17);p.cubicTo(25,8,10,5,7,16);
   p.cubicTo(5,26,21,29,23,18);p.cubicTo(24,12,14,11,13,17);
   p.cubicTo(12,21,19,22,18,17);break;
  case 16: // three water ripples
   for(int row=0;row<3;++row){
    float y=9.f+7.f*row;
    p.startNewSubPath(3,y);p.cubicTo(8,y-5,12,y+5,17,y);
    p.cubicTo(22,y-5,26,y+5,29,y);
   }break;
  case 17: // solar barque on water
   q(4,20,16,29,28,20);line(4,20,28,20);
   line(16,20,16,6);oval(13,4,6,6);
   q(16,8,7,12,11,17);line(4,28,28,28);break;
  case 18: // jackal head silhouette
   poly({{6,24},{8,15},{12,4},{15,13},{21,5},{22,17},{27,20},{23,24},{6,24}},true);
   oval(18,16,2,2);line(21,20,27,20);break;
  case 19: // scarab wing spread
   oval(13,13,6,9);oval(14,9,4,3);
   q(13,15,3,11,4,23);q(19,15,29,11,28,23);
   line(4,23,13,24);line(19,24,28,23);line(16,13,16,23);break;
  case 20: // temple gate
   poly({{5,27},{7,10},{25,10},{27,27}});
   line(4,27,28,27);line(5,8,27,8);
   line(12,27,12,15);line(20,27,20,15);line(12,15,20,15);break;
  case 21: // lunar crescent over starlight
   q(22,4,10,5,10,16);q(10,16,11,25,23,27);
   q(23,27,5,28,6,15);q(6,15,5,6,22,4);
   line(25,11,25,18);line(22,14,28,14);break;
  case 22: // crook staff and small flail
   q(12,27,9,13,13,6);q(13,6,15,2,20,5);
   q(20,5,24,8,20,11);line(20,11,17,11);
   line(22,17,28,25);line(22,17,22,28);break;
  case 23: // paired plumes on ceremonial column
   line(16,11,16,29);q(15,14,4,9,8,3);
   q(16,13,28,8,24,3);q(8,3,10,14,16,16);
   q(24,3,22,14,16,16);line(11,29,21,29);break;
  default:break;
 }
 return p;
}

inline juce::Image extendedAtlas(const juce::Image& original) {
 juce::Image image(juce::Image::ARGB,columns*cellSize,columns*cellSize,true);
 juce::Graphics g(image);
 if(!original.isNull())g.drawImageAt(original,0,0);
 for(int motifId=0;motifId<egyptianCount;++motifId){
  const int index=originalCount+motifId;
  juce::Graphics::ScopedSaveState state(g);
  g.addTransform(juce::AffineTransform::translation(
    float((index%columns)*cellSize),float((index/columns)*cellSize)));
  g.setColour(juce::Colours::white);
  const auto motif=motifPath(motifId);
  g.strokePath(motif,juce::PathStrokeType(2.25f,
                 juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 }
 return image;
}
}
