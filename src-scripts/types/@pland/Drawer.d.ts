import type { Player } from "@minecraft";

declare module "@pland" {
    export type GeoId = number;

    interface ShapeDrawerAPI {
        drawAABB(player: Player, aabb: LandAABB, dimension: DimensionType, color: Color): GeoId;

        drawLand(player: Player, land: Land, color: Color): void;

        removeByGeoId(player: Player, geoId: GeoId): void;

        removeByLandId(player: Player, landId: LandID): void;

        removeByLand(player: Player, land: Land): void;

        clearAll(player: Player): void;

        clearLandOnly(player: Player): void;
    }
    export const shapeDrawer: ShapeDrawerAPI;
}
