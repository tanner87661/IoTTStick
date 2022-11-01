  int kGain100 = 1000;
  int errEst100 = 1000;
  int errMeasure100 = 800;
  int currEst100 = 1000;

int getEstimate(int Measurement)
{
  int Measurement100 = Measurement * 100;
  int lastEst100 = currEst100;
  kGain100 = round(100 * errEst100 / (errEst100 + errMeasure100));
  currEst100 = currEst100 + round(kGain100 * (Measurement100 - currEst100) / 100);
  errEst100 = round((100 - kGain100) * errEst100 / 100) + abs(lastEst100 - currEst100);
  errEst100 = round(errEst100 * (100 - kGain100) / 100);
  return round(currEst100/100);
}
