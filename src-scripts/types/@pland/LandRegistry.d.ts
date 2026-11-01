declare module "@pland" {
    export interface LandRegistryAPI {
        /** 判断指定玩家是否为领地管理员。 */
        isAdmin(uuid: UUID): boolean;

        /**
         * 将玩家添加为领地管理员。
         * @returns 是否成功添加。
         */
        addAdmin(uuid: UUID): boolean;

        /**
         * 移除玩家的领地管理员身份。
         * @returns 是否成功移除。
         */
        removeAdmin(uuid: UUID): boolean;

        /** 获取当前所有领地管理员的 UUID。 */
        getAdmins(): UUID[];

        /** 判断指定 ID 的领地是否已注册。 */
        hasLand(id: LandID): boolean;

        /**
         * 根据 ID 获取领地。
         * @returns 找到的领地；不存在时返回 null。
         */
        getLandById(id: LandID): Land | null;

        /**
         * 根据多个 ID 批量获取领地。
         * @returns 找到的领地集合，不存在的 ID 不会产生对应结果。
         */
        getLandByIds(ids: LandID[]): Land[];

        /**
         * 获取指定玩家拥有的领地。
         * @param includeShared 是否同时包含该玩家作为共享成员的领地。
         */
        getLandsByOwner(owner: UUID, includeShared: boolean): Land[];

        /**
         * 获取指定玩家在指定维度拥有的领地。
         * @param dimension 目标维度。
         */
        getLandsByOwnerInDimension(owner: UUID, dimension: DimensionType): Land[];

        /**
         * 获取各领地主人的领地数量。
         * @returns [主人 UUID, 领地数量] 的数组。
         */
        getOwnerLandCounts(): [UUID, number][];

        /**
         * 获取玩家在指定领地上的最终生效角色。
         * 该结果会综合领地归属、成员权限、领地状态以及管理员身份等因素。
         * @param uuid 玩家 UUID。
         * @param id 目标领地 ID；使用 INVALID_LAND_ID 时表示不指定具体领地。
         * @param includeOperator 是否将管理员身份纳入角色判定。
         */
        getEffectiveRole(uuid: UUID, id: LandID, includeOperator: boolean): LandRole;

        /**
         * 查询指定坐标所在的领地。
         * @param pos 查询坐标。
         * @param dimension 坐标所属维度。
         * @returns 包含该坐标的领地；不存在时返回 null。
         */
        getLandAtPosition(pos: BlockPos, dimension: DimensionType): Land | null;

        /**
         * 查询指定坐标周围一定范围内的领地。
         * 查询范围以 pos 为中心，按 radius 扩展；二维领地和三维领地的高度判定遵循各自的边界规则。
         * @param pos 查询中心坐标。
         * @param radius 查询半径。
         * @param dimension 查询维度。
         */
        getLandsInRadius(pos: BlockPos, radius: number, dimension: DimensionType): Land[];

        /**
         * 查询与指定边界范围相交的领地。
         * @param a 查询范围的一个端点。
         * @param b 查询范围的另一个端点。
         * @param dimension 查询维度。
         */
        getLandsIntersectingBounds(a: BlockPos, b: BlockPos, dimension: DimensionType): Land[];
    }
    export const LandRegistry: LandRegistryAPI;
}
