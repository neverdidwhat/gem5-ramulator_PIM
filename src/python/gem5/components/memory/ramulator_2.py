import configparser
import os
from typing import (
    List,
    Optional,
    Sequence,
    Tuple,
)

import m5
from m5.objects import (
    Ramulator2,
    MemCtrl,
)
from m5.params import (
    AddrRange,
    Port,
)
from m5.util.convert import toMemorySize

from ...utils.override import overrides
from ..boards.abstract_board import AbstractBoard
from .abstract_memory_system import AbstractMemorySystem




class RamulatorMemCtrl(Ramulator2):
    def __init__(self, config_path: str, range: Optional[str]) -> None:
        super().__init__(range = range)
        self.config_path = config_path


class SingleChannel(AbstractMemorySystem):
    """
    A Single Channel Memory system.
    """

    def __init__(self, config_path: str, size: Optional[str], range: Optional[str]):
        """
        :param mem_name: The name of the type  of memory to be configured.
        :param num_chnls: The number of channels.
        """
        super().__init__()
        self.mem_ctrl = RamulatorMemCtrl(config_path = config_path, range = range)
        self._size = toMemorySize(size)
        if not size:
            raise NotImplementedError(
                "Ramulator2 memory controller requires a size parameter."
            )

    @overrides(AbstractMemorySystem)
    def incorporate_memory(self, board: AbstractBoard) -> None:
        pass

    @overrides(AbstractMemorySystem)
    def get_mem_ports(self) -> Tuple[Sequence[AddrRange], Port]:
        return [(self.mem_ctrl.range, self.mem_ctrl.port)]

    @overrides(AbstractMemorySystem)
    def get_memory_controllers(self) -> List[MemCtrl]:
        return [self.mem_ctrl]

    @overrides(AbstractMemorySystem)
    def get_size(self) -> int:
        return self._size

    @overrides(AbstractMemorySystem)
    def set_memory_range(self, ranges: List[AddrRange]) -> None:
        if len(ranges) != 1 or ranges[0].size() != self._size:
            raise Exception(
                "Single channel Ramulator2 memory controller requires a single "
                "range which matches the memory's size."
            )
        self.mem_ctrl.range = ranges[0]
