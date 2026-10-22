declare module "@levilamina" {
    export enum LogLevel {
        Off = -1,
        Fatal,
        Error,
        Warn,
        Info,
        Debug,
        Trace,
    }

    /** Describes the shape of the exported logger object. */
    export interface LoggerAPI {
        log(level: LogLevel, message: any): void;
        log(level: LogLevel, fmt: string, ...args: any[]): void;

        info(message: any): void;
        info(fmt: string, ...args: any[]): void;

        warn(message: any): void;
        warn(fmt: string, ...args: any[]): void;

        error(message: any): void;
        error(fmt: string, ...args: any[]): void;

        fatal(message: any): void;
        fatal(fmt: string, ...args: any[]): void;

        debug(message: any): void;
        debug(fmt: string, ...args: any[]): void;

        trace(message: any): void;
        trace(fmt: string, ...args: any[]): void;

        getTitle(): string;

        getLevel(): LogLevel;

        shouldLog(level: LogLevel): boolean;

        setLevel(level: LogLevel): void;
    }

    export const logger: LoggerAPI;
}
