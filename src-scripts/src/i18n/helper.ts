import { LandOwnershipKind, LandType, LeaseState } from "@pland";
import { tr, trl } from "./i18n.js";
import { I18n } from "@levilamina";

export function trOwnershipKind(kind: LandOwnershipKind, localeCode = I18n.defaultLocaleCode) {
    switch (kind) {
        case LandOwnershipKind.Player:
            return trl(localeCode, "land.ownership.player");
        case LandOwnershipKind.PendingMigration:
            return trl(localeCode, "land.ownership.pending-migration");
        case LandOwnershipKind.System:
            return trl(localeCode, "land.ownership.system");
        case LandOwnershipKind.Ownerless:
            return trl(localeCode, "land.ownership.ownerless");
        default:
            throw new Error(`Unknown ownership kind: ${kind}`);
    }
}

export function trLandType(type: LandType, localeCode = I18n.defaultLocaleCode) {
    switch (type) {
        case LandType.Ordinary:
            return trl(localeCode, "land.type.ordinary");
        case LandType.Parent:
            return trl(localeCode, "land.type.parent");
        case LandType.Mix:
            return trl(localeCode, "land.type.mix");
        case LandType.Sub:
            return trl(localeCode, "land.type.sub");
        default:
            throw new Error(`Unknown land type: ${type}`);
    }
}

export function trLeaseState(state: LeaseState, localeCode = I18n.defaultLocaleCode) {
    switch (state) {
        case LeaseState.None:
            return trl(localeCode, "land.lease-state.none");
        case LeaseState.Active:
            return trl(localeCode, "land.lease-state.active");
        case LeaseState.Frozen:
            return trl(localeCode, "land.lease-state.frozen");
        case LeaseState.Expired:
            return trl(localeCode, "land.lease-state.expired");
        default:
            throw new Error(`Unknown lease state: ${state}`);
    }
}
