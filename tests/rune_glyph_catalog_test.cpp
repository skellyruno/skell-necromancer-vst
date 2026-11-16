#include "../Source/ui/RuneGlyphCatalog.h"
#include <iostream>
int main(){
 using namespace skell::necromancer::runes::glyphs;
 if(totalCount!=64||originalCount!=40||egyptianCount!=24)return 1;
 if(!validIndex(0)||!validIndex(63)||validIndex(64)||validIndex(-1))return 2;
 int motifs=0,originals=0;
 for(int i=0;i<totalCount;++i){
  if(isEgyptian(i)){
   ++motifs;
   if(motif(i)!=i-originalCount)return 3;
  }else{
   ++originals;
   if(motif(i)!=-1)return 4;
  }
 }
 if(motifs!=24||originals!=40||egyptianFraction!=.375f)return 5;
 // Ten complete catalogue cycles, matching the actual 640 placement count:
 int egyptianPlacements=0;
 for(int i=0;i<640;++i)if(isEgyptian(i%totalCount))++egyptianPlacements;
 if(egyptianPlacements!=240)return 6;
 std::cout<<"PASS: preserved 40 original motifs + 24 unique Egyptian-inspired motifs, 37.5% catalogue share\n";
}
