#pragma once
#include <JuceHeader.h>
#include <cmath>
namespace NecroDSP {
struct Biquad {
    double b0=1,b1=0,b2=0,a1=0,a2=0, z1[2]{},z2[2]{};
    void reset() noexcept { z1[0]=z1[1]=z2[0]=z2[1]=0; }
    double process(int ch,double x) noexcept { const auto y=b0*x+z1[ch];z1[ch]=b1*x-a1*y+z2[ch];z2[ch]=b2*x-a2*y;return y; }
    void setRaw(double b0_,double b1_,double b2_,double a0_,double a1_,double a2_) noexcept {const double d=1.0/a0_;b0=b0_*d;b1=b1_*d;b2=b2_*d;a1=a1_*d;a2=a2_*d;}
    enum class Kind { peak, lowShelf, highShelf, highpass };
    // RBJ audio EQ cookbook: signed gains yield reciprocal transfer functions.
    void design(Kind kind,double sr,double hz,double db,double q=0.70710678) noexcept {
        const double w=juce::MathConstants<double>::twoPi*juce::jlimit(5.0,sr*.45,hz)/sr;
        const double c=std::cos(w),s=std::sin(w),A=std::pow(10.0,db/40.0);
        if(kind==Kind::highpass) {const auto a=s/(2.0*q);setRaw((1+c)/2,-(1+c),(1+c)/2,1+a,-2*c,1-a);return;}
        if(kind==Kind::peak) {const double a=s/(2.0*q);setRaw(1+a*A,-2*c,1-a*A,1+a/A,-2*c,1-a/A);return;}
        // Slope=1.0 shelf, numerically stable at supported sample rates.
        const double alpha=s/std::sqrt(2.0), root=2.0*std::sqrt(A)*alpha;
        if(kind==Kind::lowShelf)
            setRaw(A*((A+1)-(A-1)*c+root),2*A*((A-1)-(A+1)*c),A*((A+1)-(A-1)*c-root),
                   (A+1)+(A-1)*c+root,-2*((A-1)+(A+1)*c),(A+1)+(A-1)*c-root);
        else
            setRaw(A*((A+1)+(A-1)*c+root),-2*A*((A-1)+(A+1)*c),A*((A+1)+(A-1)*c-root),
                   (A+1)-(A-1)*c+root,2*((A-1)-(A+1)*c),(A+1)-(A-1)*c-root);
    }
};
inline double dbToGain(double d) noexcept {return std::pow(10.0,d/20.0);}
inline double gainToDb(double g) noexcept {return 20.0*std::log10(std::max(g,1.0e-12));}
}
