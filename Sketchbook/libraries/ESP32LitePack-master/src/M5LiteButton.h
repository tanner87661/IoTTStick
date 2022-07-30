#ifndef __M5LiteButton_H__
#define __M5LiteButton_H__

typedef void (*btnEvt) ();
typedef void (*btnCtrEvt) (uint8_t);

#define dblClickTime 1000
#define holdTime 2000

class M5LiteButton {
  public:
    btnEvt onBtnDown = NULL;
    btnEvt onBtnUp = NULL;
    btnEvt onBtnClick = NULL;
    btnCtrEvt onBtnDblClick = NULL;
    btnCtrEvt onBtnHold = NULL; 
    
    void begin(int8_t pin, uint8_t invert, uint32_t dbTime) {
      _pin = pin;
      if (_pin == -1) {
        return;
      }
      if (_pin < 40) {
        pinMode(_pin, INPUT);
      }
      _invert = invert;
      _dbTime = dbTime;
      _state = getPin();
      _time = millis();
      _lastState = _state;
      _changed = 0;
      _changedLow = 0;
      _changedHigh = 0;
      _clear = 0;
      _hold_time = -1;
      _lastHoldTime = _time;
      _lastTime = _time;
      _lastClickTime = _time;
      _lastChange = _time;
      _pressTime = _time;
      _holdCtr = 0;
      _clkCtr = 0;
    }

    void setAXP192(I2C_AXP192 *axp192) {
      _axp192 = axp192;
    }

    void setTouch(M5LiteTouch *Touch) {
      _Touch = Touch;
    }

	uint8_t getPwrPin(void)
	{
		return _axp192->getPekPress();
	}
	
    uint8_t getPin(void) {
      if (_pin == -1) {
        return 0;
      } else if (_pin == 100) {
        // AXP192 Pek
        return _axp192->getPekPress(); // != 0;
      } else if (_pin == 101) {
        // Touch A
        TouchPoint_t p = _Touch->getPressPointRaw();
        if (240 < p.y && p.x <= 106) {
          return true;
        } else {
          return false;
        }
      } else if (_pin == 102) {
        // Touch B
        TouchPoint_t p = _Touch->getPressPointRaw();
        if (240 < p.y && 106 < p.x && p.x < 212) {
          return true;
        } else {
          return false;
        }
      } else if (_pin == 103) {
        // Touch C
        TouchPoint_t p = _Touch->getPressPointRaw();
        if (240 < p.y && 212 <= p.x) {
          return true;
        } else {
          return false;
        }
      }

      if (_invert) {
        return !digitalRead(_pin);
      }
      return digitalRead(_pin);
    }

    uint8_t read() {
      static uint32_t nowtime;
      static uint8_t pinVal;

      if (_clear) {
        _changed = 0;
        _changedLow = 0;
        _changedHigh = 0;
        _clear = 0;
      }

      nowtime = millis();
      pinVal = getPin();
      if (_pin == 100)
      {
			switch (pinVal)
			{
				case 2: //short press
					if ((nowtime - _lastClickTime) < dblClickTime)
					{
						_clkCtr++;
						if (onBtnDblClick) onBtnDblClick(_clkCtr);
					}
					else   
					{         
						if (onBtnClick) onBtnClick();
						_clkCtr = 0;
					}
					_lastClickTime = nowtime;
					break;
				case 1: //long press
					Serial.println("Pin C Hold");
					if (onBtnHold) onBtnHold(1);
					break;
			}
			return _state;
	  }
      if (nowtime - _lastChange < _dbTime) {
        _lastTime = _time;
        _time = nowtime;
        return _state; //wait for debounce time
      } else {
        _lastTime = _time;
        _time = nowtime;
        _lastState = _state;
        _state = pinVal;
        if (_state != _lastState) 
        {
          _lastChange = nowtime;
          _changed = 1;
          if (_state) 
          {
            _pressTime = _time;
            _lastHoldTime = _time;
            _holdCtr = 0;
            _changedHigh = 1;
			if (onBtnDown) onBtnDown();
          } 
          else 
          {
            if (onBtnUp) onBtnUp();
            if ((_lastTime - _lastClickTime) < dblClickTime)
            {
				_clkCtr++;
				if (onBtnDblClick) onBtnDblClick(_clkCtr);
			}
			else   
			{         
				if (onBtnClick) onBtnClick();
				_clkCtr = 0;
			}
			_lastClickTime = _lastTime;
            _changedLow = 1;
          }
        }
        else
        {
          if (_state) 
          {
            if ((_time - _lastHoldTime) > holdTime)
            {
				if (onBtnHold) onBtnHold(_holdCtr);
				_holdCtr++;
				_lastHoldTime = _time;
			}
		  }
		}
        return _state;
      }
    }

    bool    processEvents()
    {
		read();
	}

    uint8_t isPressed() {
      _clear = 1;
      return _state == 0 ? 0 : 1;
    }

    uint8_t isReleased() {
      _clear = 1;
      return _state == 0 ? 1 : 0;
    }

    uint8_t wasPressed() {
      _clear = 1;
      return _changedHigh;
    }

    uint8_t wasReleased() {
      _clear = 1;
      return _changedLow && millis() - _pressTime < _hold_time;
    }

    uint8_t pressedFor(uint32_t ms) {
      _clear = 1;
      return (_state == 1 && _time - _lastChange >= ms) ? 1 : 0;
    }

    uint8_t releasedFor(uint32_t ms) {
      _clear = 1;
      return (_state == 0 && _time - _lastChange >= ms) ? 1 : 0;
    }

    uint8_t wasReleasefor(uint32_t ms) {
      _clear = 1;
      _hold_time = ms;
      return !_state && _changed && millis() - _pressTime >= ms;
    }

    uint32_t lastChange() {
      _clear = 1;
      return _lastChange;
    }


  private:
    int8_t _pin;
    uint8_t _puEnable;
    uint8_t _invert;
    uint8_t _state;
    uint8_t _lastState;
    uint8_t _changed;
    uint8_t _changedHigh;
    uint8_t _changedLow;
    uint8_t _clear;
    uint8_t _holdCtr;
    uint8_t _clkCtr;
    uint32_t _time;
    uint32_t _lastTime;
    uint32_t _lastClickTime;
    uint32_t _lastChange;
    uint32_t _dbTime;
    uint32_t _pressTime;
    uint32_t _clickTime;
    uint32_t _hold_time;
    uint32_t _lastHoldTime;

    I2C_AXP192 *_axp192;
    M5LiteTouch *_Touch;
};

#endif
