#include "pim_ctrl.hh"
#include "base/trace.hh"    // 用于调试 DPRINTF
#include "debug/PIMCTRL.hh"


namespace gem5
{
// 构造函数实现：必须透传参数给基类 BasicPioDevice
PimCtrl::PimCtrl(const PimCtrlParams &p)
    : BasicPioDevice(p, p.pio_size), irq(p.interrupt->get()), pending(false)
{
}

Tick PimCtrl::read(PacketPtr pkt) {
    DPRINTF(PIMCTRL, "PimCtrl: Read Packet Received: addr=0x%lx, size=%lu\n", pkt->getAddr(), pkt->getSize());
    pkt->makeResponse();
    Addr offset = pkt->getAddr() - pioAddr;
    if(pkt->getAddr() >= pioAddr && pkt->getAddr() + pkt->getSize() <= pioAddr + pioSize){
        pkt->setData(reg + offset);
    }else{
        DPRINTF(PIMCTRL, "PimCtrl: Invalid Read Address: addr=0x%lx, size=%lu\n", pkt->getAddr(), pkt->getSize());
    }
    return pioDelay;
}

Tick PimCtrl::write(PacketPtr pkt) {
    DPRINTF(PIMCTRL, "PimCtrl: Write Packet Received: addr=0x%lx, size=%lu\n", pkt->getAddr(), pkt->getSize());
    raiseIrq();

    Addr offset = pkt->getAddr() - pioAddr;
    if(pkt->getAddr() >= pioAddr && pkt->getAddr() + pkt->getSize() <= pioAddr + pioSize){
        pkt->writeData(reg + offset);
    }else{
        DPRINTF(PIMCTRL, "PimCtrl: Invalid Write Address: addr=0x%lx, size=%lu\n", pkt->getAddr(), pkt->getSize());
    }

    if(reg[0x0000] != 0){
        DPRINTF(PIMCTRL, "Trigger IRQ\n");
        reg[0x0000] = 0;
        raiseIrq();
    }
    if(reg[0x3010] != 0){ // 如果 CPU 写入了中断清除寄存器
        DPRINTF(PIMCTRL, "Clear IRQ\n");
        reg[0x3010] = 0;
        clearIrq();
    }
    
    pkt->makeResponse();
    return pioDelay;
}
} // namespace gem5