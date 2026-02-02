#ifndef __DEV_PIM_CTRL_HH__
#define __DEV_PIM_CTRL_HH__

#include "dev/io_device.hh"
#include "params/PimCtrl.hh"
#include "dev/arm/base_gic.hh"

namespace gem5
{

class PimCtrl : public BasicPioDevice {
  public:
    PimCtrl(const PimCtrlParams &p);
    
    Tick read(PacketPtr pkt) override;
    Tick write(PacketPtr pkt) override; // 重点在这里：解析 CPU 的写请求
  
  private:
    /*
      offset               |  size  |   描述
    ---------------------------------------
      0x0000-0x1FFF        |   8KB  | reg data

      0x2000-0x2007        |   8B   | command type
      0x2008-0x200F        |   8B   | weight start addr
      0x2010-0x2017        |   8B   | pim rank Num
      0x2018-0x201F        |   8B   | K
      0x2020-0x2027        |   8B   | N
      0x2028-0x202F        |   8B   | M
      0x2030-0x2037        |   8B   | input precision
      0x2038-0x203F        |   8B   | weight precision
      0x2040-0x2047        |   8B   | output precision

      0x3000-0x3007        |   8B   | trigger signal
      0x3008-0x300F        |   8B   | status
      0x3010-0x3017        |   8B   | intrrupt clear
    */
    uint8_t reg[16 * 1024]; // 16KB 的寄存器空间

  private:
    ArmInterruptPin *irq;
    bool pending = false;

    void raiseIrq() {
        if (!pending) {
            irq->raise();
            pending = true;
        }
    }

    void clearIrq() {
        if (pending) {
            irq->clear();
            pending = false;
        }
    }
};

}


#endif 