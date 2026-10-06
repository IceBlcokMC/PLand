declare module "@runtime" {
    export function log(fmt: string, ...args: any): void;

    export function format(fmt: string, ...args: any): void;

    export function self(): object;

    export function setTimeout(callback: () => any, timeout: number): number;

    export function setInterval(callback: () => any, timeout: number): number;

    export function clearTimeout(id: number): boolean;

    export function clearInterval(id: number): boolean;
}