declare module "@pland" {
    interface EconomyAPI {
        /** 经济系统是否启用 */
        readonly isEnabled: boolean;

        /** 货币名称 */
        readonly currencyName: string;

        /** 获取玩家余额 */
        get(uid: UUID): bigint;

        /** 设置玩家余额 */
        set(uid: UUID, amount: bigint | number): boolean;

        /** 增加玩家余额 */
        add(uid: UUID, amount: bigint | number): boolean;

        /** 扣除玩家余额 */
        reduce(uid: UUID, amount: bigint | number, allowNegative: boolean): boolean;

        /** 转账 */
        transfer(from: UUID, to: UUID, amount: bigint | number, allowNegative: boolean): boolean;
    }

    /**
     * 经济系统API
     * @note 底层根据配置自动处理不同后端(由领地系统配置决定)
     * @note 此接口仅支持操作在线玩家, 不支持离线玩家
     */
    export const economy: EconomyAPI;
}
