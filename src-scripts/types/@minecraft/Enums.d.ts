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

}