from m5.params import *
from m5.objects.Device import BasicPioDevice
from m5.objects.Gic import ArmSPI

class PimCtrl(BasicPioDevice):
    type = 'PimCtrl'
    cxx_header = "dev/pim_ctrl/pim_ctrl.hh"
    cxx_class = "gem5::PimCtrl"
    #表示设备占用的内存大小
    pio_size = Param.Addr(0x4000, "Device window size")
    # pio_addr = Param.Addr(0x2E010000, "Device pio address")
    interrupt = Param.ArmInterruptPin(
        ArmSPI(num=108), "Interrupt that connects to GIC"
    )
    ramulator2_f = Param.Ramulator2("Reference to Ramulator2 for sending GEMV requests")

    def generateDeviceTree(self, state):
        """
        生成设备树节点。

        目标：为 OS/驱动描述一个最小且完整的 MMIO 设备节点。
        """
        # 1) 生成通用 MMIO 设备节点（包含 reg：地址+大小）
        node = self.generateBasicPioDeviceNode(
            state,
            "pimctrl",          # 设备节点名（设备树中的节点前缀）
            self.pio_addr,      # MMIO 基地址
            self.pio_size,      # MMIO 窗口大小
            interrupts=[self.interrupt],  # 中断号列表
        )

        # 2) 添加兼容字符串，供内核/驱动匹配（简洁且明确）
        node.appendCompatible(["gem5,pim-ctrl"])

        # 3) 返回节点（使用 yield 以匹配 gem5 设备树生成约定）
        yield node