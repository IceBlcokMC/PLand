declare module "@pland" {
    export type LandID = bigint;

    export const INVALID_LAND_ID: LandID;

    export const SYSTEM_ACCOUNT_UUID_STR: UUID;

    export enum LandOwnershipKind {
        Player = 0, // 领地主为玩家 UUID
        PendingMigration = 1, // 领地主为 XUID 字符串，主人上线后自动迁移为 Player
        System = 2, // 系统所有，租赁欠费回收后归系统账号
        Ownerless = 3, // 无主领地，由领地管理员设置，领地主为空 UUID
    }

    export enum LandHoldType {
        Bought = 0, // 购买模式
        Leased = 1, // 租赁模式
    }

    export enum LeaseState {
        None = 0, // 非租赁领地 / 无状态
        Active = 1, // 正常期
        Frozen = 2, // 冻结期
        Expired = 3, // 已到期(已回收)
    }

    export enum LandType {
        Ordinary = 0, // 普通领地(无父、无子)
        Parent = 1, // 父领地(无父、有子)
        Mix = 2, // 混合领地(有父、有子)
        Sub = 3, // 子领地(有父、无子)
    }

    export enum LandRole {
        Admin = 0, // 管理员
        Owner = 1, // 领地主人
        Member = 2, // 领地成员

        // Actor includes both non-member players and non-player entities (e.g., Mobs, TNT).
        Actor = 3, // 实体
    }

    export enum LandRecycleReason {
        LeaseExpired = 0, // 租赁到期
        Inactivity = 1, // 闲置
        ForceRecycle = 2, // 强制回收
    }
}
