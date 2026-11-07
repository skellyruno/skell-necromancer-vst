#pragma once
#include "TapeStage.h"
#include "GlueStage.h"
#include "HarmonicLife.h"
#include "TransientRestore.h"
#include "Glow.h"
namespace NecroDSP {
struct Voicing {float tape,glue,life,transient;};
inline constexpr Voicing voicings[3] = {{.76f,.93f,1.0f,.50f},{1.0f,1.0f,.76f,.45f},{.83f,.78f,.88f,1.0f}};
class ReviveEngine {
public:
 void prepare(double sr) noexcept {tape.prepare(sr);glue.prepare(sr);life.prepare(sr);transient.prepare(sr);glow.prepare(sr);}
 void reset() noexcept {tape.reset();glue.reset();life.reset();transient.reset();glow.reset();}
 void set(float value,int mode,int oversampling,float deEss) noexcept {
  int m=juce::jlimit(0,2,mode);float a=juce::jlimit(0.f,1.f,value);const auto v=voicings[m];
  tape.set(a*v.tape,m,oversampling);glue.set(a*v.glue,m);life.set(a*v.life,m,oversampling,deEss);transient.set(a*v.transient,m);glow.set(a,m);
 }
 void process(float& l,float& r) noexcept {
  glue.advance(l,r);transient.advance(l,r);life.advance(l,r);
  l=transient.process(glue.process(tape.process(0,l)));
  r=transient.process(glue.process(tape.process(1,r)));
  l=life.process(0,l);r=life.process(1,r);glow.process(l,r);
 }
 float getGainReduction() const noexcept {return glue.getGainReduction();}
 float sibilanceStrength() const noexcept {return life.sibilanceStrength();}
private:TapeStage tape;GlueStage glue;HarmonicLife life;TransientRestore transient;Glow glow;
};
}
