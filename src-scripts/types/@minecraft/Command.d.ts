declare module "@minecraft" {

    export class CommandOrigin {
        readonly originType: CommandOriginType
        readonly blockPosition: BlockPos
        readonly worldPosition: Vec3

        private constructor();
    }

    export enum CommandOriginType {
        Player = 0,
        CommandBlock = 1,
        MinecartCommandBlock = 2,
        DevConsole = 3,
        Test = 4,
        AutomationPlayer = 5,
        ClientAutomation = 6,
        DedicatedServer = 7,
        Entity = 8,
        Virtual = 9,
        GameArgument = 10,
        EntityServer = 11,
        Precompiled = 12,
        GameDirectorEntityServer = 13,
        Scripting = 14,
        ExecuteContext = 15,
    }

    export class CommandOutput {
        private constructor();

        success(fmt: string, ...args: any): void;

        error(fmt: string, ...args: any): void;
    }

}