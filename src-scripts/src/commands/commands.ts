import { CommandHandle, CommandParamKind, CommandRegistrar, logger, PlayerInfo } from "@levilamina";
import { CommandOrigin, CommandOriginType, CommandOutput } from "@minecraft";
import { tr } from "../i18n/i18n.js";
import { LandRegistry } from "@pland";

export default function initCommands() {
    const cmd = CommandRegistrar.getOrCreateCommand("pland");

    initAdminCommand(cmd);
}

function initAdminCommand(cmd: CommandHandle) {
    const adminOperationName = "pland-admin-operation";
    enum AdminOperation {
        add,
        remove,
    }

    if (!CommandRegistrar.hasEnum(adminOperationName)) {
        const r = [
            ["add", AdminOperation.add],
            ["remove", AdminOperation.remove],
        ];
        CommandRegistrar.tryRegisterRuntimeEnum(adminOperationName, r as [string, number][]);
    }

    type BatchOperation = [string, UUID][]; // name, uuid
    const doOperationImpl = (output: CommandOutput, oper: AdminOperation, batchs: BatchOperation) => {
        for (const [name, uuid] of batchs) {
            switch (oper) {
                case AdminOperation.add:
                    if (LandRegistry.isAdmin(uuid)) {
                        output.error(tr("command.admin.add.already", name));
                        continue;
                    }
                    if (LandRegistry.addAdmin(uuid)) {
                        output.success(tr("command.admin.add.success", name));
                    } else {
                        output.error(tr("command.admin.add.fail", name));
                    }
                    break;
                case AdminOperation.remove:
                    if (!LandRegistry.isAdmin(uuid)) {
                        output.error(tr("command.admin.remove.notAdmin", name));
                        continue;
                    }
                    if (LandRegistry.removeAdmin(uuid)) {
                        output.success(tr("command.admin.remove.success", name));
                    } else {
                        output.error(tr("command.admin.remove.fail", name));
                    }
                    break;
                default:
                    logger.error("Unknown admin operation: {}", oper);
            }
        }
    };

    // pland admin <add|remove> <name>
    cmd.runtimeOverload()
        .text("admin")
        .required("operation", CommandParamKind.Enum, adminOperationName)
        .required("name", CommandParamKind.String)
        .execute((origin, output, args) => {
            if (origin.originType !== CommandOriginType.DedicatedServer) {
                output.error(tr("command.admin.requrie.console.execute"));
                return;
            }
            const name = args.name;
            if (name.length == 0) {
                output.error(tr("command.admin.name.empty"));
                return;
            }

            const info = PlayerInfo.fromName(name);
            if (!info) {
                output.error(tr("command.admin.name.notFound", name));
                return;
            }

            doOperationImpl(output, args.operation as AdminOperation, [[info.name, info.uuid]]);
        });

    // pland admins <add|remove> <player>
    cmd.runtimeOverload()
        .text("admins")
        .required("operation", CommandParamKind.Enum, adminOperationName)
        .required("players", CommandParamKind.Player)
        .execute((origin, output, args) => {
            if (origin.originType !== CommandOriginType.DedicatedServer) {
                output.error(tr("command.admin.requrie.console.execute"));
                return;
            }

            if (!args.players.length) {
                output.error(tr("command.admin.players.empty"));
                return;
            }

            const batchs = args.players.map((p) => {
                return [p.realName, p.uuid];
            });
            doOperationImpl(output, args.operation as AdminOperation, batchs as BatchOperation);
        });

    // pland admin list
    cmd.runtimeOverload()
        .text("admin")
        .text("list")
        .execute((origin, output) => {
            if (origin.originType !== CommandOriginType.DedicatedServer) {
                output.error(tr("command.admin.requrie.console.execute"));
                return;
            }

            const admins = LandRegistry.getAdmins();
            if (admins.length == 0) {
                output.error(tr("command.admin.list.empty"));
                return;
            }

            output.success(tr("command.admin.list.count", admins.length));
            admins.forEach((a) => {
                const info = PlayerInfo.fromUuid(a);
                output.success(info?.name ?? a);
            });
        });
}
