#include "pim_ctrl.hh"
#include "base/trace.hh"    // 用于调试 DPRINTF


namespace gem5
{
// 构造函数实现：必须透传参数给基类 BasicPioDevice
PimCtrl::PimCtrl(const PimCtrlParams &p)
    : BasicPioDevice(p, p.pio_size) 
{
}

Tick PimCtrl::read(PacketPtr pkt) {
    // 必须处理读请求，否则系统会 Crash
    pkt->makeResponse();
    pkt->setUintX(0, ByteOrder::little); // 默认读回 0
    return pioDelay;
}

Tick PimCtrl::write(PacketPtr pkt) {
    Addr offset = pkt->getAddr() - pioAddr;
    uint64_t data = pkt->getUintX(ByteOrder::little);
    
    if (offset == 0x10) { 
        // LazyMan 推荐使用 DPRINTF 代替 printf，这样可以用 --debug-flags 开启
        // 如果想看打印，必须在编译后运行命令加 --debug-flags=PimCtrl
        inform("PimCtrl: GEMV Request Received: data=%lu\n", data);
    }
    
    pkt->makeResponse();
    return pioDelay;
}
} // namespace gem5