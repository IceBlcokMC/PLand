declare module "@levilamina" {
    import type {
        CommandOrigin,
        CommandOutput,
        CommandParameterOption,
        CommandPermissionLevel,
        Player,
    } from "@minecraft";

    export interface CommandRegistrarAPI {
        getOrCreateCommand(name: string, description?: string, level?: CommandPermissionLevel): CommandHandle;

        hasEnum(name: string): boolean;

        tryRegisterRuntimeEnum(name: string, values: pair<string, number>[]): boolean;

        addRuntimeEnumValues(name: string, values: pair<string, number>[]): boolean;

        hasSoftEnum(name: string): boolean;

        tryRegisterSoftEnum(name: string, values: string[]): boolean;

        addSoftEnumValues(name: string, values: string[]): boolean;

        removeSoftEnumValues(name: string, values: string[]): boolean;

        setSoftEnumValues(name: string, values: string[]): boolean;
    }
    export const CommandRegistrar: CommandRegistrarAPI;

    export class CommandHandle {
        private constructor();

        runtimeOverload(): RuntimeOverload<Params>;

        addAlias(name: string): CommandHandle;

        getAliases(): string[];
    }

    export enum CommandParamKind {
        Int = 0,
        Bool = 1,
        Float = 2,
        Dimension = 3,
        String = 4,
        Enum = 5,
        SoftEnum = 6,
        Actor = 7,
        Player = 8,
        BlockPos = 9,
        Vec3 = 10,
        RawText = 11,
        Message = 12,
        JsonValue = 13,
        Item = 14,
        BlockName = 15,
        BlockState = 16,
        Effect = 17,
        ActorType = 18,
        Command = 19,
        RelativeFloat = 20,
        IntegerRange = 21,
        FilePath = 22,
        WildcardInt = 23,
        WildcardActor = 24,
        Count = 25,
    }

    type KindMapping = {
        [CommandParamKind.Int]: number;
        [CommandParamKind.Float]: number;
        [CommandParamKind.Enum]: number;
        [CommandParamKind.SoftEnum]: string;
        [CommandParamKind.String]: string;
        [CommandParamKind.RawText]: string;
        [CommandParamKind.Bool]: boolean;
        [CommandParamKind.Player]: Player[];
        [CommandParamKind.BlockPos]: BlockPos;
        [CommandParamKind.Vec3]: Vec3;
    };

    type SupportedKind = Extract<CommandParamKind, keyof KindMapping>;
    type EnumTypeKind = CommandParamKind.Enum | CommandParamKind.SoftEnum;
    type NonEnumKind = Exclude<SupportedKind, EnumTypeKind>;

    type KindToType<K extends SupportedKind> = KindMapping[K];
    type KindToTypeOptional<K extends SupportedKind> = KindToType<K> | null;

    type Params = Record<string, any>;

    export class RuntimeOverload<T extends Params> {
        private constructor();

        optional<N extends string, K extends NonEnumKind>(
            name: N,
            kind: K,
        ): RuntimeOverload<T & { [P in N]?: KindToTypeOptional<K> }>;

        required<N extends string, K extends NonEnumKind>(
            name: N,
            kind: K,
        ): RuntimeOverload<T & { [P in N]: KindToType<K> }>;

        optional<N extends string, K extends EnumTypeKind>(
            name: N,
            enumKind: K,
            enumName: string,
        ): RuntimeOverload<T & { [P in N]?: KindToTypeOptional<K> }>;

        required<N extends string, K extends EnumTypeKind>(
            name: N,
            enumKind: K,
            enumName: string,
        ): RuntimeOverload<T & { [P in N]: KindToType<K> }>;

        text(text: string): this;

        postfix(postfix: string): this;

        option(option: CommandParameterOption): this;

        deoption(option: CommandParameterOption): this;

        execute(fn: (origin: CommandOrigin, output: CommandOutput, args: T) => void): void;
    }
}
