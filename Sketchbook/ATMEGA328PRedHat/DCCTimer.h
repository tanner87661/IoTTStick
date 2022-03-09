#ifndef DCCTimer_h
#define DCCTimer_h

typedef void (*INTERRUPT_CALLBACK)();

class DCCTimer {
  public:
  static void begin(INTERRUPT_CALLBACK interrupt);
  private:
};


#endif
