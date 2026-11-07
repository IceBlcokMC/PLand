declare module "@levilamina" {
    type NamedTranslateParams = {
        [key: string]: string | any;
    };

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

        tr(key: string, params?: NamedTranslateParams): string;
        trl(localeCode: string, key: string, params?: NamedTranslateParams): string;

        static readonly defaultLocaleCode: string;
    }
}
