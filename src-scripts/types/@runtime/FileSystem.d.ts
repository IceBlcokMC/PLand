declare module "@runtime" {
    interface FileSystem {
        exists(path: string): boolean;

        readFile(path: string): optional<string>;

        writeFile(path: string, data: string): boolean;

        copy(src: string, dst: string): boolean;
        move(src: string, dst: string): boolean;

        mkdir(path: string): boolean;
        mkdir(path: string, recursive: boolean): boolean;

        appendFile(path: string, data: string): boolean;
        appendFile(
            path: string,
            data: string,
            createIfNotExists: boolean,
        ): boolean;

        remove(path: string): boolean;
        remove(path: string, recursive: boolean): boolean;
    }

    export const fs: FileSystem;
}
