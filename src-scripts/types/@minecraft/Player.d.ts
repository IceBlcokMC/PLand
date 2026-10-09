declare module "@minecraft" {

    export class Player {
        readonly isOperator: boolean;

        readonly localeCode: string;

        readonly xuid: string;

        readonly uuid: string;

        readonly realName: string;

        private constructor();

        static tryGet(uuid: UUID): Player | null;

        sendMessage(fmt: string, ...args: any): void;

        static tryGet(realName: string): Player | null;

        sendToast(title: string, content: string): void;

    }

}