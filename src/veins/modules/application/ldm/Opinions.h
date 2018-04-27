#ifndef OPINIONS_H_
#define OPINIONS_H_
class Opinions {
  private:
    double belief = 0;
    double disbelief = 0;
    double uncertainty = 1;

  public:
    void setbelief(double bel); //override;
    double getbelief(); //override;
    void setdisbelief(double disbel); //override;
    double getdisbelief(); //override;
    void setuncertainty(double uncert); //override;
    double getuncertainty(); //override;
    bool isConsistent();
};
#endif /* OPINIONS_H*/
