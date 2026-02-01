#ifndef __DEV_PIM_CTRL_HH__
#define __DEV_PIM_CTRL_HH__

#include "dev/io_device.hh"
#include "params/PimCtrl.hh"

namespace gem5
{

class PimCtrl : public BasicPioDevice {
  public:
    PimCtrl(const PimCtrlParams &p);
    
    Tick read(PacketPtr pkt) override;
    Tick write(PacketPtr pkt) override; // 重点在这里：解析 CPU 的写请求
};

}


#endif 