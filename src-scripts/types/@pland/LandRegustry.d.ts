declare module "@pland" {
    export class LandRegistry {
        private constructor();

        static isAdmin(uuid: UUID): boolean;

        static addAdmin(uuid: UUID): boolean;

        static removeAdmin(uuid: UUID): boolean;

        static getAdmins(uuid: UUID): boolean;
    }
}
