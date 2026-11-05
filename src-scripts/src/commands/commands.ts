import { CommandRegistrar } from "@levilamina";
import initAdminSubCommand from "./sub/admin.js";

export default function initCommands() {
    const cmd = CommandRegistrar.getOrCreateCommand("pland");

    initAdminSubCommand(cmd);
}
