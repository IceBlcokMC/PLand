declare module "@pland" {
    export interface LandRegistryAPI {
        isAdmin(uuid: UUID): boolean;

        addAdmin(uuid: UUID): boolean;

        removeAdmin(uuid: UUID): boolean;

        getAdmins(): UUID[];
    }
    export const LandRegistry: LandRegistryAPI;
}
