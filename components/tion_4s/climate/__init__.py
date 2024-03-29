from esphome.cpp_types import PollingComponent
from esphome.components import climate

from ... import tion  # pylint: disable=relative-beyond-top-level
from .. import (
    tion_4s_schema,
    setup_tion_4s,
)  # pylint: disable=relative-beyond-top-level

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["tion_4s"]

Tion4sClimate = tion.tion_ns.class_("Tion4sClimate", PollingComponent, climate.Climate)

CONFIG_SCHEMA = tion_4s_schema(Tion4sClimate, climate.CLIMATE_SCHEMA)


async def to_code(config):
    """Code generation entry point"""
    # pylint: disable=unused-variable
    var = await setup_tion_4s(config, climate.register_climate)
