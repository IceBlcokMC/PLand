import type { CancellableMixin, Event } from "@levilamina";
import type { Player } from "@minecraft";
import type { _EventMap } from "@pland";

declare module "@levilamina" {
    export class Event {
        protected constructor();

        getId(): string;
    }

    interface CancellableMixin {
        readonly isCancelled: boolean;

        setCancelled(cancelled: boolean): void;

        cancel(): void;
    }

    /**
     * @param event 事件对象(此对象仅当前回调可用，超出作用域使用将会抛出异常)
     */
    export type Listener<T extends Event> = (event: T) => void;

    export type ListenerId = bigint;

    export enum EventPriority {
        Highest,
        High,
        Normal,
        Low,
        Lowest,
    }

    interface EventBusAPI {
        subscribe<T extends keyof _EventMap>(
            eventName: T,
            callback: Listener<_EventMap[T]>,
            priority?: EventPriority,
        ): optional<ListenerId>;

        unsubscribe(id: ListenerId | number): boolean;

        emit(event: Event): void;

        emitTo(event: Event, pluginName: string): void;
    }

    export const eventBus: EventBusAPI;
}

declare module "@pland" {
    export type _EventMap = {
        ["ScriptEvent"]: ScriptEvent;
        ["ConfigReloadEvent"]: ConfigReloadEvent;
        ["LandRecycleEvent"]: LandRecycleEvent;
        ["LandResizedEvent"]: LandResizedEvent;
        ["LandStateChangedEvent"]: LandStateChangedEvent;
        ["MemberChangedEvent"]: MemberChangedEvent;
        ["MembersClearedEvent"]: MembersClearedEvent;
        ["OwnerChangedEvent"]: OwnerChangedEvent;
        ["LandOwnershipChangedEvent"]: LandOwnershipChangedEvent;
    };

    /**
     * 脚本事件(用于脚本引擎之间的通信)
     * @note 此事件基于 structuredClone 进行分发载荷，每个引擎访问 data 得到的均为副本
     * @note 所有监听方统一用 ScriptEvent 监听脚本事件，内部自行根据 type 区分具体事件
     */
    export class ScriptEvent<T = any> extends Event implements CancellableMixin {
        /**
         * @param type 事件类别(脚本之间的事件名在 eventBus 均为 ScriptEvent，type 用于区分具体的事件)
         * @param data 事件数据(structuredClone 后分发于事件总线)
         * @note
         */
        constructor(type: string, data: T);

        readonly isCancelled: boolean;

        setCancelled(cancelled: boolean): void;

        cancel(): void;

        /** 发布此事件的插件名称 */
        readonly pluginName: string;

        /** 此事件的类别 */
        readonly type: string;

        /**
         * 事件数据(每次访问均通过 structuredClone 获取独立副本)
         * @note 当事件的发送方引擎关闭后，data 将为 null
         */
        readonly data: T | null;

        /**
         * 更新事件数据快照
         * @param newData 新数据
         *
         * @note 仅允许发布此事件的引擎调用
         * @note 更新仅替换事件内部保存的数据快照，不会修改此前通过 data 获取的副本
         * @note 更新不会通知其它引擎，需重新发布事件才能让监听方收到新事件
         */
        updateData(newData: T): void;
    }

    interface LandEventMixin {
        readonly land: Land;
    }

    export class ConfigReloadEvent extends Event {
        private constructor();
    }

    export class LandRecycleEvent extends Event implements LandEventMixin {
        private constructor();

        readonly land: Land;

        readonly reason: LandRecycleReason;
    }

    export class LandResizedEvent extends Event implements LandEventMixin {
        private constructor();

        readonly land: Land;

        /**
         * @warning 此对象只读，不可修改
         */
        readonly newRange: LandAABBView;
    }

    export class LandStateChangedEvent extends Event implements LandEventMixin {
        private constructor();

        readonly land: Land;

        readonly oldState: LeaseState;
        readonly newState: LeaseState;
    }

    export class MemberChangedEvent extends Event implements LandEventMixin {
        private constructor();

        readonly land: Land;

        readonly target: Player;
        readonly isAdd: boolean;
    }

    export class MembersClearedEvent extends Event implements LandEventMixin {
        private constructor();

        readonly land: Land;
    }

    export class OwnerChangedEvent extends Event implements LandEventMixin {
        private constructor();

        readonly land: Land;

        readonly oldOwner: Player;
        readonly newOwner: Player;
    }

    export class LandOwnershipChangedEvent extends Event implements LandEventMixin {
        private constructor();

        readonly land: Land;

        readonly oldKind: LandOwnershipKind;
        readonly newKind: LandOwnershipKind;
    }
}
