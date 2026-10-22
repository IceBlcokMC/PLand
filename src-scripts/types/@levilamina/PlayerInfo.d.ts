declare module "@levilamina" {
    export interface PlayerInfoEntry {
        uuid: string;
        xuid: string;
        name: string;
    }

    export interface PlayerInfoAPI {
        fromUuid(uuid: string): PlayerInfoEntry | null;

        fromXuid(xuid: string): PlayerInfoEntry | null;

        fromName(name: string): PlayerInfoEntry | null;
    }

    export const PlayerInfo: PlayerInfoAPI;
}
