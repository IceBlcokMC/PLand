import {log, setTimeout, setInterval, clearInterval} from "@runtime"

function launchTask() {

    setTimeout(() => log("timeout called"), 1000);

    let clear = false
    let count = 0;
    const id = setInterval(() => {
        if (count < 3) {
            count++;
            log("setInterval called, count = {}", count);
        } else {
            if (clear == true) {
                throw new Error("clear interval failed");
            }
            clear = true;
            const ok = clearInterval(id);
            log("clearInterval called, res={}", ok);
            if (!ok) {
                throw new Error("clear interval failed");
            }
        }
    }, 1000)
}

export default class ScriptMod {
    constructor() {
    }

    onLoad() {
        log("onLoad called");
        log("onLoad called, {}", 123);
    }

    onEnable() {
        log("onEnable called");

        launchTask();
    }

    onDisable() {
        log("onDisable called");
    }
}