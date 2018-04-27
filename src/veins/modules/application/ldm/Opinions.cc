#include "Opinions.h"
#include<cmath> //for std::abs on doubles
#include<veins/base/utils/FWMath.h> //for EPSILON to compare doubles

///////////////////class "Opinions" functions///////////////

void Opinions::setbelief(double bel) {
  belief = bel;
}
double Opinions::getbelief() {
  return belief;
}
void Opinions::setdisbelief(double disbel) {
  disbelief = disbel;
}
double Opinions::getdisbelief() {
  return disbelief;
}
void Opinions::setuncertainty(double uncert) {
  uncertainty = uncert;
}
double Opinions::getuncertainty() {
  return uncertainty;
}
bool Opinions::isConsistent(){
  return uncertainty >= 0 && belief >= 0 && disbelief >= 0 &&
    uncertainty <= 1 && belief <= 1 && disbelief <= 1 &&
    std::abs((uncertainty+belief+disbelief)-1) <= EPSILON;
}
//////////////////////////////////////////////////////////

