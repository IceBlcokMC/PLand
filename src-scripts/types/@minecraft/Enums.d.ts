declare module "@minecraft" {

    // mc/server/commands/CommandPermissionLevel.h
    export enum CommandPermissionLevel {
        Any = 0,
        GameDirectors = 1,
        Admin = 2,
        Host = 3,
        Owner = 4,
        Internal = 5,
    }

    export enum CommandParameterOption {
        None = 0,
        EnumAutocompleteExpansion = 1,
        HasSemanticConstraint = 2,
        EnumAsChainedCommand = 4,
    }

    export enum ModalFormCancelReason {
        UserClosed = 0,
        UserBusy = 1,
    }

    export enum AbilitiesIndex {
        Invalid = -1,
        Build = 0,
        Mine = 1,
        DoorsAndSwitches = 2,
        OpenContainers = 3,
        AttackPlayers = 4,
        AttackMobs = 5,
        OperatorCommands = 6,
        Teleport = 7,
        Invulnerable = 8,
        Flying = 9,
        MayFly = 10,
        Instabuild = 11,
        Lightning = 12,
        FlySpeed = 13,
        WalkSpeed = 14,
        Muted = 15,
        WorldBuilder = 16,
        NoClip = 17,
        PrivilegedBuilder = 18,
        VerticalFlySpeed = 19,
        AbilityCount = 20,
    }
}