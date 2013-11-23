
class Model
{
private:
  uint32_t update_threshold;
  uint32_t updated_at;

  int rpm;
  int kph;
  float oil_temp;
  float throttle;

public:
  /**
   * @brief Construct a new Model object
   * 
   * @param update_rate The rate at which the model should be updated in Hz
   */
  Model(uint8_t update_rate) : rpm(0), kph(0), oil_temp(0), throttle(0)
  {
    this->update_threshold = uint32_t(1000.0 / update_rate);
  }

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

  bool should_update()
  {
    uint32_t now = millis();
    if (now - updated_at < update_threshold)
    {
      return false;
    }
    updated_at = now;
    return true;
  }
};