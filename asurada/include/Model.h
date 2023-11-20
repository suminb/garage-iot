
class Model
{
private:
  int rpm;
  int kph;
  float oil_temp;
  float throttle;

public:
  int get_rpm()
  {
    return rpm;
  }

  void set_rpm(int rpm)
  {
    this->rpm = rpm;
  }

  int get_kph()
  {
    return kph;
  }

  void set_kph(int kph)
  {
    this->kph = kph;
  }

  float get_oil_temp()
  {
    return oil_temp;
  }

  void set_oil_temp(float temp)
  {
    this->oil_temp = temp;
  }

  float get_throttle()
  {
    return throttle;
  }

  void set_throttle(float throttle)
  {
    this->throttle = throttle;
  }
};