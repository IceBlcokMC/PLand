import { CommandRegistrar } from "@levilamina";
import initAdminSubCommand from "./sub/admin.js";
import initInfoSubCommand from "./sub/info.js";

export default function initCommands() {
    const cmd = CommandRegistrar.getOrCreateCommand("pland");

    initAdminSubCommand(cmd);
    initInfoSubCommand(cmd);
}
