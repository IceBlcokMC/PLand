import { I18n } from "@levilamina";
import { fs, path, pluginCtx } from "@runtime";

export const i18nKeyMap = {
    "debug.load.i18n.loaded": "i18n 已加载。",
    "debug.enable.command.registered": "命令注册完成",
    "command.admin.requrie.console.execute": "此命令只能在控制台中执行",
    "command.admin.name.empty": "请输入有效的玩家名称",
    "command.admin.name.notFound": "未找到玩家 {0} 的信息，请先进入服务器后再执行此命令",
    "command.admin.add.already": "玩家 {0} 已经是管理员, 请不要重复添加哦~",
    "command.admin.add.success": "玩家 {0} 已添加为管理员",
    "command.admin.add.fail": "玩家 {0} 添加为管理员失败",
    "command.admin.remove.notAdmin": "玩家 {0} 不是管理员，无法进行移除操作",
    "command.admin.remove.success": "已移除 {0} 的管理员身份",
    "command.admin.remove.fail": "移除 {0} 的管理员身份失败",
    "command.admin.players.empty": "请选择至少一个玩家进行操作",
    "command.admin.list.empty": "当前没有管理员",
    "command.admin.list.count": "当前共有 {0} 位管理员:",
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

// 构造长度为 N 的元组
type BuildTuple<N extends number, T extends unknown[] = []> = T["length"] extends N
    ? T
    : BuildTuple<N, [...T, unknown]>;

// 判断 A 是否大于 B
type GreaterThan<A extends number, B extends number> =
    BuildTuple<A> extends [...BuildTuple<B>, ...infer R] ? (R extends [] ? false : true) : false;

// 提取格式字符串中的最大占位符编号
type MaxPlaceholder<
    S extends string,
    Max extends number = -1,
> = S extends `${string}{${infer I extends number}}${infer Rest}`
    ? MaxPlaceholder<Rest, Max extends -1 ? I : GreaterThan<I, Max> extends true ? I : Max>
    : Max;

// 生成必需参数元组：最大编号 + 1
type FormatParams<S extends string> =
    MaxPlaceholder<S> extends infer N extends number ? (N extends -1 ? [] : [...BuildTuple<N>, any]) : never;

// 根据翻译 key 推导参数
type ParamsFor<K extends I18nKeys> = FormatParams<(typeof i18nKeyMap)[K]>;

// 至少满足占位符要求，多余参数允许传入
export const tr = <K extends I18nKeys>(key: K, ...params: [...ParamsFor<K>, ...any[]]): string =>
    i18n.tr(key, ...params);

export const trl = <K extends I18nKeys>(localeCode: string, key: K, ...params: [...ParamsFor<K>, ...any[]]): string =>
    i18n.trl(localeCode, key, ...params);
