from gem5.coherence_protocol import CoherenceProtocol
from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.cachehierarchies.classic.private_l1_cache_hierarchy import PrivateL1CacheHierarchy
# from gem5.components.memory.dramsim_3 import SingleChannel
from gem5.components.memory.ramulator_2 import SingleChannel
from gem5.components.processors.cpu_types import CPUTypes
from gem5.components.processors.simple_processor import (
    SimpleProcessor,
)
from gem5.isas import ISA
from gem5.resources.resource import obtain_resource
from gem5.simulate.exit_event import ExitEvent
from gem5.simulate.simulator import Simulator
from gem5.utils.requires import requires
from gem5.resources.resource import BinaryResource

requires(
    isa_required=ISA.ARM,
    coherence_protocol_required=CoherenceProtocol.MESI_TWO_LEVEL,
)

cache_hierarchy = PrivateL1CacheHierarchy(
    l1d_size="128KiB",
    l1i_size="128KiB",
)

# memory = SingleChannelDDR3_1600(size="2GiB")
memory = SingleChannel(config_path="/home/pqr/project/Ramulator_LPDDR6_PIM/example_config_LPDDR6_PIM_gem5.yaml", size="256MiB", range="256MiB")

processor = SimpleProcessor(
    cpu_type=CPUTypes.TIMING,
    isa=ISA.ARM,
    num_cores=1,
)

board = SimpleBoard(
    clk_freq="3GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy,
)


# binary_resource = BinaryResource(local_path="tests/test-progs/hello/bin/arm/linux/hello")
binary_resource = BinaryResource(local_path="/home/pqr/project/gem5/tests/test-progs/s1mple/src/s1mple_arm64")
board.set_se_binary_workload(binary=binary_resource)



simulator = Simulator(
    board=board,
)
simulator.run()