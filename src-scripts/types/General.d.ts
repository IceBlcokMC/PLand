// 通用类型：无具体对象

declare interface Vec2 {
    x: number;
    z: number;
}

declare interface Vec3 {
    x: number;
    y: number;
    z: number;
}

declare interface BlockPos {
    x: number;
    y: number;
    z: number;
}

declare type DimensionType = number;

declare type UUID = string;

/** Hex color in RGBA format (#RRGGBBAA). */
type Color = `#${string}`;

/**
 * 只读视图类型
 * @template T 原始类型
 * @template ReadonlyKeys 只读键
 * @template OmittedKeys 剔除键
 */
type ReadonlyView<T, ReadonlyKeys extends keyof T = never, OmittedKeys extends keyof T = never> = Omit<
    T,
    ReadonlyKeys | OmittedKeys
> & {
    readonly [K in ReadonlyKeys]: T[K];
};

declare interface PluginEntry {
    onLoad(): void;

    onEnable(): void;

    onDisable(): void;
}
