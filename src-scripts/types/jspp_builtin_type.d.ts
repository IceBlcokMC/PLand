// This type of file is a specialized type for type conversion built into jspp

declare type enum_t<T> = number;

declare type optional<T> = T | null;

declare type vector<T> = Array<T>;

declare type unordered_map<V> = {
  [key: string]: V;
};

declare type variant<T extends any[]> = T[number];

declare type monostate = null;

declare type pair<first, second> = [first, second];

declare type function_t<return_type, args extends any[] = []> = (
  ...args: args
) => return_type;

declare type shared_ptr<T> = T | null;

declare type unique_ptr<T> = T | null;

declare type weak_ptr<T> = T | null;

declare type reference_wrapper<T> = T;

declare type filesystem_path = string;
