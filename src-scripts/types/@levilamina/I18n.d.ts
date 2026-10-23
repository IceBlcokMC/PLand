declare module "@levilamina" {
    export class I18n {
        constructor();

        /**
         * Load lang file
         * @param path lang path (lang dir or lang file)
         * @throws if load lang file failed
         */
        load(path: string): void;

        clear(): void;

        get(key: string, localeCode: string): string;

        set(localeCode: string, key: string, value: string): void;

        tr(key: string, ...params: string[]): string;
        trl(localeCode: string, key: string, ...params: string[]): string;

        static readonly defaultLocaleCode: string;
    }
}
