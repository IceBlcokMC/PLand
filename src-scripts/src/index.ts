import { logger } from "@levilamina";
import { initI18n, tr } from "./i18n/i18n.js";
import initCommands from "./commands/commands.js";

export default class Entry implements PluginEntry {
    constructor() {}

    onLoad() {
        logger.debug("pland script logic loading...");

        initI18n();
        logger.debug(tr("debug.load.i18n.loaded"));
    }

    onEnable() {
        initCommands();
        logger.debug(tr("debug.enable.command.registered"));
    }

    onDisable() {}
}
