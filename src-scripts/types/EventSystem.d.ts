import type { Event } from "@levilamina";
import type { Player } from "@minecraft";
import type { _EventMap } from "@pland";

declare module "@levilamina" {
    export class Event {
        protected constructor();

        getId(): string;
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
    }

    export const eventBus: EventBusAPI;
}

declare module "@pland" {
    export type _EventMap = {
        ["ConfigReloadEvent"]: ConfigReloadEvent;
        ["LandRecycleEvent"]: LandRecycleEvent;
        ["LandResizedEvent"]: LandResizedEvent;
        ["LandStateChangedEvent"]: LandStateChangedEvent;
        ["MemberChangedEvent"]: MemberChangedEvent;
        ["MembersClearedEvent"]: MembersClearedEvent;
        ["OwnerChangedEvent"]: OwnerChangedEvent;
        ["LandOwnershipChangedEvent"]: LandOwnershipChangedEvent;
    };

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
