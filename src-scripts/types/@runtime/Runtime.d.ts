declare module "@runtime" {
    export function log(fmt: string, ...args: any): void;

    export function format(fmt: string, ...args: any): string;

    export function self(): PluginEntry;

    export function setTimeout(callback: () => any, timeout: number): number;

    export function setInterval(callback: () => any, timeout: number): number;

    export function clearTimeout(id: number): boolean;

    export function clearInterval(id: number): boolean;

    interface PluginContext {
        readonly name: string;
        readonly type: string;
        readonly dir: string;
        readonly dataDir: string;
        readonly configDir: string;
        readonly langDir: string;
    }
    export const pluginsRoot: string;
    export const pluginCtx: PluginContext;
}
