declare module "@runtime" {
    enum SerializableType {
        kUndefined = 0,
        kNull = 1,
        kBoolean = 2,
        kNumber = 3,
        kString = 4,
        kBigint = 5,
        kObject = 6, // plain object
        kArray = 7, // plain array
        kDate = 8,
        kRegExp = 9,
        kMap = 10,
        kSet = 11,
    }
    type PlainObject = { [key: string]: SerializableValue };
    type SerializableValue =
        | undefined
        | null
        | boolean
        | number
        | string
        | bigint
        | PlainObject // plain object
        | SerializableValue[] // plain array
        | Date
        | RegExp
        | Map<SerializableValue, SerializableValue>
        | Set<SerializableValue>;
    type TypeMap = {
        [SerializableType.kUndefined]: undefined;
        [SerializableType.kNull]: null;
        [SerializableType.kBoolean]: boolean;
        [SerializableType.kNumber]: number;
        [SerializableType.kString]: string;
        [SerializableType.kBigint]: bigint;
        [SerializableType.kObject]: PlainObject; // plain object
        [SerializableType.kArray]: SerializableValue[]; // plain array
        [SerializableType.kDate]: Date;
        [SerializableType.kRegExp]: RegExp;
        [SerializableType.kMap]: Map<SerializableValue, SerializableValue>;
        [SerializableType.kSet]: Set<SerializableValue>;
    };

    type Assert<T extends true> = T;
    type _Assert1 = Assert<TypeMap[keyof TypeMap] extends SerializableValue ? true : false>;
    type _Assert2 = Assert<SerializableValue extends TypeMap[keyof TypeMap] ? true : false>;

    // func
    type RemoteFunction = (...args: SerializableValue[]) => SerializableValue;

    // instance
    type CheckFunction<F> = F extends (...args: infer A) => infer R
        ? A extends SerializableValue[]
            ? R extends SerializableValue | void
                ? F
                : { error: "return-type"; got: R }
            : { error: "parameters"; got: A }
        : { error: "not-a-method"; got: F };

    type CheckMethod<K extends PropertyKey, F> = F extends (...args: any) => any
        ? CheckFunction<F> extends F
            ? F
            : { method: K; error: CheckFunction<F> }
        : { method: K; error: "not-a-method"; got: F };

    type CheckService<T> = { [K in keyof T]: CheckMethod<K, T[K]> };

    type CheckExport<T> = T extends (...args: any) => any
        ? CheckFunction<T>
        : T extends object
          ? CheckService<T>
          : { error: "not-exportable"; got: T };

    type RemoteObject = Record<string, RemoteFunction>;
    type Exportable = RemoteFunction | RemoteObject;

    type Identifier = `${string}:${string}`;

    interface BridgeAPI {
        /**
         * Check if a function/class instance is exported
         */
        has(identifier: Identifier): boolean;

        /**
         * Export a function or class instance for other engines to call
         * @param identifier unique identifier
         * @param obj object to export
         *
         * @throws {Error} if the identifier is already in use
         *                 if the object is not a function or object
         *                 if the argument count is not 2
         *
         * @note Class instances only support method calls
         *
         * @note All parameters and return values for remote calls through the bridge
         *       use structured clone, and cross-script references are not supported.
         */
        exports<T extends object>(identifier: Identifier, obj: T & CheckExport<NoInfer<T>>): void;

        /**
         * Unexport class instances or functions
         * @param identifier unique identifier
         *
         * @note Only supports canceling class instances or functions exported by the current script,
         *        and does not support canceling class objects or functions exported by other scripts.
         */
        unexport(identifier: Identifier): boolean;

        /**
         * Import a class instance or function
         * @param identifier unique identifier
         *
         * @note The imported class instance is actually a proxy object, and the
         *       functions are wrapper functions, all forwarded and called by the underlying C++.
         *
         * @note All parameters and return values for remote calls through the bridge
         *       use structured clone, and cross-script references are not supported.
         */
        imports(identifier: Identifier): Exportable | null;
    }
    export const bridge: BridgeAPI;
}
