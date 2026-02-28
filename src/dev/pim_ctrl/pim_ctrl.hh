#ifndef __DEV_PIM_CTRL_HH__
#define __DEV_PIM_CTRL_HH__

#include "dev/io_device.hh"
#include "params/PimCtrl.hh"
#include "dev/arm/base_gic.hh"
#include "mem/ramulator2.hh"
#include "dev/gemv_request.hh"

namespace gem5
{

class PimCtrl : public BasicPioDevice {
  public:



    PimCtrl(const PimCtrlParams &p);
    
    Tick read(PacketPtr pkt) override;
    Tick write(PacketPtr pkt) override; // 重点在这里：解析 CPU 的写请求
    void sendGemvRequest();            // 发送GEMV请求
  
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
    #define REG_WEIGHT_ADDR  0x2000  // 权重矩阵起始物理地址（8字节）
    #define REG_RANK_NUM     0x2008  //使用的rank数目（1字节，预留7字节对齐）
    #define REG_MATRIX_K     0x2010  // 矩阵维度 K（4字节，预留4字节对齐）
    #define REG_MATRIX_N     0x2018  // 矩阵维度 N（4字节，预留4字节对齐）
    #define REG_INPUT_PREC   0x2030  // 输入数据精度（如8/16/32，1字节）
    #define REG_WEIGHT_PREC  0x2038  // 权重数据精度（字节）
    #define REG_SCALE_PREC   0x2040  // Scale精度（1字节）
    #define REG_CMD          0x2048  // 命令寄存器（写入1触发GEMV执行，1字节）
    #define REG_INPUT_DATA   0x0000   // 输入向量数据写入起始偏移（批量写入INT8数据）
    uint8_t reg[16 * 1024];           // 16KB 的寄存器空间

  private:
    gem5::GemvRequest *gemv_req;
    ArmInterruptPin *irq;
    gem5::memory::Ramulator2* ramulator2_ptr; // 用于发送GEMV请求
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