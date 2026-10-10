declare module "@levilamina" {

    export interface PlayerInfoEntry {
        uuid: string;
        xuid: string;
        name: string;
    }

    export class PlayerInfo {
        private constructor();

        static fromUuid(uuid: string): PlayerInfoEntry | null;

        static fromXuid(xuid: string): PlayerInfoEntry | null;

        static fromName(name: string): PlayerInfoEntry | null;
    }

}