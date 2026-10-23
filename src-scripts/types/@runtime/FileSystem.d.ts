declare module "@runtime" {
    // Contract: relative paths are resolved against the server's current working directory,
    // like Node.js fs. Absolute paths are used as-is.
    //
    // Paths are POSIX-style: '/' is the separator and a leading '/' means an absolute path.
    // Windows-style input is also accepted: '\' is treated as a separator, and "X:/…" / "X:\…" are absolute.
    // There is no sandbox: absolute paths and ".." can reach anywhere the server process can.
    //
    // Bool returns mean "the operation succeeded"; OS-level rejections (permission denied, etc.)
    // are thrown as exceptions instead.

    interface FileSystem {
        /** Whether the path exists. */
        exists(path: string): boolean;

        /**
         * Reads a file as text.
         * @returns null when the path is missing or is not a regular file.
         */
        readFile(path: string): optional<string>;

        /**
         * Writes data to a file, creating missing parent directories.
         * @returns true only when all data was written successfully.
         */
        writeFile(path: string, data: string): boolean;

        /**
         * Copies a file, or a whole directory tree when src is a directory.
         * Creates missing parent directories of dst; existing files are overwritten.
         */
        copy(src: string, dst: string): boolean;

        /**
         * Moves or renames a file or directory.
         * Creates missing parent directories of dst.
         */
        move(src: string, dst: string): boolean;

        /**
         * Creates a directory. The parent must exist.
         * @returns false when the directory already exists.
         */
        mkdir(path: string): boolean;
        /**
         * Creates a directory including missing parent directories.
         * @returns false when the directory already exists.
         */
        mkdir(path: string, recursive: boolean): boolean;

        /**
         * Appends data to an existing file as-is.
         * Line endings are up to the caller: put "\n" or "\r\n" into data yourself.
         * @returns false when the file does not exist.
         */
        appendFile(path: string, data: string): boolean;
        /**
         * Like appendFile, but creates the file (and missing parent directories)
         * when createIfNotExists is true.
         */
        appendFile(
            path: string,
            data: string,
            createIfNotExists: boolean,
        ): boolean;

        /**
         * Deletes a file or a directory tree. Always recursive.
         * @returns true when anything was deleted, false when the path did not exist.
         */
        remove(path: string): boolean;

        /** Whether the path is a directory. */
        isDirectory(path: string): boolean;

        /** Whether the path is a regular file. */
        isFile(path: string): boolean;
    }

    export const fs: FileSystem;

    /**
     * POSIX path utilities, aligned with Node.js path.posix.
     * Windows-style separators ('\') are accepted and normalized to '/'.
     */
    interface PathAPI {
        /** Joins the segments with '/' and normalizes the result. */
        join(...paths: string[]): string;
        basename(path: string): string;
        dirname(path: string): string;
        extname(path: string): string;
        /** Relative path from `from` to `to`; relative inputs are resolved against the plugin folder. */
        relative(from: string, to: string): string;
        /** Resolves the path against the plugin folder (kept as-is when absolute) and normalizes it. */
        absolute(path: string): string;
        normalize(path: string): string;
    }
    export const path: PathAPI;
}
