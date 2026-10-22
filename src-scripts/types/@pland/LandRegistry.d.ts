declare module "@pland" {
    export interface LandRegistryAPI {
        isAdmin(uuid: UUID): boolean;

        addAdmin(uuid: UUID): boolean;

        removeAdmin(uuid: UUID): boolean;

        getAdmins(uuid: UUID): boolean;
    }
    export const LandRegistry: LandRegistryAPI;
}
