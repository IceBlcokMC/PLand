declare module "@pland" {
    export class Land {
        constructor();
        constructor(aabb: LandAABB, dimension: number, is3D: boolean, owner: UUID);

        /** 领地唯一 ID */
        readonly id: LandID;
        /** 是否为3D领地 */
        readonly is3D: boolean;
        /** 领地所在维度 ID */
        readonly dimensionId: number;
        /** 是否已注册到 LandRegistry */
        readonly isRegistered: boolean;
        /** 判断当前领地是否为无主领地 */
        readonly isOwnerless: boolean;
        /** 判断当前领地是否由玩家持有 */
        readonly isPlayerOwned: boolean;
        /** 判断当前领地是否为系统所有(租赁欠费回收) */
        readonly isSystemOwned: boolean;
        /** 是否为买断制领地 */
        readonly isBought: boolean;
        /** 是否为租赁领地 */
        readonly isLeased: boolean;
        /** 租期是否有效 */
        readonly isLeaseActive: boolean;
        /** 租赁是否冻结(未续租缓冲期) */
        readonly isLeaseFrozen: boolean;
        /** 租赁是否过期 */
        readonly isLeaseExpired: boolean;
        /** @deprecated 当前领地是否来自其它插件导入转换 */
        readonly isConvertedLand: boolean;
        /** @deprecated 领地主人数据是否以 XUID 存储 */
        readonly isOwnerDataIsXUID: boolean;
        /** 当前领地是否有父领地 */
        readonly hasParentLand: boolean;
        /** 当前领地名下是否有子领地 */
        readonly hasSubLand: boolean;
        /** 当前领地是否为子领地(有父领地、无子领地) */
        readonly isSubLand: boolean;
        /** 是否为父领地(有子领地、无父领地) */
        readonly isParentLand: boolean;
        /** 是否为混合领地(有父领地、有子领地) */
        readonly isMixLand: boolean;
        /** 是否为普通领地(无父领地、无子领地) */
        readonly isOrdinaryLand: boolean;
        /** 是否可以创建子领地 */
        readonly canCreateSubLand: boolean;

        /**
         * 获取领地 AABB 范围
         * @warning 此方法返回的对象为只读对象，不可修改，否则会抛出异常
         */
        getAABB(): LandAABBView;

        /**
         * 获取领地传送点
         */
        getTeleportPos(): BlockPos;

        /**
         * 设置领地传送点
         */
        setTeleportPos(pos: BlockPos): boolean;

        /**
         * 获取领地主人的 UUID
         *
         * NOTE: 两种归属类型都会返回 `mce::UUID::EMPTY()`，请用 `getOwnershipKind()` 区分：
         *       - Ownerless: 无主领地
         *       - PendingMigration: 旧数据，主人数据仍为 XUID，待主人上线后自动迁移
         *       如需访问原始存储值（可能是 XUID 或 UUID 字符串），请使用 `getRawOwner()`。
         */
        getOwner(): UUID;

        /**
         * 设置领地主人的 UUID
         *
         * NOTE: 归属类型由传入的 UUID 派生（空 UUID => Ownerless，系统账号 => System，其余 => Player），
         *       本函数是归属状态的唯一写入点，不要单独修改归属类型。
         */
        setOwner(owner: UUID): void;

        /**
         * @brief 获取领地归属类型
         */
        getOwnershipKind(): LandOwnershipKind;

        /**
         * 获取存储的领地原始主人数据
         * @note 此函数可能返回 XUID 或 UUID 字符串，也有可能为空，具体请根据 LandOwnershipKind 判断
         * @deprecated 除非有特殊需求，否则推荐使用 `getOwner()` 获取领地主人的 UUID
         */
        getRawOwner(): string;

        /**
         * @brief 获取上一任领地主
         *
         * NOTE: 返回值是原始存储值(可能是 XUID)，仅在归属发生变化时更新，可用于审计与撤销
         * @return 从未发生过归属变更时为空
         */
        getPreviousOwner(): optional<string>;

        /**
         * 获取领地成员列表
         */
        getMembers(): UUID[];

        /**
         * 添加领地成员
         * @param member 要添加的领地成员 UUID
         * @return 是否添加成功
         * @note 此函数拒绝添加 Owner 为 Member
         */
        addLandMember(member: UUID): boolean;

        /**
         * 移除领地成员
         */
        removeLandMember(member: UUID): boolean;

        /**
         * 清空领地成员列表
         */
        clearMembers(): void;

        /**
         * 获取领地名称
         */
        getName(): string;

        /**
         * 设置领地名称
         */
        setName(name: string): void;

        /**
         * 获取领地原始购买价格
         */
        getOriginalBuyPrice(): number;

        /**
         * 设置领地原始购买价格
         */
        setOriginalBuyPrice(price: number): void;

        /**
         * 获取领地持有类型
         */
        getHoldType(): LandHoldType;

        /**
         * 获取租赁状态
         */
        getLeaseState(): LeaseState;

        /**
         * 获取租赁开始时间(UNIX 时间戳)
         */
        getLeaseStartAt(): bigint;

        /**
         * 获取租赁结束时间(UNIX 时间戳)
         */
        getLeaseEndAt(): bigint;

        /**
         * @brief 判断 uuid 是否为领地主
         *
         * NOTE: 空 UUID 永远不是领地主，无主领地的归属请用 `isOwnerless()` 判定
         */
        isOwner(owner: UUID): boolean;

        /**
         * 判断 uuid 是否为领地成员
         */
        isMember(member: UUID): boolean;

        /**
         * 对领地进行碰撞检测(半径)
         */
        isCollision(pos: BlockPos, radius: number): boolean;

        /**
         * @brief 获取领地类型
         */
        getType(): LandType;

        /**
         * @brief 获取父领地 ID
         */
        getParentLandID(): LandID;

        /**
         * @brief 获取子领地 ID (当前领地名下的所有子领地)
         */
        getSubLandIDs(): LandID[];

        /**
         * @brief 获取嵌套层级(相对于父领地)
         */
        getNestedLevel(): number;

        /**
         * @brief 获取玩家在当前领地的有效角色
         * @return 领地冻结时返回 Actor，否则根据权限返回对应角色
         */
        getEffectiveRole(player: UUID): LandRole;

        /**
         * 判断领地实例是否为同一个领地(相等比较)
         */
        equals(other: Land): boolean;
    }
}
