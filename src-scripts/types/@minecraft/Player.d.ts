declare module "@minecraft" {

    export class Player {
        readonly localeCode: string;
        readonly xuid: string;
        readonly uuid: string;
        readonly realName: string;

        private constructor();

        static tryGet(realName: string): Player | null;

    }

}