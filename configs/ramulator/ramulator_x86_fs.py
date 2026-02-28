from gem5.utils.requires import requires  
from gem5.components.boards.x86_board import X86Board  
from gem5.components.memory.single_channel import SingleChannelDDR3_1600  
# from gem5.components.cachehierarchies.ruby.mesi_two_level_cache_hierarchy import MESITwoLevelCacheHierarchy  
from gem5.components.cachehierarchies.classic.private_l1_shared_l2_cache_hierarchy import PrivateL1SharedL2CacheHierarchy
from gem5.components.processors.simple_switchable_processor import SimpleSwitchableProcessor  
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


command = "echo 'Hello, ARM Ubuntu system is ready!';"

requires(  
    isa_required=ISA.X86,  
    coherence_protocol_required=CoherenceProtocol.MESI_TWO_LEVEL,  
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
cache_hierarchy = PrivateL1SharedL2CacheHierarchy(  
    l1d_size="32KiB",  
    l1i_size="32KiB",  
    l2_size="256KiB",  
    l1d_assoc=8,  
    l1i_assoc=8,  
    l2_assoc=16,  
)


memory = SingleChannelDDR3_1600(size="2GiB")  


processor = SimpleSwitchableProcessor(  
    starting_core_type=CPUTypes.ATOMIC, 
    switch_core_type=CPUTypes.O3,       
    num_cores=2,
    isa=ISA.X86
)  


board = X86Board(  
    clk_freq="3GHz",  
    processor=processor,  
    memory=memory,  
    cache_hierarchy=cache_hierarchy,  
)  



# workload = obtain_resource(
#     "x86-ubuntu-24.04-boot-with-systemd", resource_version="5.0.0", resource_directory="/home/fjc/project/gem5_imgs/", download_md5_mismatch=False
# )
# board.set_workload(workload)

# disk_image = obtain_resource("x86-ubuntu-24.04-img", resource_directory="/home/fjc/project/gem5_imgs/" , download_md5_mismatch=False)
disk_image = DiskImageResource(local_path="/home/fjc/project/gem5_imgs/x86-ubuntu-24.04-img-4.0.0")
board.set_kernel_disk_workload(  
    kernel=obtain_resource("x86-linux-kernel-6.8.0-52-generic", resource_directory="/home/fjc/project/gem5_imgs/", download_md5_mismatch=False),      
    disk_image=disk_image,    
    # bootloader=obtain_resource("arm64-bootloader-foundation"), # 必须添加 Bootloader
    kernel_args=[
        "earlyprintk=ttyS0",
        "console=ttyS0",
        "lpj=7999923",
        "root=/dev/sda2"
    ],
    readfile_contents=command,            
    # checkpoint=CheckpointResource(local_path="/home/pqr/gem5_checkpoints")          
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
        simulator.switch_processor()

    @overrides(ExitHandler)
    def _exit_simulation(self) -> bool:
        return False


class AfterBootScriptExitHandler(ExitHandler, hypercall_num=3):
    @overrides(ExitHandler)
    def _process(self, simulator: "Simulator") -> None:
        print(f"Third exit: {self.get_handler_description()}")
        m5.checkpoint(os.path.join("/home/pqr/gem5_checkpoints"))
        print("Checkpoint done.")

    @overrides(ExitHandler)
    def _exit_simulation(self) -> bool:
        return False




simulator = Simulator(  
    board=board
)  
# print(simulator.get_checkpoint_dir())

print("Starting simulation with gem5 25.1.0.0...")
simulator.run()