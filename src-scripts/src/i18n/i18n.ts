import { I18n } from "@levilamina";
import { fs, path, pluginCtx } from "@runtime";

const i18nKeyMap = {
    "debug.load.i18n.loaded": "i18n 已加载。",
    "debug.enable.command.registered": "命令注册完成",
    "command.admin.requrie.console.execute": "此命令只能在控制台中执行",
    "command.admin.name.empty": "请输入有效的玩家名称",
    "command.admin.name.notFound": "未找到玩家 {name} 的信息，请先进入服务器后再执行此命令",
    "command.admin.add.already": "玩家 {name} 已经是管理员, 请不要重复添加哦~",
    "command.admin.add.success": "玩家 {name} 已添加为管理员",
    "command.admin.add.fail": "玩家 {name} 添加为管理员失败",
    "command.admin.remove.notAdmin": "玩家 {name} 不是管理员，无法进行移除操作",
    "command.admin.remove.success": "已移除 {name} 的管理员身份",
    "command.admin.remove.fail": "移除 {name} 的管理员身份失败",
    "command.admin.players.empty": "请选择至少一个玩家进行操作",
    "command.admin.list.empty": "当前没有管理员",
    "command.admin.list.count": "当前共有 {count} 位管理员:",
    "command.info.land.notFound": "未找到领地信息，请检查 ID 是否有效或当前位置是否在领地内",
    "command.info.type.base":
        "领地: {name} (ID:{id})\n类型: {type}\n大小(x,y,z): {count_x}x{count_y}x{count_z} = {volume}\n范围: {range}\n归属: {ownership}\n",
    "command.info.type.lease": "租赁状态: {status}\n租赁开始时间: {start}\n租赁到期时间: {end}\n",
    "command.info.type.sub-land": "父领地ID: {parent_id}\n下属子领地数量: {sub_count}\n",
    "land.ownership.player": "玩家领地",
    "land.ownership.pending-migration": "待迁移领地",
    "land.ownership.system": "系统领地",
    "land.ownership.ownerless": "无主领地",
    "land.type.ordinary": "普通领地",
    "land.type.parent": "父领地",
    "land.type.mix": "混合领地",
    "land.type.sub": "子领地",
    "land.lease-state.none": "未租赁",
    "land.lease-state.active": "租赁中",
    "land.lease-state.frozen": "已冻结",
    "land.lease-state.expired": "已过期",
} as const;

export const i18n = new I18n();

export const initI18n = () => {
    const langDir = path.join(pluginCtx.dir, "lang-script");

    if (!fs.exists(langDir)) {
        fs.mkdir(langDir, true);
        const output = path.join(langDir, "zh_CN.json");
        fs.writeFile(output, JSON.stringify(i18nKeyMap, null, 2));
    }
    i18n.load(langDir);
};

type I18nKeys = keyof typeof i18nKeyMap;

/** 提取 fmt 字段名称，忽略格式说明符 */
type ExtractFormatField<S extends string> = S extends `${infer Name}:${string}` ? Name : S;

/** 从 fmt 字符串提取所有命名参数 */
type ExtractFormatNames<S extends string> = S extends `${string}{${infer Field}}${infer Rest}`
    ? Field extends `{${string}`
        ? ExtractFormatNames<Rest>
        : ExtractFormatField<Field> | ExtractFormatNames<Rest>
    : never;

/** 根据 fmt 字符串生成参数对象 */
type NamedFormatArgs<S extends string> = string extends S
    ? Record<string, unknown>
    : [ExtractFormatNames<S>] extends [never]
      ? Record<string, never>
      : Record<ExtractFormatNames<S>, unknown>;

/** 根据模板决定参数是否必填 */
type NamedFormatParams<S extends string> = [ExtractFormatNames<S>] extends [never]
    ? [params?: NamedFormatArgs<S>]
    : [params: NamedFormatArgs<S>];

type NamedParamFor<K extends I18nKeys> = NamedFormatParams<(typeof i18nKeyMap)[K]>;

export const tr = <K extends I18nKeys>(key: K, ...params: NamedParamFor<K>): string => {
    return params.length === 0 ? i18n.tr(key) : i18n.tr(key, params[0]);
};

export const trl = <K extends I18nKeys>(localeCode: string, key: K, ...params: NamedParamFor<K>): string => {
    return params.length === 0 ? i18n.trl(localeCode, key) : i18n.trl(localeCode, key, params[0]);
};
