#include "pim_ctrl.hh"
#include "mem/ramulator2.hh"
#include "base/trace.hh"    // 用于调试 DPRINTF
#include "debug/PIMCTRL.hh"
#include "base/logging.hh"  // 用于 panic/warn 断言
#include "mem/packet.hh"   // 用于 PacketPtr
#include <vector>           // 存储输入向量数据

namespace gem5
{
// 构造函数实现：必须透传参数给基类 BasicPioDevice
PimCtrl::PimCtrl(const PimCtrlParams &p)
    :gemv_req(new GemvRequest()),
     ramulator2_ptr(p.ramulator2_f),
     irq(p.interrupt->get()), pending(false),
     BasicPioDevice(p, p.pio_size)
    
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
    uint64_t is_valid = 0;

    Addr offset = pkt->getAddr() - pioAddr;
    if(pkt->getAddr() >= pioAddr && pkt->getAddr() + pkt->getSize() <= pioAddr + pioSize){
        pkt->writeData(reg + offset);
        DPRINTF(PIMCTRL, "PimCtrl write offset 0x%x: data=0x%lx\n", (reg + offset), pkt->getUintX(ByteOrder::little));
    }else{
        DPRINTF(PIMCTRL, "PimCtrl: Invalid Write Address: addr=0x%lx, size=%lu\n", pkt->getAddr(), pkt->getSize());
    }
    uint64_t* cmd_addr = (uint64_t*)(reg + REG_CMD);
    is_valid = *cmd_addr;
    DPRINTF(PIMCTRL, "PimCtrl: Current GEMV request valid status: %lu\n", is_valid);
    //触发GEMV请求
    if(is_valid == 1) {
        sendGemvRequest();
        *cmd_addr = 0;
    }
    
    is_valid = 0;

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


// 发送GEMV请求（核心逻辑，后续可对接DDR控制器/计算单元）
void PimCtrl::sendGemvRequest() {
    gemv_req->weight_addr = * (uint64_t*) (reg + REG_WEIGHT_ADDR);
    gemv_req->rank_num = * (uint8_t*) (reg + REG_RANK_NUM);
    gemv_req->k = * (uint32_t*) (reg + REG_MATRIX_K);
    gemv_req->n = * (uint32_t*) (reg + REG_MATRIX_N);
    gemv_req->input_prec = * (uint8_t*) (reg + REG_INPUT_PREC);
    gemv_req->weight_prec = * (uint8_t*) (reg + REG_WEIGHT_PREC);
    gemv_req->scale_prec = * (uint8_t*) (reg + REG_SCALE_PREC);
    gemv_req->is_valid = true;

    inform("=====================================\n");
    inform("PimCtrl: Sending GEMV Request:\n");
    inform("PimCtrl: GEMV Request Type: %u\n", gemv_req->req_type);
    inform("  Weight Address: 0x%lx\n", gemv_req->weight_addr);
    inform("  Rank Number: %u\n", gemv_req->rank_num);
    inform("  Matrix Size: K=%u, N=%u\n", gemv_req->k, gemv_req->n);
    inform("  Precisions: Input(INT%u), Weight(INT%u), Scale(INT%u)\n",
           gemv_req->input_prec, gemv_req->weight_prec, gemv_req->scale_prec);
    inform("  Input Vector Length: %u (INT8)\n", (gemv_req->k * gemv_req->input_prec));
    inform("  First 4 vector values: %d, %d, %d, %d\n",
           * (uint8_t*) (reg + REG_INPUT_DATA), * (uint8_t*) (reg + REG_INPUT_DATA + 8),* (uint8_t*) (reg + REG_INPUT_DATA + 8*2), * (uint8_t*) (reg + REG_INPUT_DATA + 8*3));
    inform("=====================================\n");

    // TODO: 此处添加请求发送逻辑
    // 1. 对接DDR控制器：读取权重矩阵数据
    // 2. 对接计算单元：传入输入向量+权重+参数执行GEMV
    // 3. 发送响应：将计算结果写回指定地址
    ramulator2_ptr->recvGemvRequest(*gemv_req);
    inform("pim_ctrl send gemv request success!\n");
    
}
} // namespace gem5