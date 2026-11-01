declare module "@pland" {
    export class LandAABB {
        constructor(min: BlockPos, max: BlockPos);

        min: BlockPos;
        max: BlockPos;

        /** X 轴坐标跨度 (depth) */
        readonly spanX: number;
        /** Z 轴坐标跨度 (width) */
        readonly spanY: number;
        /** Y 轴坐标跨度 (height) */
        readonly spanZ: number;

        // AABB 区域内各轴包含的方块数量
        readonly blockCountX: number;
        readonly blockCountY: number;
        readonly blockCountZ: number;

        /** AABB底面积（X-Z平面方块总数） */
        readonly square: bigint;
        /** AABB总体积（总方块数） */
        readonly volume: bigint;

        /**
         * Ensures the AABB is in canonical form (min <= max per axis).
         * Does not alter the actual bounding volume.
         */
        canonicalize(): void;

        toString(): string;

        hasPos(pos: BlockPos, includeY: boolean): boolean;

        /**
         * @brief 扩展 AABB 边界
         */
        expanded(spacing: number, includeY: boolean): LandAABB;

        /**
         * @brief 判断某个pos是否在领地内边界
         */
        isOnInnerBoundary(pos: BlockPos): boolean;

        /**
         * @brief 判断某个pos是否在领地外边界
         */
        isOnOuterBoundary(pos: BlockPos): boolean;

        /**
         * @brief 检查位置是否在领地上方（x/z 在领地范围内，且 y > max.y）
         */
        isAboveLand(pos: BlockPos): boolean;

        equals(other: LandAABB): boolean;

        /**
         * @brief 判断两个 AABB 是否有重叠部分
         */
        static isCollision(a: LandAABB, b: LandAABB): boolean;

        // 计算两个 AABB 之间的最短距离的平方（避免开根号，高性能）
        static getDistanceSq(a: LandAABB, b: LandAABB, includeY: boolean): bigint;

        /**
         * @brief 判断两个AABB是否满足最小间距要求
         */
        static isComplisWithMinSpacing(a: LandAABB, b: LandAABB, spacing: number, includeY: boolean): boolean;

        /**
         * @brief 判断一个 AABB 区域是否完整包含另一个 AABB 区域
         * 如果目标 AABB 在源 AABB 内，则返回 true，否则返回 false
         */
        static isContain(a: LandAABB, b: LandAABB): boolean;

        /**
         * @brief 获取两个 AABB 之间的最小间距(x/z 轴)
         * @return 返回实际间距（重叠则为实际重叠部分长度，负数）
         */
        static getMinSpacing(a: LandAABB, b: LandAABB, includeY: boolean): number;
    }

    type LandAABBView = ReadonlyView<LandAABB, "min" | "max", "canonicalize">;
}
