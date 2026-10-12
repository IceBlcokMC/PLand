declare module "@minecraft" {

    export class Player {
        private constructor();

        static tryGet(uuid: UUID): Player | null;
        static tryGet(realName: string): Player | null;

        readonly isOperator: boolean;

        readonly localeCode: string;

        readonly xuid: string;

        readonly uuid: string;

        readonly realName: string;

        sendMessage(fmt: string, ...args: any): void;

        sendToast(title: string, content: string): void;

        setAbility(ability: AbilitiesIndex, value: boolean): void;

        canUseAbility(ability: AbilitiesIndex): boolean;
    }

}