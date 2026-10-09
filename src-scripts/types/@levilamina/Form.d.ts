import type {ModalFormCancelReason, Player} from "@minecraft";

declare module "@levilamina" {
    export type FormCancelReason = optional<ModalFormCancelReason>;

    export type CustomFormElementResult = variant<
        [null, number, string]// std::monostate, uint64, double, std::string
    >;
    export type CustomFormResult<T extends Record<string, CustomFormElementResult>> = optional<T>;
    export type CustomFormCallback<T extends Record<string, CustomFormElementResult>> = (
        player: Player,
        result: CustomFormResult<T>,
        cancelReason: FormCancelReason
    ) => void;

    type _NoDuplicateKey_<T extends Record<string, any>, K extends string> = K extends keyof T ? never : K;

    export class CustomForm<T extends Record<string, CustomFormElementResult> = {}> {
        constructor();
        constructor(title: string);

        setTitle(title: string): this;

        setSubmitButton(text: string): this;

        appendHeader(header: string): this;

        appendLabel(label: string): this;

        appendDivider(): this;

        appendInput<K extends string>(
            key: _NoDuplicateKey_<T, K>,
            text: string,
            placeholder: string,
            defaultVal: string,
            tooltip: string
        ): CustomForm<T & { [P in K]: string }>; // this

        appendToggle<K extends string>(
            key: _NoDuplicateKey_<T, K>,
            text: string,
            defaultVal: boolean, // default: false
            tooltip: string
        ): CustomForm<T & { [P in K]: number }>; // this, true: 1, false: 0

        appendDropdown<K extends string>(
            key: _NoDuplicateKey_<T, K>,
            text: string,
            options: string[],
            defaultVal: number, // default: 0
            tooltip: string
        ): CustomForm<T & { [P in K]: string }>; // this

        appendSlider<K extends string>(
            key: _NoDuplicateKey_<T, K>,
            text: string,
            min: number,
            max: number,
            step: number, // default: 0.0
            defaultVal: number, // default: 0
            tooltip: string
        ): CustomForm<T & { [P in K]: number }>; // this

        appendStepSlider<K extends string>(
            key: _NoDuplicateKey_<T, K>,
            text: string,
            steps: string[],
            defaultVal: number, // default: 0
            tooltip: string
        ): CustomForm<T & { [P in K]: string }>; // this

        sendTo(player: Player, callback: CustomFormCallback<T>): this;

        sendUpdate(player: Player, callback: CustomFormCallback<T>): this;

        getFormData(): string;
    }


    export enum ModalFormSelectedButton {
        Upper = 1, // true
        Lower = 0, // false
    }

    export type ModalFormResult = optional<ModalFormSelectedButton>;
    export type ModalFormCallback = (player: Player, result: ModalFormResult, cancelReason: FormCancelReason) => void;

    export class ModalForm {
        constructor();

        setTitle(title: string): this;

        setContent(content: string): this;

        setUpperButton(text: string): this;

        setLowerButton(text: string): this;

        sendTo(player: Player, callback: ModalFormCallback): this;

        sendUpdate(player: Player, callback: ModalFormCallback): this;
    }


    export type SimpleFormButtonCallback = (player: Player) => void;
    export type SimpleFormCallback = (player: Player, index: number, cancelReason: FormCancelReason) => void;

    export class SimpleForm {
        constructor();

        setTitle(title: string): this;

        setContent(content: string): this;

        appendHeader(header: string): this;

        appendLabel(label: string): this;

        appendDivider(): this;

        /**
         * Append a button to the form
         * @param  text      The text of the button
         * @param  imageData The image data of the button
         * @param  imageType The type of the image (url or path)
         * @param  callback  The callback of the button
         * @return SimpleForm&  *this
         * @note   If the `callback` parameter of sendTo() is set, the callback of the button will be ignored
         */
        appendButton(
            text: string,
            imageData: string,
            imageType: "url" | "path",
            callback: SimpleFormButtonCallback | null
        ): this;

        /**
         * Append a button to the form
         * @param  text      The text of the button
         * @param  callback  The callback of the button
         * @return SimpleForm&  *this
         * @note   If the `callback` parameter of sendTo() is set, the callback of the button will be ignored
         */
        appendButton(text: string, callback: SimpleFormButtonCallback | null): this;

        /**
         * Send the form to the player
         * @param  player    The player to receive the form
         * @param  callback  The callback of the form(if set, the callback of the button will be ignored)
         * @return SimpleForm&  *this
         * @note   If `callback` is set, the callbacks of each buttons will be ignored
         */
        sendTo(player: Player, callback: SimpleFormCallback | null): this;

        sendUpdate(player: Player, callback: SimpleFormCallback | null): this;
    }
}