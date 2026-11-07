import { CommandParamKind, CommandRegistrar, type CommandHandle } from "@levilamina";
import { Player, type CommandOutput } from "@minecraft";
import { INVALID_LAND_ID, Land, LandRegistry, type LandID } from "@pland";
import { tr } from "../../i18n/i18n.js";
import { trLandType, trLeaseState, trOwnershipKind } from "../../i18n/helper.js";
import { formatDate, newDateFromNativeTimestamp } from "../../utils.js";

enum InfoType {
    Base = 0, // 基础信息
    Lease, // 租赁信息
    SubLand, // 子领地信息
    Full, // 全部信息
}
const kInfoTypeEnum = "pland-info-type";

interface QueryInfoArgs {
    type: InfoType;
    id: LandID | number;
    player: Player | null;
}

function resolveLand(query: QueryInfoArgs) {
    let land = null;
    if (query.id != INVALID_LAND_ID) {
        land = LandRegistry.getLandById(query.id as bigint);
    }
    if (!land) {
        if (query.player) {
            land = LandRegistry.getLandAtPosition(query.player.getPosition(), query.player.dimensionId);
        }
    }
    return land;
}

function logLandInfo(query: QueryInfoArgs, out: CommandOutput) {
    let land = resolveLand(query);
    if (!land) {
        out.error(tr("command.info.land.notFound"));
        return;
    }
    switch (query.type) {
        case InfoType.Base:
            showBaseInfo(land, out);
            break;
        case InfoType.Lease:
            showLeaseInfo(land, out);
            break;
        case InfoType.SubLand:
            showSubLandInfo(land, out);
            break;
        case InfoType.Full:
            showBaseInfo(land, out);
            showLeaseInfo(land, out);
            showSubLandInfo(land, out);
            break;
    }
}

function showBaseInfo(land: Land, out: CommandOutput) {
    const aabb = land.getAABB();
    out.success(
        tr("command.info.type.base", {
            name: land.getName(),
            id: land.id,
            type: trLandType(land.getType()),
            count_x: aabb.blockCountX,
            count_y: aabb.blockCountY,
            count_z: aabb.blockCountZ,
            volume: aabb.volume,
            range: aabb.toString(),
            ownership: trOwnershipKind(land.getOwnershipKind()),
        }),
    );
}
function showLeaseInfo(land: Land, out: CommandOutput) {
    out.success(
        tr("command.info.type.lease", {
            status: trLeaseState(land.getLeaseState()),
            start: formatDate(newDateFromNativeTimestamp(land.getLeaseStartAt())),
            end: formatDate(newDateFromNativeTimestamp(land.getLeaseEndAt())),
        }),
    );
}
function showSubLandInfo(land: Land, out: CommandOutput) {
    out.success(
        tr("command.info.type.sub-land", {
            parent_id: land.getParentLandID(),
            sub_count: land.getSubLandIDs().length,
        }),
    );
}

export default function initInfoSubCommand(cmd: CommandHandle) {
    if (!CommandRegistrar.hasEnum(kInfoTypeEnum)) {
        CommandRegistrar.tryRegisterRuntimeEnum(kInfoTypeEnum, [
            ["base", InfoType.Base],
            ["lease", InfoType.Lease],
            ["sub_land", InfoType.SubLand],
            ["full", InfoType.Full],
        ]);
    }

    // pland info [base|lease|sub_land|full] [id]
    cmd.runtimeOverload()
        .text("info")
        .optional("type", CommandParamKind.Enum, kInfoTypeEnum)
        .optional("id", CommandParamKind.Int)
        .execute((ori, out, args) => {
            const type = args?.type ?? InfoType.Base;
            const id = args?.id ?? INVALID_LAND_ID;
            logLandInfo({ type, id, player: ori.player }, out);
        });
}
