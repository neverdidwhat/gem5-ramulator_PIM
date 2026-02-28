from gem5.utils.requires import requires  
from gem5.components.boards.arm_board import ArmBoard  
# from gem5.components.memory.single_channel import DIMM_DDR5_8400  
from gem5.components.memory.ramulator_2 import SingleChannel
# from gem5.components.cachehierarchies.ruby.mesi_two_level_cache_hierarchy import MESITwoLevelCacheHierarchy  
from gem5.components.cachehierarchies.classic.private_l1_private_l2_cache_hierarchy import PrivateL1PrivateL2CacheHierarchy
from gem5.components.processors.simple_processor import SimpleProcessor  
from gem5.coherence_protocol import CoherenceProtocol  
from gem5.isas import ISA  
from gem5.components.processors.cpu_types import CPUTypes  

from gem5.resources.resource import obtain_resource, DiskImageResource, CheckpointResource
from gem5.simulate.simulator import Simulator  
from gem5.simulate.exit_event import ExitEvent  
from gem5.simulate.exit_handler import ExitHandler
from gem5.utils.override import overrides
from gem5.utils.requires import requires
import m5
import os
from m5.objects import VExpress_GEM5_V1_PIM


command = "echo 'Hello, ARM Ubuntu system is ready!';m5 checkpoint;"

requires(  
    isa_required=ISA.ARM,  
)  

# cache_hierarchy = MESITwoLevelCacheHierarchy(  
#     l1d_size="32KiB",  
#     l1d_assoc=8,  
#     l1i_size="32KiB",  
#     l1i_assoc=8,  
#     l2_size="256KiB",  
#     l2_assoc=16,  
#     num_l2_banks=1,  
# )  
cache_hierarchy = PrivateL1PrivateL2CacheHierarchy(  
    l1d_size="32KiB",  
    l1i_size="32KiB",  
    l2_size="256KiB",  
)


# memory = DIMM_DDR5_8400(size="2GiB") 
memory = SingleChannel(config_path="/home/fjc/project/Ramulator_LPDDR6_PIM_V2_copy/example_config_LPDDR6_PIM_gem5.yaml", size="256MiB", range="256MiB") 


processor = SimpleProcessor(  
    cpu_type=CPUTypes.TIMING,        
    num_cores=1,
    isa=ISA.ARM
)  


board = ArmBoard(  
    clk_freq="3GHz",  
    processor=processor,  
    memory=memory,  
    cache_hierarchy=cache_hierarchy,  
    platform=VExpress_GEM5_V1_PIM()
)  
ramu = board.memory.mem_ctrl
board._platform.pimctrl.ramulator2_f = ramu


# disk_image = obtain_resource("arm64-ubuntu-18.04-img", resource_directory="/home/fjc/project/gem5_imgs/" , download_md5_mismatch=False)
disk_image = DiskImageResource(local_path="/home/fjc/project/gem5_imgs/arm64-ubuntu-18.04-img-1.0.0", root_partition="1")
board.set_kernel_disk_workload(  
    kernel=obtain_resource("arm64-linux-kernel-5.4.49", resource_directory="/home/fjc/project/gem5_imgs/", download_md5_mismatch=False),      
    disk_image=disk_image,    
    bootloader=obtain_resource("arm64-bootloader-foundation"), # 必须添加 Bootloader        
    # checkpoint=CheckpointResource(local_path="/home/pqr/gem5_outputs/ramulator_arm_fs/cpt.302319543168"),
    readfile="/home/fjc/project/gem5_ramulator/Pim_test/pim_api_v2"         
)  




class CustomKernelBootedExitHandler(ExitHandler, hypercall_num=1):
    @overrides(ExitHandler)
    def _process(self, simulator: "Simulator") -> None:
        print("First exit: kernel booted")

    @overrides(ExitHandler)
    def _exit_simulation(self) -> bool:
        return False


class CustomAfterBootExitHandler(ExitHandler, hypercall_num=2):
    @overrides(ExitHandler)
    def _process(self, simulator: "Simulator") -> None:
        print("Second exit: after boot command completed")

    @overrides(ExitHandler)
    def _exit_simulation(self) -> bool:
        return False


class AfterBootScriptExitHandler(ExitHandler, hypercall_num=3):
    @overrides(ExitHandler)
    def _process(self, simulator: "Simulator") -> None:
        print(f"Third exit: {self.get_handler_description()}")

    @overrides(ExitHandler)
    def _exit_simulation(self) -> bool:
        return False


print(board._platform.pimctrl.pio_addr)  # 输出 pimctrl 设备的 pio_addr 地址，验证配置是否生效

simulator = Simulator(  
    board=board,
    outdir="/home/fjc/gem5_outputs/ramulator_arm_fs",
)  
# print(simulator.get_checkpoint_dir())

print("Starting simulation with gem5 25.1.0.0...")
simulator.run()