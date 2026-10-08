declare module "@levilamina" {

    import type {
        CommandOrigin,
        CommandOutput,
        CommandParameterOption,
        CommandPermissionLevel,
        Player
    } from "@minecraft";

    export class CommandRegistrar {
        private constructor();

        static getInstance(): CommandRegistrar;

        getOrCreateCommand(
            name: string,
            description?: string,
            level?: CommandPermissionLevel
        ): CommandHandle

        hasEnum(name: string): boolean;

        tryRegisterRuntimeEnum(
            name: string,
            values: pair<string, number>[]
        ): boolean;

        addRuntimeEnumValues(
            name: string,
            values: pair<string, number>[]
        ): boolean;

        hasSoftEnum(name: string): boolean;

        tryRegisterSoftEnum(
            name: string,
            values: string[]
        ): boolean;

        addSoftEnumValues(
            name: string,
            values: string[]
        ): boolean;

        removeSoftEnumValues(
            name: string,
            values: string[]
        ): boolean;

        setSoftEnumValues(
            name: string,
            values: string[]
        ): boolean;
    }

    export class CommandHandle {
        private constructor();

        runtimeOverload(): RuntimeOverload<Params>;

        addAlias(name: string): CommandHandle

        getAliases(): string[]
    }

    export enum CommandParamKind {
        Int,
        Bool,
        Float,
        Dimension,
        String,
        Enum,
        SoftEnum,
        Actor,
        Player,
        BlockPos,
        Vec3,
        RawText,
        Message,
        JsonValue,
        Item,
        BlockName,
        BlockState,
        Effect,
        ActorType,
        Command,
        RelativeFloat,
        IntegerRange,
        FilePath,
        WildcardInt,
        WildcardActor,
        // New types can only be added here, to keep the ABI stable.
        Count,
    }

    type KindMapping = {
        [CommandParamKind.Int]: number
        [CommandParamKind.Float]: number
        [CommandParamKind.Enum]: number
        [CommandParamKind.SoftEnum]: string
        [CommandParamKind.String]: string
        [CommandParamKind.RawText]: string
        [CommandParamKind.Bool]: boolean
        [CommandParamKind.Player]: Player[]
        [CommandParamKind.BlockPos]: BlockPos
        [CommandParamKind.Vec3]: Vec3
    }

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
            kind: K
        ): RuntimeOverload<T & { [P in N]?: KindToTypeOptional<K> }>;

        required<N extends string, K extends NonEnumKind>(
            name: N,
            kind: K
        ): RuntimeOverload<T & { [P in N]: KindToType<K> }>;

        optional<N extends string, K extends EnumTypeKind>(
            name: N,
            enumKind: K,
            enumName: string
        ): RuntimeOverload<T & { [P in N]: KindToTypeOptional<K> }>;

        required<N extends string, K extends EnumTypeKind>(
            name: N,
            enumKind: K,
            enumName: string
        ): RuntimeOverload<T & { [P in N]: KindToType<K> }>;

        text(
            text: string
        ): this;

        postfix(
            postfix: string
        ): this;

        option(
            option: CommandParameterOption
        ): this;

        deoption(
            option: CommandParameterOption
        ): this;

        execute(
            fn: (origin: CommandOrigin, output: CommandOutput, args: T) => void
        ): void;

    }

}