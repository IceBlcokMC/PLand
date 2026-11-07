declare module "@minecraft" {
    // 底层未对 Actor 进行绑定独立类，仅 Player 绑定声明了继承关系，故这里声明为接口
    interface Actor {
        readonly dimensionId: DimensionType;

        getPosition(): Vec3;

        teleport(pos: Vec3, dimension: DimensionType): void;
    }

    export class Player implements Actor {
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

        readonly dimensionId: DimensionType;

        getPosition(): Vec3;

        teleport(pos: Vec3, dimension: DimensionType): void;
    }
}
