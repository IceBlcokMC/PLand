import { log } from "@runtime"

export default class Entry {

    onLoad() {
        log("onLoad called, test-pland-script")
    }

    onEnable() {
        log("onEnable called, test-pland-script")
    }

    onDisable() {
        log("onDisable called, test-pland-script")
    }
}